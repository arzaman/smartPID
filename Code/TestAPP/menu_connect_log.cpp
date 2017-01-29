#include "ABM_Buzzer.h"
#include "delay.h"
#include "log.h"
#include "vsd_GUI.h"
#include "storage_var.h"
#include "WiFi.h"

const char *svLogMode[]={"OFF","WiFi","USB","WiFi+USB"};
const char *svConnectMode[]={"OFF","Client","AP","Auto"};

/* Print a name-value pair, where the value string can occupy more than one line
 * of text in the display.
 * Parameters:
 * - vsdGUI: object that handles the GUI
 * - name: name in the name-value pair
 * - value: value in the name-value pair
 * Returns: none
 */
static void printMultiLine(VSD_GUI &vsdGUI, const char *name, const char *value)
{
	mGUI.setFont(u8g_font_profont11);
	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, 0);
	vsdGUI.printf(name);
	mGUI.setPrintPos(0, 20);
	vsdGUI.print_multiline(value, 0, mGUI.getWidth());
}

/* Print an alert message indicating that the Wi-Fi module has not been found
 * Parameters: none
 * Returns: none
 */
static void alertWiFi(void)
{
	vsdGUI.ClrDisp();
	mGUI.setDefaultForegroundColor();
	vsdGUI.DrawTextCenterFrame(mGUI.getWidth() / 2, 40, "WI-FI NOT PRESENT!");
	Buzzer_wgen(1000, 0, 1);
	delay(1500);
}

static bool logStatusCB(VSD_GUI &vsdGUI, sValueList *lst, uint8_t keypress)
{
	int status;
	const char *val;
	uint8_t w, lineHeight;

	vsdGUI.DrawFrame("Logging Status");
	mGUI.setFont(u8g_font_profont11);
	w = mGUI.getWidth();
	lineHeight = mGUI.getFontLineSpacing() + 1;
	mGUI.setPrintPos(2, 2 + lineHeight);
	vsdGUI.printf("EEPROM Mem");
	status = logEepromMemStatus();
	mGUI.setPrintPos(w - 2, 2 + lineHeight);
	if (status == 0)
		vsdGUI.printf_aright("Empty");
	else if (status == 100)
		vsdGUI.printf_aright("Full");
	else
		vsdGUI.printf_aright("%d%%", status);
	mGUI.setPrintPos(2, 2 + 2 * lineHeight);
	vsdGUI.printf("Server Status");
	if ((wiFi.status != WiFi::STATUS_STA) || !(_stv.mConLogP.LogMode & LOG_WIFI))
		val = "-";
	else
		val = (logServerStatus == LOG_SERVER_STATUS_OK) ? "OK" : "Error";
	mGUI.setPrintPos(w - 2, 2 + 2 * lineHeight);
	vsdGUI.printf_aright(val);
	mGUI.setPrintPos(2, 2 + 3 * lineHeight);
	vsdGUI.printf("Channel Status");
	if ((wiFi.status != WiFi::STATUS_STA) || !(_stv.mConLogP.LogMode & LOG_WIFI)
			|| (logServerStatus != LOG_SERVER_STATUS_OK))
		val = "-";
	else
		val = (logChanStatus == LOG_CHAN_STATUS_OK) ? "OK" : "Error";
	mGUI.setPrintPos(w - 2, 2 + 3 * lineHeight);
	vsdGUI.printf_aright(val);
	return false;
}

sValueList menu_Logging[] = {
		{"Log Mode", (char **)svLogMode, &_stv.mConLogP.LogMode, 0,
				sCOUNT(svLogMode), lSTR_LIST},
		{"Sample Time", NULL, &_stv.mConLogP.SampleTime, 1, 60, lUINT8},
		{"Status", NULL, (void *)logStatusCB, 0, 0, lFULL_SCREEN},
};

static void logModeCheck(int pass)
{
	if (pass == 1) {
		if ((wiFi.status == WiFi::STATUS_NA)
				&& (_stv.mConLogP.LogMode & LOG_WIFI)) {
			alertWiFi();
			_stv.mConLogP.LogMode &= ~LOG_WIFI;
		}
	}
}

static bool menu_draw_logging(void)
{
	vsdGUI.DrawValueList("Logging", menu_Logging, sCOUNT(menu_Logging),
			logModeCheck);
	StoreVarToEEprom();
	return false;
}

static bool wifiSsidCB(VSD_GUI &vsdGUI, sValueList *lst, uint8_t keypress)
{
	char ssid[WiFi::SSID_MAX_LEN + 1];

	if (wiFi.getSSID(ssid) < 0)
		sprintf(ssid, "N/A");
	printMultiLine(vsdGUI, "Wi-Fi SSID", ssid);
	return false;
}

static bool wifiPwdCB(VSD_GUI &vsdGUI, sValueList *lst, uint8_t keypress)
{
	char pwd[WiFi::PWD_MAX_LEN + 1];

	if (wiFi.getPwd(pwd) < 0)
		sprintf(pwd, "N/A");
	printMultiLine(vsdGUI, "Wi-Fi Password", pwd);
	return false;
}

