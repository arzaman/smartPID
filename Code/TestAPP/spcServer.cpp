/*
 * SmartPID Controller server for remote management
 *
 *  Copyright (C) 2016 Arzaman
 */

#include "ABM_Cfg.h"
#include "ArduinoJson.h"
#include "ESP8266UdpServer.h"
#include "PID_control.h"
#include "run_mode.h"
#include "storage_var.h"
#include "WiFi.h"

#define htons ntohs

#define HTTP_HEADER_HTML	"HTTP/1.1 200 OK\r\n"	\
	"Content-Type: text/html\r\n"					\
	"Connection: close\r\n\r\n"
#define HTTP_HEADER_JSON	"HTTP/1.1 200 OK\r\n"	\
	"Content-Type: application/json\r\n"			\
	"Connection: close\r\n\r\n"
#define HTML_HEAD_START												\
	"<!DOCTYPE html><html lang=\"en\"><head>"					\
	"<meta name=\"viewport\" content=\"width=device-width, "	\
	"initial-scale=1.0\"/><title>SmartPID</title>\r\n"
#define HTML_STYLE														\
	"<style>.c{text-align: center;} "									\
	"input{padding:5px;margin-top:2px;margin-bottom:5px;font-size:1em;"	\
	"width:96%;} body{text-align: center;font-family:arial;} "			\
	"button{border:0;border-radius:0.3rem;background-color:#ffc20e;"	\
	"color:#fff;line-height:2rem;font-size:1rem;width:100%;}</style>\r\n"
#define HTML_HEAD_END													\
	"</head><body><div style='text-align:left;display:inline-block;"	\
	"min-width:260px;'>\r\n"
#define HTML_FORM_START	"<h2>SmartPID Wi-Fi Configuration</h2>"			\
		"<form method='post' action='/'>"
#define HTML_FORM_END	"<br/><button type='submit'>Save</button></form>\r\n"
#define HTML_END	"</div></body></html>"

#define SPC_SERVER_PARAM_SSID		"ssid"
#define SPC_SERVER_PARAM_AP_PWD		"ap_pwd"
#define SPC_SERVER_PARAM_REMOTEIP	"remote_ip"
#define SPC_SERVER_PARAM_CHAN		"chan"
#define SPC_SERVER_PARAM_REMOTEKEY	"remote_key"
#define SPC_SERVER_PARAM_BAUD		"baud"
#define SPC_SERVER_PARAM_LOCAL_PORT	"local_port"

/* If the following timeout occurs after a connection from a client, we process
 * any data we received from the client, send a response to the client if these
 * data make some sense, and close the connection. */
#define SPC_SERVER_TIMEOUT	10000

#define DNS_UDP_PORT	53

#define DNS_FLAGS_RESP				(1 << 15)
#define DNS_FLAGS_OPCODE_SHIFT		11
#define DNS_FLAGS_OPCODE_MASK		0x7800
#define DNS_FLAGS_OPCODE(flags)		\
	(((flags) & DNS_FLAGS_OPCODE_MASK) >> DNS_FLAGS_OPCODE_SHIFT)
#define DNS_FLAGS_AA				(1 << 10)
#define DNS_FLAGS_TRUNC				(1 << 9)
#define DNS_FLAGS_RECDES			(1 << 8)
#define DNS_FLAGS_RECAV				(1 << 7)
#define DNS_FLAGS_RETCODE_MASK		0x000F
#define DNS_FLAGS_RETCODE(flags)	((flags) & DNS_FLAGS_RETCODE_MASK)

#define DNS_TYPE_A		0x0001
#define DNS_TYPE_NS		0x0002
#define DNS_TYPE_CNAME	0x0005
#define DNS_TYPE_PTR	0x000C
#define DNS_TYPE_MX		0x000F
#define DNS_TYPE_SRV	0x0021
#define DNS_TYPE_IXFR	0x00FB
#define DNS_TYPE_AXFR	0x00FC
#define DNS_TYPE_ALL	0x00FF

#define DNS_CLASS_IN	0x0001

