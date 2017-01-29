/*
 * Log management
 *
 *  Copyright (C) 2016 Arzaman
 */

#include <SpiFlash.h>
#include <ThingSpeak.h>

#include "ABM_Cfg.h"
#include "ABM_HW_switch.h"
#include "delay.h"
#include "log.h"
#include "PID_control.h"
#include "storage_var.h"
#include "WiFi.h"

#define SECONDS_PER_HOUR (60 * 60)
#define SECONDS_PER_DAY (24 * SECONDS_PER_HOUR)

/* Magic number indicating the beginning of a log */
#define LOG_EEPROM_START_MAGIC	0x55

#define LOG_EEPROM_ENTRY_SIZE	32	/* Must be a power of 2 */

/* Timeouts and intervals expressed in  milliseconds */
#define LOG_WIFI_RESP_TIMEOUT		5000
#define LOG_WIFI_CHECK_INTERVAL		10000

#define LOG_CHAN_1 (1 << 0)
#define LOG_CHAN_2 (1 << 1)

/* Finite State Machine managing asynchronous operations */
enum logAsyncFSM {
	LOG_ASYNC_IDLE,
	LOG_ASYNC_CONNECT,
	LOG_ASYNC_CONNECTED,
	LOG_ASYNC_GET_KEY,
	LOG_ASYNC_CHECK_SENT,
	LOG_ASYNC_READ_HDR,
	LOG_ASYNC_READ_KEY,
	LOG_ASYNC_READ_LOG_ENTRY,
	LOG_ASYNC_DISCONNECT,
	LOG_ASYNC_DISCONNECTED,
};

static struct {
	uint32_t start;
	uint32_t eepromNextEntry, wifiCh1NextEntry, wifiCh2NextEntry;
	uint32_t eepromLogNum, eepromLogPtr;
	int eepromUsedBlocks;
	uint8_t wifiLogPending;
	uint32_t wifiAsyncStart, wifiCheck;
	char wifiBuf[1024];
	unsigned int wifiBufIdx;
	char writeKey[THINGSPEAK_KEY_LEN + 1];	/* null-terminated string */
	enum logAsyncFSM asyncFSM;
} logStatus;

/* Erase the next memory block in the EEPROM when a new block is being written
 * to.
 * This is done so that there is always at least one free block in memory, which
 * allows overwriting old logs with new ones when the memory is full while still
 * being able to determine unequivocally where a given log ends.
 * Parameters: none
 * Returns: none
 */
static void logEepromEraseNext(void)
{
	uint32_t nextEraseAddr;

	if (logStatus.eepromLogPtr < spiFlash.memSize() - spiFlash.eraseSize())
		nextEraseAddr = logStatus.eepromLogPtr + spiFlash.eraseSize();
	else
		nextEraseAddr = 0;
	if (spiFlash.read8(nextEraseAddr) != 0xFF)
		spiFlash.erase(nextEraseAddr);
	else
		logStatus.eepromUsedBlocks++;
}

/* Add a new log entry to the EEPROM for a given PID channel
 * Parameters:
 * - n: channel number (0 or 1)
 * - timestamp: time, expressed in milliseconds, elapsed since the beginning of
 *   the PID process
 * Returns: none
 */
