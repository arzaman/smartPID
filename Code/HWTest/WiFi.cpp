/*
 * WiFi.cpp
 *
 * Wi-Fi communication manager
 */

#include <nvm.h>
#include <SparkFunESP8266WiFi.h>
#include "ABM_Cfg.h"
#include "storage_var.h"
#include "WiFi.h"

/* Intervals expressed in milliseconds */
#define WIFI_STATUS_UPDATE_INTERVAL		10000
#define WIFI_CONNECT_ATTEMPT_INTERVAL	30000

/* Number of failed connection attempts in automatic mode before Wi-Fi network
 * information is reset and the controller is rebooted. */
#define WIFI_AUTO_CONN_RETRIES	3

#define WIFI_SERVER_PORT        80

enum wifi_async_resp {
	WIFI_ASYNC_NONE,
	WIFI_ASYNC_GENERIC,
	WIFI_ASYNC_CONNECT,
	WIFI_ASYNC_DHCP_STA,
	WIFI_ASYNC_SET_AP,
	WIFI_ASYNC_SET_DHCP_AP,
	WIFI_ASYNC_IP_ADDR,
	WIFI_ASYNC_DISCONNECT,
	WIFI_ASYNC_STATUS,
};

static struct {
	int mode;
	unsigned int failedConns;
	const char *ssid, *pwd;
	IPAddress IPaddr;
	volatile int asyncResp;
	unsigned long lastStatusUpdate, lastConnectAttempt;
} wifiPriv;

/* Check for a response to a command previously sent to ESP8266
 * Parameters: none
 * Returns: none
 */
static void checkAsyncResp() {
	int16_t resp = esp8266.checkAsyncResp();

	if (!resp)
		return;
	switch (wifiPriv.asyncResp) {
	case WIFI_ASYNC_GENERIC:
		wifiPriv.asyncResp = WIFI_ASYNC_NONE;
		break;
	case WIFI_ASYNC_CONNECT:
		if (resp > 0) {
			wifiPriv.asyncResp = WIFI_ASYNC_DHCP_STA;
			esp8266.setDHCP(ESP8266_MODE_STA, true, true);
		} else {
			wifiPriv.asyncResp = WIFI_ASYNC_NONE;
			wifiPriv.lastConnectAttempt = millis();
			wifiPriv.failedConns++;
			if ((wifiPriv.mode == WiFi::MODE_AUTO)
					&& (wifiPriv.failedConns == WIFI_AUTO_CONN_RETRIES)) {
				StvResetWiFiNwk();
				wiFi.reset();
				NVIC_SystemReset();
			}
		}
		break;
	case WIFI_ASYNC_DHCP_STA:
		wiFi.staStatus = WiFi::STATUS_CONNECTED;
		wifiPriv.asyncResp = WIFI_ASYNC_NONE;
		break;
	case WIFI_ASYNC_SET_AP:
		wifiPriv.asyncResp = WIFI_ASYNC_SET_DHCP_AP;
		esp8266.setDHCP(ESP8266_MODE_AP, true, true);
		break;
	case WIFI_ASYNC_SET_DHCP_AP:
		wiFi.status = WiFi::STATUS_AP;
		wifiPriv.asyncResp = WIFI_ASYNC_NONE;
		break;
	case WIFI_ASYNC_IP_ADDR:
		if (resp > 0)
			wifiPriv.IPaddr = esp8266.localIPFromResp();
		wifiPriv.asyncResp = WIFI_ASYNC_NONE;
		break;
	case WIFI_ASYNC_DISCONNECT:
		wifiPriv.asyncResp = WIFI_ASYNC_GENERIC;
		esp8266.setMode(ESP8266_MODE_STA, true);
		break;
	case WIFI_ASYNC_STATUS:
		if (resp > 0) {
			enum esp8266_connect_status status =
					(enum esp8266_connect_status) esp8266.statusFromResp();

			if (status == ESP8266_STATUS_NOWIFI) {
				wiFi.staStatus = WiFi::STATUS_DISCONNECTED;
				wifiPriv.IPaddr = INADDR_NONE;
			}
		}
		wifiPriv.asyncResp = WIFI_ASYNC_NONE;
		break;
	}
}

WiFi wiFi;
ESP8266Client wiFiClient;
ESP8266Server wiFiServer(WIFI_SERVER_PORT);

WiFi::WiFi() {
	client = &wiFiClient;
	server = &wiFiServer;
	wifiPriv.asyncResp = WIFI_ASYNC_NONE;
}

int WiFi::init(uint32_t baud) {
	uint8_t serNoBuf[NVMCTRL_PAGE_SIZE];
	uint8_t *serNoPtr =
			serNoBuf + (HW_SERIAL_NUM_ADDR & (NVMCTRL_PAGE_SIZE - 1));

	nvm_read_buffer(HW_SERIAL_NUM_ADDR & ~(NVMCTRL_PAGE_SIZE - 1), serNoBuf,
			sizeof(serNoBuf));
	sprintf(own_ssid, "SPC1000_%02X%02X%02X%02X%02X%02X%02X", serNoPtr[1],
			serNoPtr[2], serNoPtr[3], serNoPtr[4], serNoPtr[5], serNoPtr[6],
			serNoPtr[7]);
	if (esp8266.begin(Serial1, baud) == true)
	{
		status = STATUS_OFF;
	}
	else if (esp8266.begin(Serial1, 9600) || esp8266.begin(Serial1, 19200)
			|| esp8266.begin(Serial1, 38400)|| esp8266.begin(Serial1, 57600)
			|| esp8266.begin(Serial1, 115200)) {
		esp8266.setBaud(baud);
		delay(10);
		esp8266.begin(Serial1, baud);
		status = STATUS_OFF;
	}
	else
		status = STATUS_NA;
	return status;
}