#define DNS_RETCODE_NOERROR		0x0000
#define DNS_RETCODE_FORMERR		0x0001
#define DNS_RETCODE_SERVFAIL	0x0002
#define DNS_RETCODE_NXDOMAIN	0x0003
#define DNS_RETCODE_NOTIMP		0x0004
#define DNS_RETCODE_REFUSED		0x0005
#define DNS_RETCODE_YXDOMAIN	0x0006
#define DNS_RETCODE_YXRRSET		0x0007
#define DNS_RETCODE_NXRRSET		0x0008
#define DNS_RETCODE_NOAUTH		0x0009
#define DNS_RETCODE_NOTZONE		0x000A

/* Length of a resource record whose name is encoded as a pointer and whose data
 * is an IPv4 address. */
#define DNS_RESREC_LEN	16

struct dnsHeader {
	uint16_t id;
	uint16_t flags;
	uint16_t questCount;
	uint16_t ansCount;
	uint16_t authCount;
	uint16_t addCount;
};

/* Finite State Machine managing server operation */
enum spcServerFSM {
	SPC_SERVER_OFF,
	SPC_SERVER_DNS_START,
	SPC_SERVER_IDLE,
	SPC_SERVER_DNS_RESP_OK,
	SPC_SERVER_DNS_RESP_ERROR,
	SPC_SERVER_RESPONSE,
};

enum spcServerResource {
	SPC_SERVER_RES_DEFAULT,
	SPC_SERVER_RES_TEMP,
};

enum http_method {
	HTTP_METHOD_UNKNOWN,
	HTTP_GET,
	HTTP_POST,
};

static struct {
	enum spcServerFSM status;
	ESP8266Client client;
	uint32_t connTime;
	char req[256];
	bool reqHdr;
	unsigned int rxPtr;
	enum http_method method;
	char url[16];
	unsigned int contentLen;
} spcServer;

static unsigned int hexValue(char c)
{
	if (c <= '9')
		return (c - '0');
	else
		return (10 + toupper(c) - 'A');
}

#ifdef SPC_DNS_SERVER

static ESP8266UdpServer dnsServer(DNS_UDP_PORT);
static uint8_t dnsBuf[128];
static unsigned int dnsPtr;

static uint16_t ntohs(uint16_t val)
{
	if (LITTLE_ENDIAN)
		return ((val << 8) | (val >> 8));
	else
		return val;
}

static bool spcServerDnsQuestionParse(uint8_t *buf, unsigned int len)
{
	unsigned int offset = 0;
	unsigned int labelLen;

	while (offset < len) {
		labelLen = buf[offset];
		if (labelLen == 0)
			/* Return true if the two 16-bit fields following the domain name
			 * (i.e. Question Type and Question Class) have been received as
			 * well). */
			return (len > offset + 4);
		else
			offset += 1 + labelLen;
	}
	return false;
}

static bool spcServerDnsTick(void)
{
	if (!dnsServer.running()) {
		if (wiFi.status == WiFi::STATUS_AP) {
			spcServer.status = SPC_SERVER_DNS_START;
			return true;
		} else
			return false;
	}
	if (dnsServer.available()) {
		do {
			dnsBuf[dnsPtr] = dnsServer.read();

			/* If there is no space left to parse the request, any newly arrived
			 * byte will overwrite the last received byte, and when no more
			 * bytes are available the request will likely end up being
			 * discarded when parsed. */
			if (dnsPtr < sizeof(dnsBuf))
				dnsPtr++;
		} while (dnsServer.available());
		return true;
	} else {
		struct dnsHeader *hdr;
		uint16_t flags;
		uint16_t type;

		if ((dnsPtr < sizeof(*hdr))
				|| !spcServerDnsQuestionParse(dnsBuf + sizeof(*hdr),
						dnsPtr - sizeof(*hdr)))
			return false;
		hdr = (struct dnsHeader *)dnsBuf;
		flags = ntohs(hdr->flags);
		type = (((uint16_t)(*(dnsBuf + dnsPtr - 4))) << 8)
				| *(dnsBuf + dnsPtr - 3);
		if (!(flags & DNS_FLAGS_RESP) && (DNS_FLAGS_OPCODE(flags) == 0x00)
				&& (ntohs(hdr->questCount) == 1) && (type == DNS_TYPE_A)
				&& (ntohs(hdr->ansCount) == 0) && (ntohs(hdr->authCount) == 0)
				&& (ntohs(hdr->addCount) == 0))
			spcServer.status = SPC_SERVER_DNS_RESP_OK;
		else
			/* discard packet */
			dnsPtr = 0;
		return true;
	}
}