static void logEepromChan(int n, uint32_t timestamp)
{
	uint8_t buf[LOG_EEPROM_ENTRY_SIZE];
	PIDe *pid = (n == 0) ? &PID_n1 : &PID_n2;

	if (logStatus.eepromLogPtr % spiFlash.eraseSize() == 0)
		logEepromEraseNext();
	buf[0] = n;
	memcpy(&buf[1], &timestamp, 4);
	buf[5] = _stv.HW_var.contr_mode;
	if (_stv.HW_var.contr_mode == T_STATIC)
		buf[6] = 'T';
	else if (((_stv.HW_var.contr_mode == HEATING)
			&& (_stv.HW_var.heat_mode == MODE_PID))
			|| ((_stv.HW_var.contr_mode == COOLING)
					&& (_stv.HW_var.cool_mode == MODE_PID)))
		buf[6] = 'P';
	else
		buf[6] = 'O';
	memcpy(&buf[7], pid->mySetpoint, 4);
	memcpy(&buf[11], pid->myInput, 4);
	if ((SwitchMode[n].mode == mpsHEAT_PID)
			|| (SwitchMode[n].mode == mpsCOOLING_PID))
		buf[15] = SwitchMode[n].pwm;
	else {
		if (SwitchMode[n].on_off == onoffSET)
			buf[15] =  100;
		else
			buf[15] = 0;
	}
	if ((SwitchMode[n].mode == mpsHEAT_PID)
			|| ((SwitchMode[n].mode == mpsHEAT_ONOFF)
					&& (SwitchMode[n].on_off == onoffSET)))
		buf[16] = 'H';
	else if ((SwitchMode[n].mode == mpsCOOLING_PID)
			|| ((SwitchMode[n].mode == mpsCOOLING_ONOFF)
					&& (SwitchMode[n].on_off == onoffSET)))
		buf[16] = 'C';
	else
		buf[16] = 'O';
	spiFlash.writePage(logStatus.eepromLogPtr, buf, 17);
	logStatus.eepromLogPtr =
			(logStatus.eepromLogPtr + LOG_EEPROM_ENTRY_SIZE)
			% spiFlash.memSize();
}

/* Add a new log entry to the EEPROM for each active PID channel
 * Parameters:
 * - timestamp: time, expressed in milliseconds, elapsed since the beginning of
 *   PID processes
 * Returns: none
 */
static int logEeprom(uint32_t timestamp)
{
	if (PID_n1.Enable)
		logEepromChan(0, timestamp);
	if (PID_n2.Enable)
		logEepromChan(1, timestamp);
	return 0;
}

/* Find the end of a JSON object in a given string
 * Parameters:
 * - buf: buffer containing the JSON string: the pointer must point to any
 *   location inside the JSON object (i.e. between the opening parenthesis and
 *   the closing parenthesis)
 * Returns: pointer to the closing parenthesis that indicates the end of the
 *          JSON object
 */
static char *jsonObjectEnd(char *buf)
{
	int len = strlen(buf);
	int escape = 0, in_string = 0, paren_level = 1;

	for (int i = 0; i < len; i++) {
		if (((buf[i] == '"') || (buf[i] == '\'')) && !escape)
			in_string = !in_string;
		else if ((buf[i] == '\\') && !escape)
			escape = 1;
		else
			escape = 0;
		if (!in_string && !escape) {
			if (buf[i] == '{')
				paren_level++;
			else if (buf[i] == '}') {
				paren_level--;
				if (paren_level == 0)
					return (buf + i);
			}
		}
	}
	return NULL;
}

/* Parse headers in the HTTP response contained in the logStatus.wifiBuf buffer
 * Parameters: none
 * Returns: HTTP status code if the set of headers could be parsed, negative
 *          value otherwise
 */
static int logGetHttpHdr(void)
{
	logStatus.wifiBuf[logStatus.wifiBufIdx] = '\0';

	/* Find HTTP status code */
	char *ptr = strstr(logStatus.wifiBuf, "HTTP/1.1");
	if (!ptr)
		return -1;

	/* Find end of HTTP headers */
	char *hdrEnd = strstr(ptr, "\n\r\n");
	if (!hdrEnd)
		return -1;

	ptr += 8;	/* strlen("HTTP/1.1") */
	int status = atoi(ptr);

	/* Header parsing completed: make room for the HTTP response body */
	memmove(logStatus.wifiBuf, hdrEnd,
			logStatus.wifiBufIdx - (hdrEnd - logStatus.wifiBuf));
	logStatus.wifiBufIdx -= hdrEnd - logStatus.wifiBuf;

	return status;
}