static bool connectStatusCB(VSD_GUI &vsdGUI, sValueList *lst, uint8_t keypress)
{
	uint8_t w, line, lineHeight;
	const char *val;
	IPAddress ipAddr;
	char ssid[WiFi::SSID_MAX_LEN + 1];

	vsdGUI.DrawFrame("Wi-Fi Status");
	mGUI.setFont(u8g_font_profont11);
	w = mGUI.getWidth();
	lineHeight = mGUI.getFontLineSpacing() + 1;
	line = 1;
	switch (_stv.mConLogP.ConnectMode) {
	case WiFi::MODE_OFF:
		val = "Off";
		break;
	case WiFi::MODE_STA:
		val = (wiFi.staStatus == WiFi::STATUS_CONNECTED) ? "Connected"
				: "Disconnected";
		break;
	case WiFi::MODE_AP:
		val = "Access Point";
		break;
	case WiFi::MODE_AUTO:
		if (wiFi.status == WiFi::STATUS_STA)
			val = (wiFi.staStatus == WiFi::STATUS_CONNECTED) ? "Connected"
					: "Disconnected";
		else
			val = "Access Point";
		break;
	}
	mGUI.setPrintPos(2, 2 + line * lineHeight);
	vsdGUI.printf(val);
	if (_stv.mConLogP.ConnectMode == WiFi::MODE_OFF)
		return false;
	line++;
	mGUI.setPrintPos(2, 2 + line * lineHeight);
	vsdGUI.printf("IP");
	ipAddr = wiFi.getIP();
	mGUI.setPrintPos(w - 2, 2 + line * lineHeight);
	vsdGUI.printf_aright(vsdGUI.sb_printf("%d.%d.%d.%d", ipAddr[0], ipAddr[1],
			ipAddr[2], ipAddr[3]));
	line++;
	mGUI.setPrintPos(2, 2 + line * lineHeight);
	vsdGUI.printf("SSID");
	line++;
	wiFi.getSSID(ssid);
	mGUI.setPrintPos(2, 2 + line * lineHeight);
	vsdGUI.print_multiline(ssid, 2, w - 2);
	return false;
}

sValueList menu_WiFi[] = {
		{"Wi-Fi Mode", (char **)svConnectMode, &_stv.mConLogP.ConnectMode, 0,
				sCOUNT(svConnectMode), lSTR_LIST},
		{"Server Port", NULL, &_stv.mConLogP.localServerPort, 0, 0, lUINT16_RO},
		{"SSID", NULL, (void *)wifiSsidCB, 0, 0, lFULL_SCREEN},
		{"Password", NULL, (void *)wifiPwdCB, 0, 0, lFULL_SCREEN},
		{"Status", NULL, (void *)connectStatusCB, 0, 0, lFULL_SCREEN},
};

static void wifiCheck(int pass)
{
	if (pass == 1) {
		if ((wiFi.status == WiFi::STATUS_NA)
				&& (_stv.mConLogP.ConnectMode != WiFi::MODE_OFF)) {
			alertWiFi();
			_stv.mConLogP.ConnectMode = WiFi::MODE_OFF;
		} else
			wiFi.config(_stv.mConLogP.ConnectMode, _stv.mConLogP.SSID,
					_stv.mConLogP.pwd);
	}
}

static bool menu_draw_wifi(void)
{
	vsdGUI.DrawValueList("Wi-Fi", menu_WiFi, sCOUNT(menu_WiFi), wifiCheck);
	StoreVarToEEprom();
	return false;
}

static bool userKeyCB(VSD_GUI &vsdGUI, sValueList *lst, uint8_t keypress)
{
	printMultiLine(vsdGUI, "User API Key", _stv.mConLogP.userKey);
	return false;
}

sValueList menu_Server[] = {
		{"IP", NULL, &_stv.mConLogP.serverIP, 0, 0, lIP_ADDR_RO},
		{"Channel ID", NULL, &_stv.mConLogP.channelID, 0, 0, lUINT32_RO},
		{"User API Key", NULL, (void *)userKeyCB, 0, 0, lFULL_SCREEN},
};

static bool menu_draw_server(void)
{
	vsdGUI.DrawValueList("Server", menu_Server, sCOUNT(menu_Server));
	return false;
}

static vsdMenuStrCb str_LogConnect[] = {
		{"Logging", &menu_draw_logging},
		{"Wi-Fi", &menu_draw_wifi},
		{"Server", &menu_draw_server},
};

static vsdStrCbGroup LogConnectMenu = {
		str_LogConnect,
		sCOUNT(str_LogConnect),
		0
};

bool menu_connect_log (void)
{
	vsdGUI.DrawListMenu("Connectivity/log", &LogConnectMenu);
	return false;
}