#endif

static String urlDecode(String string)
{
	char escapeSeq[4], escapedChar;

	string.replace('+', ' ');
	if (string.length() <= 2)
		return string;
	escapeSeq[0] = '%';
	escapeSeq[3] = '\0';
	for (int i = 0; i < string.length() - 2; i++) {
		if (string[i] == escapeSeq[0]) {
			escapeSeq[1] = string[i + 1];
			escapeSeq[2] = string[i + 2];
			escapedChar =
					(hexValue(escapeSeq[1]) << 4) + hexValue(escapeSeq[2]);
			string.replace(escapeSeq, String(escapedChar));
		}
	}
	return string;
}

static String spcServerFormParam(const char *name, const char *id,
		const char *value)
{
	String formParam;

	formParam = "<br/>";
	formParam += name;
	formParam += ": <input name='";
	formParam += id;
	formParam += "' value='";
	formParam += value;
	formParam += "'>";
	return formParam;
}

static int spcServerParseParam(const char *buf, unsigned int bufLen,
		String &paramName, String &paramValue)
{
	int skippedChars;
	int paramLen, nameLen, valueLen;
	char helperString[256];

	if (buf[0] == '&') {
		/* separator between parameters: skip it */
		buf++;
		bufLen--;
		skippedChars = 1;
	} else
		skippedChars = 0;
	for (paramLen = 0; paramLen < bufLen; paramLen++)
		if (buf[paramLen] == '&')
			break;
	for (nameLen = 0; nameLen < bufLen; nameLen++)
		if (buf[nameLen] == '=')
			break;
	valueLen = paramLen - nameLen - 1;
	if ((nameLen <= 0) || (nameLen >= sizeof(helperString))
			|| (valueLen < 0) || (valueLen >= sizeof(helperString)))
		return 0;
	memcpy(helperString, buf, nameLen);
	helperString[nameLen] = '\0';
	paramName = String(helperString);
	memcpy(helperString, buf + nameLen + 1, valueLen);
	helperString[valueLen] = '\0';
	paramValue = urlDecode(String(helperString));
	return (skippedChars + paramLen);
}

static bool spcServerSendCfg()
{
	String response;
	char strValue[16];

	response = String(HTTP_HEADER_HTML HTML_HEAD_START HTML_STYLE HTML_HEAD_END
			HTML_FORM_START
			+ spcServerFormParam("SSID", SPC_SERVER_PARAM_SSID,
					_stv.mConLogP.SSID)
			+ spcServerFormParam("Password", SPC_SERVER_PARAM_AP_PWD,
					_stv.mConLogP.pwd));
	sprintf(strValue, "%d.%d.%d.%d", _stv.mConLogP.serverIP[0],
			_stv.mConLogP.serverIP[1], _stv.mConLogP.serverIP[2],
			_stv.mConLogP.serverIP[3]);
	response += spcServerFormParam("Server IP Address",
			SPC_SERVER_PARAM_REMOTEIP, strValue);
	itoa(_stv.mConLogP.channelID, strValue, 10);
	response += spcServerFormParam("Channel Id", SPC_SERVER_PARAM_CHAN,
			strValue)
			+ spcServerFormParam("API Key", SPC_SERVER_PARAM_REMOTEKEY,
					_stv.mConLogP.userKey);
	itoa(_stv.mConLogP.localServerPort, strValue, 10);
	response += spcServerFormParam("Local Server Port",
			SPC_SERVER_PARAM_LOCAL_PORT, strValue);
	itoa(_stv.mConLogP.baud, strValue, 10);
	response += spcServerFormParam("Serial Baud Rate", SPC_SERVER_PARAM_BAUD,
			strValue) + HTML_FORM_END HTML_END;
	return (spcServer.client.print(response) != ESP8266_CMD_BUSY);
}