/* Retrieve the write API key from a server response contained in the
 * logStatus.wifiBuf buffer.
 * If the key can be parsed, it is copied to the logStatus.writeKey array
 * Parameters: none
 * Returns: zero if the key could be parsed, negative value otherwise
 */
static int logGetWriteKey()
{
	char *ptr = logStatus.wifiBuf, *ptr1, *ptr2;
	char *objEnd;

	logStatus.wifiBuf[logStatus.wifiBufIdx] = '\0';
	while (1) {
		ptr = strstr(ptr, "\"api_key\"");
		if (!ptr)
			break;
		ptr += 9;	/* strlen("\"api_key\"") */
		ptr = strstr(ptr, "\"");
		if (!ptr)
			break;
		objEnd = jsonObjectEnd(ptr);
		if (!objEnd)
			break;
		ptr1 = strstr(++ptr, "\"");
		if (!ptr1 || (ptr1 - ptr != THINGSPEAK_KEY_LEN))
			break;
		ptr1 = strstr(ptr1, "\"write_flag\":");
		if (!ptr1 || (ptr1 >= objEnd))
			break;
		ptr1 += 13;	/* strlen("\"write_flag\":") */
		while (isspace(*ptr1))
			ptr1++;
		ptr2 = strstr(ptr1, "true");
		if (!ptr2)
			break;
		if (ptr2 == ptr1) {
			memcpy(logStatus.writeKey, ptr, THINGSPEAK_KEY_LEN);
			logStatus.writeKey[THINGSPEAK_KEY_LEN] = '\0';
			logChanStatus = LOG_CHAN_STATUS_OK;
			return 0;
		} else if (ptr2 >= objEnd)
			break;
	}
	return -1;
}

/* Add a new log entry to the logging for a given PID channel
 * Parameters:
 * - n: channel number (0 or 1)
 * - timestamp: time, expressed in milliseconds, elapsed since the beginning of
 *   the PID process
 * Returns: zero if the log entry could be sent to the server, negative value
 *          otherwise
 */
static int logWiFiChan(int n, uint32_t timestamp)
{
	PIDe *pid;
	uint32_t seconds;
	char str[16];
	const char *str_ptr;

	if (logStatus.asyncFSM != LOG_ASYNC_CONNECTED)
		return -1;
	pid = (n == 0) ? &PID_n1 : &PID_n2;
	seconds = timestamp / 1000;
	sprintf(str, "%d", seconds / SECONDS_PER_DAY);
	seconds %= SECONDS_PER_DAY;
	sprintf(str + strlen(str), "-%02d", seconds / SECONDS_PER_HOUR);
	seconds %= SECONDS_PER_HOUR;
	sprintf(str + strlen(str), ":%02d", seconds / 60);
	seconds %= 60;
	sprintf(str + strlen(str), ":%02d", seconds);
	ThingSpeak.setField(1, str);
	ThingSpeak.setField(2, n + 1);	/* CH1 or CH2 */
	switch (_stv.HW_var.contr_mode) {
	case HEATING:
		str_ptr = "Heating";
		break;
	case COOLING:
		str_ptr = "Cooling";
		break;
	default:
		str_ptr = "Thermostatic";
	}
	ThingSpeak.setField(3, str_ptr);
	if (_stv.HW_var.contr_mode == T_STATIC)
		str_ptr = "T";
	else if (((_stv.HW_var.contr_mode == HEATING)
			&& (_stv.HW_var.heat_mode == MODE_PID))
			|| ((_stv.HW_var.contr_mode == COOLING)
					&& (_stv.HW_var.cool_mode == MODE_PID)))
		str_ptr = "PID";
	else
		str_ptr = "ON-OFF";
	ThingSpeak.setField(4, str_ptr);
	ThingSpeak.setField(5, *pid->mySetpoint);
	ThingSpeak.setField(6, *pid->myInput);
	if ((SwitchMode[n].mode == mpsHEAT_PID)
			|| (SwitchMode[n].mode == mpsCOOLING_PID))
		ThingSpeak.setField(7, SwitchMode[n].pwm);
	else {
		if (SwitchMode[n].on_off == onoffSET)
			ThingSpeak.setField(7, 100);
		else
			ThingSpeak.setField(7, 0);
	}
	if ((SwitchMode[n].mode == mpsHEAT_PID)
			|| ((SwitchMode[n].mode == mpsHEAT_ONOFF)
					&& (SwitchMode[n].on_off == onoffSET)))
		str_ptr = "Heating";
	else if ((SwitchMode[n].mode == mpsCOOLING_PID)
			|| ((SwitchMode[n].mode == mpsCOOLING_ONOFF)
					&& (SwitchMode[n].on_off == onoffSET)))
		str_ptr = "Cooling";
	else
		str_ptr = "Off";
	ThingSpeak.setField(8, str_ptr);
	String postMessage = ThingSpeak.formatFields();
	sprintf(logStatus.wifiBuf, "GET /update?key=%s&%s\n\n",
			logStatus.writeKey, postMessage.c_str());
	if (wiFi.client->sendAsync((uint8_t *) logStatus.wifiBuf,
			strlen(logStatus.wifiBuf)) == 0) {
		logStatus.asyncFSM = LOG_ASYNC_CHECK_SENT;
		return 0;
	} else
		return -1;
}