void WiFi::config(int mode, const char *ssid, const char *pwd) {
	switch (mode) {
	case MODE_OFF:
		status = STATUS_OFF;
		while (esp8266.disconnect(true) == ESP8266_CMD_BUSY) {}
		wifiPriv.asyncResp = WIFI_ASYNC_DISCONNECT;
		break;
	case MODE_STA:
		configSta(ssid, pwd);
		break;
	case MODE_AP:
		configAP();
		break;
	case MODE_AUTO:
		if (*ssid && *pwd) {
			configSta(ssid, pwd);
		} else {
			configAP();
		}
		break;
	default:
		return;
	}
	wifiPriv.mode = mode;
	wifiPriv.failedConns = 0;
	wifiPriv.IPaddr = INADDR_NONE;
}

void WiFi::configSta(const char *ssid, const char *pwd) {
	wifiPriv.ssid = ssid;
	wifiPriv.pwd = pwd;
	while (esp8266.setMode(ESP8266_MODE_STA) == ESP8266_CMD_BUSY) {}
	while (esp8266.connect(ssid, pwd, true) == ESP8266_CMD_BUSY) {}
	status = STATUS_STA;
	staStatus = STATUS_DISCONNECTED;
	wifiPriv.asyncResp = WIFI_ASYNC_CONNECT;
}


void WiFi::configAP(void) {
	char cur_ssid[SSID_MAX_LEN + 1];	/* null-terminated string */
	char cur_pwd[PWD_MAX_LEN + 1];	/* null-terminated string */

	while (esp8266.setMode(ESP8266_MODE_AP) == ESP8266_CMD_BUSY) {}
	while (esp8266.getAPConfig(cur_ssid, cur_pwd) == ESP8266_CMD_BUSY) {}
	if (strcmp(cur_ssid, own_ssid) || strcmp(cur_pwd, WIFI_OWN_PWD)) {
		while (esp8266.configAP(own_ssid, WIFI_OWN_PWD, true)
				== ESP8266_CMD_BUSY) {}
		wifiPriv.asyncResp = WIFI_ASYNC_SET_AP;
	} else {
		wifiPriv.asyncResp = WIFI_ASYNC_SET_DHCP_AP;
	}
}

IPAddress WiFi::getIP() {
	return wifiPriv.IPaddr;
}

int WiFi::getSSID(char *buf) {
	if (status == STATUS_STA)
		strcpy(buf, wifiPriv.ssid);
	else if (status == STATUS_AP)
		strcpy(buf, own_ssid);
	else
		return -1;
	return 0;
}

int WiFi::getPwd(char *buf) {
	if (status == STATUS_STA)
		strcpy(buf, wifiPriv.pwd);
	else if (status == STATUS_AP)
		strcpy(buf, WIFI_OWN_PWD);
	else
		return -1;
	return 0;
}

void WiFi::setServerPort(uint16_t port) {
	wiFiServer = ESP8266Server(port);
}

void WiFi::reset()
{
	esp8266.reset(false);
}

void WiFi::tick()
{
	/* Check if communication with ESP8266 is ongoing. */
	if (wifiPriv.asyncResp == WIFI_ASYNC_NONE) {
		if ((status == STATUS_STA) && (staStatus == STATUS_DISCONNECTED)
				&& (millis() - wifiPriv.lastConnectAttempt
						>= WIFI_CONNECT_ATTEMPT_INTERVAL)) {
			/* Retry to connect to the access point */
			if (esp8266.connect(wifiPriv.ssid, wifiPriv.pwd, true)
					== ESP8266_RSP_SUCCESS)
				wifiPriv.asyncResp = WIFI_ASYNC_CONNECT;
		} else if ((status == STATUS_STA) && (staStatus == STATUS_CONNECTED)
				&& (millis() - wifiPriv.lastStatusUpdate
						>= WIFI_STATUS_UPDATE_INTERVAL)) {
			/* Check if still connected. */
			if (esp8266.updateStatus(true) == ESP8266_RSP_SUCCESS) {
				wifiPriv.asyncResp = WIFI_ASYNC_STATUS;
				wifiPriv.lastStatusUpdate = millis();
			}
		} else if ((((status == STATUS_STA) && (staStatus == STATUS_CONNECTED))
				|| (status == STATUS_AP)) && (wifiPriv.IPaddr == INADDR_NONE)) {
			/* Retrieve local IP address */
			if ((uint32_t) esp8266.localIP(true) == ESP8266_RSP_SUCCESS)
				wifiPriv.asyncResp = WIFI_ASYNC_IP_ADDR;
		}
	}
	if (wifiPriv.asyncResp)
		checkAsyncResp();
}
void WiFiTick()
{
	wiFi.tick();
}