static bool spcServerRecvCfg()
{
	String paramName, paramValue;
	int parsedChars;
	bool cfgUpdated = false;
	String result, response;

	spcServer.rxPtr = 0;
	while (spcServer.rxPtr < spcServer.contentLen) {
		parsedChars = spcServerParseParam(spcServer.req + spcServer.rxPtr,
				spcServer.contentLen - spcServer.rxPtr, paramName, paramValue);
		if (parsedChars == 0) {
			result = "Unable to parse request!";
			break;
		}
		if (paramName.equals(SPC_SERVER_PARAM_SSID)) {
			if (paramValue.length() <= WiFi::SSID_MAX_LEN)
				strcpy(_stv.mConLogP.SSID, paramValue.c_str());
			else {
				result = "SSID too long!";
				break;
			}
		} else if (paramName.equals(SPC_SERVER_PARAM_AP_PWD)) {
			if (paramValue.length() <= WiFi::PWD_MAX_LEN)
				strcpy(_stv.mConLogP.pwd, paramValue.c_str());
			else {
				result = "Password too long!";
				break;
			}
		} else if (paramName.equals(SPC_SERVER_PARAM_REMOTEIP)) {
			IPAddress remoteIP;

			if (remoteIP.fromString(paramValue))
				_stv.mConLogP.serverIP = remoteIP;
			else {
				result = "Invalid server IP address!";
				break;
			}
		} else if (paramName.equals(SPC_SERVER_PARAM_CHAN)) {
			int chan = paramValue.toInt();

			if (chan > 0)
				_stv.mConLogP.channelID = chan;
			else {
				result = "Invalid channel!";
				break;
			}
		} else if (paramName.equals(SPC_SERVER_PARAM_REMOTEKEY)) {
			if (paramValue.length() == THINGSPEAK_KEY_LEN)
				strcpy(_stv.mConLogP.userKey, paramValue.c_str());
			else {
				result = "Invalid API key!";
				break;
			}
		} else if (paramName.equals(SPC_SERVER_PARAM_LOCAL_PORT)) {
			int port = paramValue.toInt();

			if ((port <= 0) || (port > 0xFFFF)) {
				result = "Invalid local server port!";
				break;
			} else
				_stv.mConLogP.localServerPort = port;
		} else if (paramName.equals(SPC_SERVER_PARAM_BAUD)) {
			int baud = paramValue.toInt();

			if (baud <= 0) {
				result = "Invalid baud rate!";
				break;
			} else if ((baud != 9600) && (baud != 19200) && (baud != 38400)
					&& (baud != 57600) && (baud != 115200)) {
				result = "Unsupported baud rate!";
				break;
			} else
				_stv.mConLogP.baud = baud;
		} else {
			result = String("Invalid parameter '") + paramName + "'!";
			break;
		}
		spcServer.rxPtr += parsedChars;
	}
	if (spcServer.contentLen == 0)
		result = "Empty request!";
	else if (spcServer.rxPtr == spcServer.contentLen) {
		StoreVarToEEprom();
		cfgUpdated = true;
		result = "Configuration updated, restarting controller";
	} else
		RestoreVarFromEEprom();
	response = String(HTTP_HEADER_HTML HTML_HEAD_START)
			+ String(HTML_STYLE) + String(HTML_HEAD_END) + result
			+ String(HTML_END);
	spcServer.client.print(response);
	return cfgUpdated;
}

static bool spcServerCheckChanStatus(int chan, JsonObject &response)
{
	if (mProcStat.enable != 1) {
		response["status"] = "process not running";
	} else if ((chan < 0) || (chan > 2)) {
		response["status"] = "invalid channel";
	} else if ((chan == 2) && !PID_n2.Enable) {
		response["status"] = "channel not enabled";
	} else {
		return true;
	}
	return false;
}

static void spcServerGetTemp(PIDe &pid, JsonObject &json)
{
	json["t"] = *pid.myInput;
	json["sp"] = *pid.mySetpoint;
}