enum logServerStatus logServerStatus;
enum logChanStatus logChanStatus;

void logInit(void)
{
	/* A log in EEPROM starts at the beginning of a flash erase block */
	int	logStartAlign = spiFlash.eraseSize();

	uint32_t flashSize = spiFlash.memSize();
	uint8_t magicNum;
	uint32_t logNum;
	bool logStartFound = false;

	spiFlash.init(SPI, PIN_SPI_SS);
	for (uint32_t i = 0; i < flashSize; i += logStartAlign) {
		magicNum = spiFlash.read8(i);
		if (magicNum == 0xFF) {
			if (!logStartFound) {
				logStatus.eepromLogPtr = i;
				logStartFound = true;
			}
		} else {
			logStatus.eepromUsedBlocks++;
			if (magicNum == LOG_EEPROM_START_MAGIC) {
				spiFlash.read(i + 1, (uint8_t *)&logNum, sizeof(logNum));
				if (logNum >= logStatus.eepromLogNum) {
					logStatus.eepromLogNum = logNum + 1;
					logStartFound = false;
				}
			}
		}
	}
	wiFi.init(_stv.mConLogP.baud);
	if (wiFi.status != WiFi::STATUS_NA)
		wiFi.config(_stv.mConLogP.ConnectMode, _stv.mConLogP.SSID,
				_stv.mConLogP.pwd);
	logServerStatus = LOG_SERVER_STATUS_ERROR;
	logChanStatus = LOG_CHAN_STATUS_ERROR;
	logStatus.wifiCheck = millis();
	ThingSpeak.begin(*wiFi.client, _stv.mConLogP.serverIP,
			THINGSPEAK_PORT_NUMBER);
}

void logStart(void)
{
	if (_stv.mConLogP.LogMode & LOG_EEPROM) {
		if (logStatus.eepromLogPtr % spiFlash.eraseSize())
			logStatus.eepromLogPtr += spiFlash.eraseSize()
				- logStatus.eepromLogPtr % spiFlash.eraseSize();
		logEepromEraseNext();
		spiFlash.write8(logStatus.eepromLogPtr, LOG_EEPROM_START_MAGIC);
		spiFlash.write(logStatus.eepromLogPtr + 1,
				(uint8_t *)&logStatus.eepromLogNum,
				sizeof(logStatus.eepromLogNum));
		logStatus.eepromLogPtr =
				(logStatus.eepromLogPtr + LOG_EEPROM_ENTRY_SIZE)
				% spiFlash.memSize();
		logStatus.eepromLogNum++;
	}
	logStatus.start = millis();
	logStatus.eepromNextEntry = logStatus.wifiCh1NextEntry = logStatus.start;
	if (PID_n2.Enable)
		/* Log entries for channel 1 and channel 2 are sent to the logging
		 * server in a way that maximizes the temporal spacing between
		 * successive entries, to work around issues related to rate limiting
		 * enforced by the server. */
		logStatus.wifiCh2NextEntry =
				logStatus.start + _stv.mConLogP.SampleTime * 1000;
}