static bool spcServerSendTemp(int chan)
{
	String response = String(HTTP_HEADER_JSON);
	StaticJsonBuffer<128> jsonBuffer;
	JsonObject &jsonResp = jsonBuffer.createObject();

	if (spcServerCheckChanStatus(chan, jsonResp)) {
		jsonResp["status"] = "ok";
		switch (chan) {
		case 1:
			spcServerGetTemp(PID_n1, jsonResp);
			break;
		case 2:
			spcServerGetTemp(PID_n2, jsonResp);
			break;
		default:
			jsonResp["t1"] = *PID_n1.myInput;
			jsonResp["sp1"] = *PID_n1.mySetpoint;
			if (PID_n2.Enable) {
				jsonResp["t2"] = *PID_n2.myInput;
				jsonResp["sp2"] = *PID_n2.mySetpoint;
			}
			break;
		}
	}
	jsonResp.prettyPrintTo(response);
	return (spcServer.client.print(response) != ESP8266_CMD_BUSY);
}

static bool spcServerSetSP(JsonObject &req, const char *objName, PIDe &pid,
		JsonObject &resp)
{
	bool reqOK;

	if (req.containsKey(objName)) {
		if (!pid.Enable) {
			resp["status"] = "channel not enabled";
			reqOK = false;
		} else if ((req[objName].is<float>())
				|| (req[objName].is<int>())) {
			float sp = req[objName];

			if ((sp >= TEMP_C_MIN) && (sp <= TEMP_C_MAX)) {
				*pid.mySetpoint = sp;
				reqOK = true;
			} else {
				resp["status"] = "set point outside allowed range";
				reqOK = false;
			}
		} else {
			resp["status"] = "invalid set point value";
			reqOK = false;
		}
	} else {
		reqOK = true;
	}
	return reqOK;
}

static bool spcServerRecvTemp(int chan)
{
	String response = String(HTTP_HEADER_JSON);
	StaticJsonBuffer<64> jsonRespBuf;
	JsonObject &jsonResp = jsonRespBuf.createObject();

	if (spcServerCheckChanStatus(chan, jsonResp)) {
		/* To parse the JSON content we need it to be a NULL-terminated string,
		 * so if necessary we drop the last received character and hope for the
		 * best. */
		if (spcServer.contentLen == sizeof(spcServer.req))
			spcServer.contentLen--;
		spcServer.req[spcServer.contentLen] = '\0';

		StaticJsonBuffer<64> jsonReqBuf;
		JsonObject& jsonReq = jsonReqBuf.parseObject(spcServer.req);
		if (!jsonReq.success()) {
			jsonResp["status"] = "JSON parsing failed";
		} else {
			bool reqOK;

			switch (chan) {
			case 1:
				reqOK = spcServerSetSP(jsonReq, "sp", PID_n1, jsonResp);
				break;
			case 2:
				reqOK = spcServerSetSP(jsonReq, "sp", PID_n2, jsonResp);
				break;
			default:
				reqOK = spcServerSetSP(jsonReq, "sp1", PID_n1, jsonResp);
				if (reqOK)
					reqOK = spcServerSetSP(jsonReq, "sp2", PID_n2, jsonResp);
				break;
			}
			if (reqOK)
				jsonResp["status"] = "ok";
		}
	}
	jsonResp.prettyPrintTo(response);
	while (spcServer.client.print(response) == ESP8266_CMD_BUSY) {}
	return true;
}

static bool spcServerResp(void)
{
	int resId;
	bool done;
	int chan;
	bool cfgUpdated = false;

	if (!strncmp(spcServer.url, "/temp", 5)) {
		resId = SPC_SERVER_RES_TEMP;
		if ((spcServer.url[5] == '\0')
				|| ((spcServer.url[5] == '/') && (spcServer.url[6] == '\0')))
			chan = 0;
		else
			chan = atoi(spcServer.url + 6);
	} else {
		resId = SPC_SERVER_RES_DEFAULT;
	}
	if (spcServer.method == HTTP_GET)
	{
		switch (resId) {
		case SPC_SERVER_RES_TEMP:
			done = spcServerSendTemp(chan);
			break;
		default:
			done = spcServerSendCfg();
		}
	}
	else if (spcServer.method == HTTP_POST)
	{
		/* Set the content length to the number of received characters, just in
		 * case we received less characters than expected. */
		spcServer.contentLen = spcServer.rxPtr;

		switch (resId) {
		case SPC_SERVER_RES_TEMP:
			done = spcServerRecvTemp(chan);
			break;
		default:
			cfgUpdated = spcServerRecvCfg();
			done = true;
		}
	}
	else
		/* unsupported HTTP request */
		done = true;
	if (!done)
		return false;
	spcServer.client.stop();
	if (cfgUpdated) {
		wiFi.reset();
		NVIC_SystemReset();
	}
	return true;
}

void spcServerProcess(void)
{
	if ((wiFi.status == WiFi::STATUS_NA) || (wiFi.status == WiFi::STATUS_OFF))
		return;
	switch (spcServer.status) {
	case SPC_SERVER_OFF:
		wiFi.setServerPort(_stv.mConLogP.localServerPort);
		if (wiFi.server->beginCheck()) {
			spcServer.client = ESP8266Client(ESP8266_SOCK_NOT_AVAIL);
			spcServer.status = SPC_SERVER_IDLE;
		}
		break;
#ifdef SPC_DNS_SERVER
	case SPC_SERVER_DNS_START:
		if (dnsServer.beginCheck())
			spcServer.status = SPC_SERVER_IDLE;
		break;
	case SPC_SERVER_DNS_RESP_OK:
		/* Check is there is space left to store the response packet. */
		if (dnsPtr <= (sizeof(dnsBuf) - DNS_RESREC_LEN)) {
			struct dnsHeader *hdr = (struct dnsHeader *)dnsBuf;
			uint16_t flags = ntohs(hdr->flags) | DNS_FLAGS_RESP;
			uint32_t ttl = 300;
			IPAddress ipAddr = wiFi.getIP();

			hdr->flags = htons(flags);
			hdr->ansCount = hdr->questCount;
			dnsBuf[dnsPtr++] = 0xC0;	/* name encoded as a pointer */
			dnsBuf[dnsPtr++] = sizeof(*hdr);	/* pointer value */
			dnsBuf[dnsPtr++] = DNS_TYPE_A >> 8;
			dnsBuf[dnsPtr++] = DNS_TYPE_A & 0x0F;
			dnsBuf[dnsPtr++] = DNS_CLASS_IN >> 8;
			dnsBuf[dnsPtr++] = DNS_CLASS_IN & 0x0F;
			dnsBuf[dnsPtr++] = ttl >> 24;
			dnsBuf[dnsPtr++] = ttl >> 16;
			dnsBuf[dnsPtr++] = ttl >> 8;
			dnsBuf[dnsPtr++] = ttl & 0xFF;
			dnsBuf[dnsPtr++] = 4 >> 8;	/* resource data length */
			dnsBuf[dnsPtr++] = 4 & 0xFF;	/* resource data length */
			dnsBuf[dnsPtr++] = ipAddr[0];
			dnsBuf[dnsPtr++] = ipAddr[1];
			dnsBuf[dnsPtr++] = ipAddr[2];
			dnsBuf[dnsPtr++] = ipAddr[3];
			if (dnsServer.write(dnsBuf, dnsPtr) != dnsPtr) {
				/* transmission failed: will retry later */
				dnsPtr -= DNS_RESREC_LEN;
				break;
			}
		}
		dnsPtr = 0;
		spcServer.status = SPC_SERVER_IDLE;
		break;
	case SPC_SERVER_DNS_RESP_ERROR: {
		struct dnsHeader *hdr = (struct dnsHeader *)dnsBuf;
		uint16_t flags = (ntohs(hdr->flags) & ~DNS_FLAGS_RETCODE_MASK)
				| DNS_FLAGS_RESP | DNS_RETCODE_NXDOMAIN;

		hdr->flags = htons(flags);

		/* Try to transmit response, but if transmission fails we can't be
		 * bothered. */
		dnsServer.write(dnsBuf, dnsPtr);

		dnsPtr = 0;
		spcServer.status = SPC_SERVER_IDLE;
		break;
	}
#endif
	case SPC_SERVER_RESPONSE:
		if (!spcServerResp())
			break;

		/* mark the client as disconnected */
		spcServer.client = ESP8266Client(ESP8266_SOCK_NOT_AVAIL);

		spcServer.status = SPC_SERVER_IDLE;
		break;
	}
}