void logProcess(void)
{
	uint32_t now = millis();

	if ((_stv.mConLogP.LogMode & LOG_EEPROM)
			&& (now >= logStatus.eepromNextEntry)) {
		if (logEeprom(now - logStatus.start) >= 0)
			logStatus.eepromNextEntry += _stv.ProcPar.s_time;
	}
	if ((_stv.mConLogP.LogMode & LOG_WIFI) && !logStatus.wifiLogPending) {
		if (PID_n1.Enable && (now >= logStatus.wifiCh1NextEntry)) {
			logStatus.wifiLogPending |= LOG_CHAN_1;
			if (PID_n2.Enable)
				/* We are logging both PID channels: double the sampling time so
				 * that the effective rate of log entries sent to the server
				 * corresponds to the sample time set by the user. */
				logStatus.wifiCh1NextEntry +=
						2 * _stv.mConLogP.SampleTime * 1000;
			else
				logStatus.wifiCh1NextEntry += _stv.mConLogP.SampleTime * 1000;
		} else if (PID_n2.Enable && (now >= logStatus.wifiCh2NextEntry)) {
			logStatus.wifiLogPending |= LOG_CHAN_2;
			logStatus.wifiCh2NextEntry += 2 * _stv.mConLogP.SampleTime * 1000;
		}
	}
	if (logStatus.wifiLogPending & LOG_CHAN_1) {
		if (logWiFiChan(0, now - logStatus.start) == 0)
			logStatus.wifiLogPending &= ~LOG_CHAN_1;
	} else if (logStatus.wifiLogPending & LOG_CHAN_2) {
		if (logWiFiChan(1, now - logStatus.start) == 0)
			logStatus.wifiLogPending &= ~LOG_CHAN_2;
	}
}

int logEepromMemStatus(void)
{
	/* Add 1 to log_status.eeprom_used_blocks because when there is only one
	 * unused block it means the EEPROM is full (the logging algorithm ensures
	 * there is always at least one free block). */
	return (100 * (logStatus.eepromUsedBlocks + 1)
			/ (spiFlash.memSize() / spiFlash.eraseSize()));
}