static bool spcServerParseHdr()
{
	if (spcServer.req[spcServer.rxPtr - 1] != '\n')
		return false;
	if (spcServer.method == HTTP_METHOD_UNKNOWN) {
		char *urlBegin, *urlEnd;

		if (!strncmp(spcServer.req, "GET", 3))
			spcServer.method = HTTP_GET;
		else if (!strncmp(spcServer.req, "POST", 4))
			spcServer.method = HTTP_POST;
		urlBegin = (char *)memchr(spcServer.req, ' ', spcServer.rxPtr);
		urlEnd = (char *)memrchr(spcServer.req, ' ', spcServer.rxPtr);
		if (!urlBegin || !urlEnd)
			strcpy(spcServer.url, '\0');
		else {
			urlBegin++;
			while ((urlBegin - spcServer.req < spcServer.rxPtr)
					&& (*urlBegin == ' '))
				urlBegin++;
			while ((urlEnd - spcServer.req > 0) && (*(urlEnd - 1) == ' '))
				urlEnd--;
			if (urlBegin >= urlEnd)
				strcpy(spcServer.url, '\0');
			else {
				if (urlEnd - urlBegin >= sizeof(spcServer.url))
					/* truncate url to maximum supported length */
					urlEnd = urlBegin + sizeof(spcServer.url) - 1;
				memcpy(spcServer.url, urlBegin, urlEnd - urlBegin);
				spcServer.url[urlEnd - urlBegin] = '\0';
			}
		}
	} else if (spcServer.contentLen == 0) {
		if (!strncmp(spcServer.req, "Content-Length:", 15))
			spcServer.contentLen = atoi(spcServer.req + 15);
		}
	if ((spcServer.rxPtr == 1)
			|| ((spcServer.rxPtr == 2) && (spcServer.req[0] == '\r')))
		/* blank line, i.e. end of HTTP headers */
		return true;
	else {
		/* The next header line will overwrite the one just parsed. */
		spcServer.rxPtr = 0;
		return false;
	}
}

void spcServerTick(void)
{
	char c;

	if (spcServer.status != SPC_SERVER_IDLE)
		return;
	if (!spcServer.client.connStatus()) {
		spcServer.client = wiFi.server->available();
		if (!spcServer.client.connStatus())
		{
#ifdef SPC_DNS_SERVER
			spcServerDnsTick();
#endif
			return;
		}
		spcServer.connTime = millis();
		spcServer.reqHdr = true;
		spcServer.rxPtr = 0;
		spcServer.method = HTTP_METHOD_UNKNOWN;
		spcServer.contentLen = 0;
	}
	while (spcServer.client.available()) {
		if (spcServer.rxPtr == sizeof(spcServer.req)) {
			/* No space left to parse the request: process what we got so far */
			spcServer.status = SPC_SERVER_RESPONSE;
			break;
		}
		spcServer.req[spcServer.rxPtr++] = spcServer.client.read();
		if (spcServer.reqHdr) {
			if (spcServerParseHdr()) {
				if (spcServer.contentLen == 0) {
					spcServer.status = SPC_SERVER_RESPONSE;
					break;
				} else {
					/* Prepare to receive the request body */
					spcServer.reqHdr = false;
					spcServer.rxPtr = 0;
				}
			}
		} else if (spcServer.rxPtr == spcServer.contentLen) {
			spcServer.status = SPC_SERVER_RESPONSE;
			break;
		}
	}
	if (millis() > (spcServer.connTime + SPC_SERVER_TIMEOUT))
		spcServer.status = SPC_SERVER_RESPONSE;
}