void logTick(void)
{
	int status;

	switch (logStatus.asyncFSM) {
	case LOG_ASYNC_IDLE:
		if (!(_stv.mConLogP.LogMode & LOG_WIFI)
				|| (wiFi.status != WiFi::STATUS_STA)
				|| (wiFi.staStatus != WiFi::STATUS_CONNECTED)) {
			logServerStatus = LOG_SERVER_STATUS_ERROR;
			break;
		}
		if ((logServerStatus != LOG_SERVER_STATUS_OK)
				|| ((logChanStatus != LOG_CHAN_STATUS_OK)
						&& (millis() >= logStatus.wifiCheck))
				|| logStatus.wifiLogPending) {
			if (wiFi.client->connectAsync(_stv.mConLogP.serverIP,
					THINGSPEAK_PORT_NUMBER) == 0)
				logStatus.asyncFSM = LOG_ASYNC_CONNECT;
		}
		break;
	case LOG_ASYNC_CONNECT:
		status = wiFi.client->connectedAsync();
		if (status > 0) {
			logServerStatus = LOG_SERVER_STATUS_OK;
			if (logChanStatus != LOG_CHAN_STATUS_OK)
				logStatus.asyncFSM = LOG_ASYNC_GET_KEY;
			else if (logStatus.wifiLogPending)
				logStatus.asyncFSM = LOG_ASYNC_CONNECTED;
			else
				logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
		} else if (status < 0) {
			logServerStatus = LOG_SERVER_STATUS_ERROR;
			logStatus.asyncFSM = LOG_ASYNC_IDLE;
		}
		break;
	case LOG_ASYNC_GET_KEY:
		sprintf(logStatus.wifiBuf, "GET /channels/%d.json?key=%s\n\n",
				_stv.mConLogP.channelID, _stv.mConLogP.userKey);
		if (wiFi.client->sendAsync((uint8_t *)logStatus.wifiBuf,
				strlen(logStatus.wifiBuf)) == 0)
			logStatus.asyncFSM = LOG_ASYNC_CHECK_SENT;
		break;
	case LOG_ASYNC_CHECK_SENT:
		status = wiFi.client->checkSent();
		if (status < 0) {
			logServerStatus = LOG_SERVER_STATUS_ERROR;
			logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
		} else if (status > 0) {
			logStatus.wifiAsyncStart = millis();
			logStatus.wifiBufIdx = 0;
			if (logChanStatus != LOG_CHAN_STATUS_OK)
				logStatus.asyncFSM = LOG_ASYNC_READ_KEY;
			else
				logStatus.asyncFSM = LOG_ASYNC_READ_LOG_ENTRY;
		}
		break;
	case LOG_ASYNC_READ_HDR:
		if ((!wiFi.client->available()
				&& (millis() - logStatus.wifiAsyncStart > LOG_WIFI_RESP_TIMEOUT))
				|| (logStatus.wifiBufIdx == sizeof(logStatus.wifiBuf) - 1)) {
			logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
			break;
		}
		if (!wiFi.client->available())
			break;
		do {
			logStatus.wifiBuf[logStatus.wifiBufIdx++] = wiFi.client->read();
		} while (wiFi.client->available()
				&& (logStatus.wifiBufIdx < sizeof(logStatus.wifiBuf) - 1));
		status = logGetHttpHdr();
		if (status > 0) {
			if (status == 200)
				logStatus.asyncFSM = LOG_ASYNC_READ_KEY;
			else {
				logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
				logStatus.wifiCheck = millis() + LOG_WIFI_CHECK_INTERVAL;
			}
		}
		break;
	case LOG_ASYNC_READ_KEY:
		if (!wiFi.client->available()
				|| (logStatus.wifiBufIdx == sizeof(logStatus.wifiBuf) - 1)) {
			if (!logGetWriteKey() /* key retrieved successfully */
					|| (logStatus.wifiBufIdx == sizeof(logStatus.wifiBuf) - 1)
					|| (millis() - logStatus.wifiAsyncStart
					> LOG_WIFI_RESP_TIMEOUT))
				logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
			break;
		}
		do {
			logStatus.wifiBuf[logStatus.wifiBufIdx++] = wiFi.client->read();
		} while (wiFi.client->available()
				&& (logStatus.wifiBufIdx < sizeof(logStatus.wifiBuf) - 1));
		break;
	case LOG_ASYNC_READ_LOG_ENTRY:
		if (!wiFi.client->available()) {
			if (millis() - logStatus.wifiAsyncStart > LOG_WIFI_RESP_TIMEOUT) {
				logServerStatus = LOG_SERVER_STATUS_ERROR;
				logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
			}
		} else if (isdigit(wiFi.client->read()))
			logStatus.asyncFSM = LOG_ASYNC_DISCONNECT;
		break;
	case LOG_ASYNC_DISCONNECT:
		if (wiFi.client->stopAsync() == 0)
			logStatus.asyncFSM = LOG_ASYNC_DISCONNECTED;
		break;
	case LOG_ASYNC_DISCONNECTED:
		if (wiFi.client->stoppedAsync())
			logStatus.asyncFSM = LOG_ASYNC_IDLE;
		break;
	}
}
