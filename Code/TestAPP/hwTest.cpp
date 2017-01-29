/*
 * SmartPID Controller hardware diagnostic tool
 *
 *  Copyright (C) 2016 Arzaman
 */

#include <eeprom.h>
#include <nvm.h>
#include <oled_dma.h>
#include <SparkFunESP8266WiFi.h>
#include <SpiFlash.h>

#include "ABM_Buttons.h"
#include "ABM_Buzzer.h"
#include "ABM_Cfg.h"
#include "ABM_HW_switch.h"
#include "hwTest.h"
#include "Sensors.h"
#include "storage_var.h"
#include "utlts.h"
#include "vsd_GUI.h"
#include "WiFi.h"

#define SAMD21_SERNO_ADDR0	((uint32_t *)0x0080A00C)
#define SAMD21_SERNO_ADDR1	((uint32_t *)0x0080A040)
#define SAMD21_SERNO_ADDR2	((uint32_t *)0x0080A044)
#define SAMD21_SERNO_ADDR3	((uint32_t *)0x0080A048)

#define SPC_SW_MAJOR	2
#define SPC_SW_MINOR	0
#define SPC_SW_BUILD	1

#define HWTEST_NVM_PAGE_SIZE	64

extern void ButtonsSetup(void);
extern void setup_lcd_2(void);
extern void setupBuzzer(void);
extern void setupDrivers(void);

extern unsigned char SmartPID_128x39_1bit_horiz[];

/* Retrieve and display information on the MCU
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestMCU(void)
{
	uint8_t serNoBuf[HWTEST_NVM_PAGE_SIZE];
	struct nvm_config config;
	enum status_code status;
	uint8_t lineHeight = mGUI.getFontLineSpacing();
	int y;
	int i;

	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, y = 0);
	vsdGUI.printf("MCU Clock: %ld MHz", SystemCoreClock / 1000000);
	((uint32_t *)serNoBuf)[0] = *SAMD21_SERNO_ADDR0;
	((uint32_t *)serNoBuf)[1] = *SAMD21_SERNO_ADDR1;
	((uint32_t *)serNoBuf)[2] = *SAMD21_SERNO_ADDR2;
	((uint32_t *)serNoBuf)[3] = *SAMD21_SERNO_ADDR3;
	mGUI.setPrintPos(0, y += lineHeight + 2);
	vsdGUI.printf("MCU Serial Number:");
	mGUI.setPrintPos(0, y += lineHeight);
	for (i = 0; i < 8; i++) {
		vsdGUI.printf("%02X", serNoBuf[i]);
	}
	mGUI.setPrintPos(0, y += lineHeight);
	for (; i < 16; i++) {
		vsdGUI.printf("%02X", serNoBuf[i]);
	}
	nvm_get_config_defaults(&config);
	do {
		status = nvm_set_config(&config);
	} while (status == STATUS_BUSY);
	status = nvm_read_buffer(HW_SERIAL_NUM_ADDR & ~(HWTEST_NVM_PAGE_SIZE - 1),
			serNoBuf, sizeof(serNoBuf));
	mGUI.setPrintPos(0, y += lineHeight + 2);
	if (status == STATUS_OK) {
		uint8_t *serNoPtr;

		vsdGUI.printf("Board Serial Number:");
		serNoPtr = serNoBuf + (HW_SERIAL_NUM_ADDR & (HWTEST_NVM_PAGE_SIZE - 1));
		mGUI.setPrintPos(0, y += lineHeight);
		for (i = 0; i < HW_SERIAL_NUM_LEN; i++) {
			vsdGUI.printf("%02X", serNoPtr[i]);
		}
	}
	else if (status == STATUS_BUSY) {
		vsdGUI.printf("Cannot read serial number: NVM busy");
	}
	return (GetKeyPressF() != NO_PRESS);
}

/* Test the internal RWW EEPROM
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestRWWEeprom(void)
{
	uint8_t eeBuf[EEPROM_PAGE_SIZE];
	struct eeprom_emulator_parameters ee_params;
	uint8_t lineHeight = mGUI.getFontLineSpacing();
	int y;
	int i;
	int j;

	configure_eeprom();
	eeprom_emulator_get_parameters(&ee_params);
	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, y = 0);
	vsdGUI.printf("Internal EEPROM");
	mGUI.setPrintPos(3, y += lineHeight + 2);
	vsdGUI.printf("Write Test");
	memset(eeBuf, 0, sizeof(eeBuf));
	for (i = 0; i < ee_params.eeprom_number_of_pages; i++) {
		eeprom_emulator_write_page(i, eeBuf);
	}
	for (i = 0; i < ee_params.eeprom_number_of_pages; i++) {
		eeprom_emulator_read_page(i, eeBuf);
		for (j = 0; j < sizeof(eeBuf); j++) {
			if (eeBuf[j] != 0) {
				break;
			}
		}
		if (j < sizeof(eeBuf)) {
			break;
		}
	}
	if (i < ee_params.eeprom_number_of_pages) {
		mGUI.setPrintPos(3, y += lineHeight);
		vsdGUI.printf(" error at page %d", i);
		goto error;
	}
	else {
		vsdGUI.printf(" OK");
	}
	mGUI.setPrintPos(3, y += lineHeight + 2);
	vsdGUI.printf("Erase Test");
	eeprom_emulator_erase_memory();
	eeprom_emulator_init();
	for (i = 0; i < ee_params.eeprom_number_of_pages; i++) {
		eeprom_emulator_read_page(i, eeBuf);
		for (j = 0; j < sizeof(eeBuf); j++) {
			if (eeBuf[j] != 0xFF) {
				break;
			}
		}
		if (j < sizeof(eeBuf)) {
			break;
		}
	}
	if (i < ee_params.eeprom_number_of_pages) {
		mGUI.setPrintPos(3, y += lineHeight);
		vsdGUI.printf(" error at page %d", i);
		goto error;
	}
	else {
		vsdGUI.printf(" OK");
	}
	mGUI.setPrintPos(mGUI.getWidth() / 2, 50);
	vsdGUI.printf_center("Internal EEPROM OK");
	goto done;
error:
	mGUI.setPrintPos(mGUI.getWidth() / 2, 50);
	vsdGUI.printf_center("EEPROM Test Failed");
done:
	ClearAllKeyPress();
	do {
	} while (GetKeyPressF() == NO_PRESS);
	return true;
}

/* Test the external SPI flash
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestSpiFlash(void)
{
	uint8_t flashBuf[128];
	uint32_t flashAddr;
	uint32_t flashSize = spiFlash.memSize();
	uint32_t eraseSize = spiFlash.eraseSize();
	uint8_t lineHeight = mGUI.getFontLineSpacing();
	int y;
	int i;

	spiFlash.init(SPI, PIN_SPI_SS);
	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, y = 0);
	vsdGUI.printf("External EEPROM");
	mGUI.setPrintPos(3, y += lineHeight + 2);
	vsdGUI.printf("Write Test ");
	memset(flashBuf, 0, sizeof(flashBuf));
	for (flashAddr = 0; flashAddr < flashSize; flashAddr += sizeof(flashBuf)) {
		spiFlash.write(flashAddr, flashBuf, sizeof(flashBuf));
		if ((flashAddr != 0) && !(flashAddr & (flashSize / 4 - 1))) {
			vsdGUI.printf(".");
		}
	}
	vsdGUI.printf(".");
	for (flashAddr = 0; flashAddr < flashSize; flashAddr += sizeof(flashBuf)) {
		spiFlash.read(flashAddr, flashBuf, sizeof(flashBuf));
		for (i = 0; i < sizeof(flashBuf); i++) {
			if (flashBuf[i] != 0) {
				break;
			}
		}
		if (i < sizeof(flashBuf)) {
			break;
		}
		if ((flashAddr != 0) && !(flashAddr & (flashSize / 4 - 1))) {
			vsdGUI.printf(".");
		}
	}
	if (flashAddr < flashSize) {
		mGUI.setPrintPos(3, y += lineHeight);
		vsdGUI.printf(" error at 0x%05X", flashAddr + i);
		mGUI.setPrintPos(mGUI.getWidth() / 2, 50);
		vsdGUI.printf_center("EEPROM Test Failed");
		goto done;
	}
	else {
		vsdGUI.printf(" OK");
	}
	mGUI.setPrintPos(3, y += lineHeight + 2);
	vsdGUI.printf("Erase Test ");
	for (flashAddr = 0; flashAddr < flashSize; flashAddr += eraseSize) {
		spiFlash.erase(flashAddr);
		if ((flashAddr != 0) && !(flashAddr & (flashSize / 4 - 1))) {
			vsdGUI.printf(".");
		}
	}
	vsdGUI.printf(".");
	for (flashAddr = 0; flashAddr < flashSize; flashAddr += sizeof(flashBuf)) {
		spiFlash.read(flashAddr, flashBuf, sizeof(flashBuf));
		for (i = 0; i < sizeof(flashBuf); i++) {
			if (flashBuf[i] != 0xFF) {
				break;
			}
		}
		if (i < sizeof(flashBuf)) {
			break;
		}
		if ((flashAddr != 0) && !(flashAddr & (flashSize / 4 - 1))) {
			vsdGUI.printf(".");
		}
	}
	if (flashAddr < flashSize) {
		mGUI.setPrintPos(3, y += lineHeight);
		vsdGUI.printf(" error at 0x%05X", flashAddr + i);
		mGUI.setPrintPos(mGUI.getWidth() / 2, 50);
		vsdGUI.printf_center("EEPROM Test Failed");
		goto done;
	}
	else {
		vsdGUI.printf(" OK");
	}
	mGUI.setPrintPos(mGUI.getWidth() / 2, 50);
	vsdGUI.printf_center("External EEPROM OK");
done:
	ClearAllKeyPress();
	do {
	} while (GetKeyPressF() == NO_PRESS);
	return true;
}

/* Test the internal RWW EEPROM and the external SPI flash
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestEeprom(void)
{
	if (!hwTestRWWEeprom()) {
		return false;
	}
	vsdGUI.ClrDisp();
	return hwTestSpiFlash();
}

/* Print alphabet letters on the screen, up to the screen width limit
 * Parameters: none
 * Returns: none
 */
static void hwTestPrintAlphabet(void)
{
	char letter[2];

	letter[0] = 'A';
	letter[1] = '\0';
	do {
		vsdGUI.printf(letter);
		letter[0]++;
	} while (mGUI.getPrintCol() < mGUI.getWidth() - mGUI.getStrWidth(letter));
}

/* Test the OLED display
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestOLED(void)
{
	extern uint8_t oled_buffer[];
	int i;
	uint8_t pixelPattern;
	const u8g_fntpgm_uint8_t *fonts[] = {
			u8g_font_profont22,
			u8g_font_10x20r,
			u8g_font_courB10,
			u8g_font_6x12r,
			u8g_font_5x8,
	};
	int y;

	for (i = 0; i < 8; i++) {
		WaitSynchDisplay();
		memset(oled_buffer, 1 << i, mGUI.getWidth() * mGUI.getHeight() / 8);
		delay(1000);
	}
	pixelPattern = 0;
	for (i = 0; i < 8; i++) {
		pixelPattern |= 1 << i;
		WaitSynchDisplay();
		memset(oled_buffer, pixelPattern,
				mGUI.getWidth() * mGUI.getHeight() / 8);
		delay(1000);
	}
	vsdGUI.ClrDisp();
	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, y = 0);
	i = 0;
	for (i = 0; i < sCOUNT(fonts); i++) {
		mGUI.setFont(fonts[i]);
		mGUI.setFontPosTop();
		if ((mGUI.getPrintRow() >=
				mGUI.getHeight() - mGUI.getFontLineSpacing())) {
			break;
		}
		hwTestPrintAlphabet();
		mGUI.setPrintPos(0, y += mGUI.getFontLineSpacing());
	}
	ClearAllKeyPress();
	do {
	} while (GetKeyPressF() == NO_PRESS);
	return true;
}

/* Draw a rectangle indicating the state of a button
 * If the button is pressed, the drawn rectangle is filled, otherwise the
 * rectangle is outlined.
 * Parameters:
 * - button: button for which the rectangle is drawn
 * - x: x coordinate of left border of rectangle
 * - y: y coordinate of upper border of rectangle
 * - w: rectangle width, in pixels
 * - h: rectangle height, in pixels
 * Returns: none
 */
static void hwTestDrawButton(uint8_t button, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h)
{
	uint8_t kb = GetAllKeyPress();

	if (kb & (1 << button)) {
		mGUI.drawBox(x, y, w, h);
	}
	else {
		mGUI.drawFrame(x, y, w, h);
	}
}

/* Test the hardware buttons
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestButtons(void)
{
	unsigned int w = mGUI.getWidth();
	unsigned int h = mGUI.getHeight();

	hwTestDrawButton(BUTTON_UP, w / 8, h / 8, w / 4, h / 4);
	hwTestDrawButton(BUTTON_DOWN, w / 8, 5 * h / 8, w / 4, h / 4);
	hwTestDrawButton(BUTTON_ENTER, 5 * w / 8, h / 8, w / 4, h / 4);
	hwTestDrawButton(BUTTON_SS, 5 * w / 8, 5 * h / 8, w / 4, h / 4);
	return (GetKeyPressL() == BUTTON_SS_LP);
}

/* Test the buzzer
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestBuzzer(void)
{
	mGUI.setFontPosBottom();
	mGUI.setPrintPos(mGUI.getWidth() / 2, mGUI.getHeight() / 2);
	vsdGUI.printf_center("Buzzer Test");
	Buzzer_wgen(100, 100, -1);
	do {
	} while (GetKeyPressF() == NO_PRESS);
	return true;
}

/* Test the Wi-Fi module
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestWiFi(void)
{
	unsigned int standardBaudRates[] = {
			115200,
			57600,
			38400,
			19200,
			9600,
	};
	char ATversion[64];
	char SDKversion[64];
	uint8_t lineHeight = mGUI.getFontLineSpacing() + 1;
	int y;
	unsigned int baud;
	int i;

	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, y = 0);
	vsdGUI.printf("Discovering module ");
	for (i = 0; i < sCOUNT(standardBaudRates); i++) {
		if (esp8266.begin(Serial1, standardBaudRates[i])) {
			break;
		}
		if ((i == 0) || (i == 2)) {
			vsdGUI.printf(".");
		}
	}
	mGUI.setPrintPos(0, y += lineHeight);
	if (i == sCOUNT(standardBaudRates)) {
		for (baud = 300, i = 0; baud <= 4608000; baud += (baud >> 3), i++) {
			if (esp8266.begin(Serial1, baud)) {
				break;
			}
			if (!(i & 0x3)) {
				vsdGUI.printf(".");
			}
		}
		mGUI.setPrintPos(0, y += lineHeight);
		if (baud <= 4608000) {
			vsdGUI.printf("Found at %d bps", baud);
			baud = WiFi::BAUD_RATE_DEFAULT;
			esp8266.setBaud(baud);
			mGUI.setPrintPos(0, y += lineHeight);
			vsdGUI.printf("Speed set to %d", baud);
			delay(1000);
			return false;
		}
		else {
			vsdGUI.printf("Wi-Fi module not");
			mGUI.setPrintPos(0, y += lineHeight);
			vsdGUI.printf("found");
			goto done;
		}
	}
	else {
		vsdGUI.printf("Found at %d bps", standardBaudRates[i]);
	}
	mGUI.setPrintPos(0, y += lineHeight);
	if (esp8266.getVersion(ATversion, SDKversion) < 0) {
		vsdGUI.printf("Cannot get firmware");
		mGUI.setPrintPos(0, y += lineHeight);
		vsdGUI.printf("version info");
	}
	else {
		/* Remove build time information from the AT version string, otherwise
		 * it doesn't fit in the screen. */
		for (i = 0; ATversion[i] != '\0'; i++) {
			if (ATversion[i] == '(') {
				ATversion[i] = '\0';
				break;
			}
		}

		vsdGUI.printf("AT version: %s", ATversion);
		mGUI.setPrintPos(0, y += lineHeight);
		vsdGUI.printf("SDK version: %s", SDKversion);
	}
done:
	ClearAllKeyPress();
	do {
	} while (GetKeyPressF() == NO_PRESS);
	return true;
}

/* Test the temperature probes
 * Parameters:
 * - probeType: enumerated value indicating the type of temperature probes
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestTempProbes(enum e_sens_mode probeType)
{
	unsigned int sampleTime = 1000;	/* in milliseconds */

	_stv.ProcPar.s_time = sampleTime;
	if ((_stv.HW_var.probe1_mode != probeType)
			|| (_stv.HW_var.probe2_mode != probeType)) {
		_stv.HW_var.probe1_mode = _stv.HW_var.probe2_mode = probeType;
		delay(sampleTime);
	}
	mGUI.setFontPosTop();
	mGUI.setPrintPos(0, 0);
	vsdGUI.printf("Temp Probe 1: ");
	if ((sensor1_fval > -50) && (sensor1_fval < 200)) {
		vsdGUI.printf("%0.1f°C", sensor1_fval);
	}
	else {
		vsdGUI.printf("N/A");
	}
	mGUI.setPrintPos(0, mGUI.getFontLineSpacing() + 1);
	vsdGUI.printf("Temp Probe 2: ");
	if ((sensor2_fval > -50) && (sensor2_fval < 200)) {
		vsdGUI.printf("%0.1f°C", sensor2_fval);
	}
	else {
		vsdGUI.printf("N/A");
	}
	return (GetKeyPressF() != NO_PRESS);
}

/* Test the NTC temperature probes
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestNTC(void)
{
	return hwTestTempProbes(SENSOR_NTC);
}

/* Test the DS18B20 temperature probes
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestDS18B20(void)
{
	return hwTestTempProbes(SENSOR_ONEWIRE);
}

/* Test the board outputs (relays and DC outputs)
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestOutputs(void)
{
	static uint8_t curOutput = 0;
	uint8_t kb;

	DrawOutputs(55, 8);
	mGUI.drawStr(50, 8 + curOutput * 8, ">");
	kb = GetKeyPressF();
	switch (kb) {
	case BUTTON_UP:
		DecVal(&curOutput, N_DRIVERS);
		break;
	case BUTTON_DOWN:
		IncVal(&curOutput, N_DRIVERS);
		break;
	case BUTTON_ENTER:
		ToggleDriveSwitch(curOutput);
		break;
	case BUTTON_SS:
		return true;
	}
	return false;
}

/* Test USB connection
 * Parameters: none
 * Returns: true if this test is completed and its output should not be
 * displayed anymore on the screen, false otherwise
 */
static bool hwTestUSB(void)
{
	uint16_t f;

	mGUI.setFontPosBottom();
	mGUI.setPrintPos(mGUI.getWidth() / 2, mGUI.getHeight() / 2);

	/* Count frame numbers to determine whether we are connected to a USB host
	 */
	f = USB->DEVICE.FNUM.bit.FNUM;
	delay(3);
	if (f == USB->DEVICE.FNUM.bit.FNUM) {
		vsdGUI.printf_center("Connect USB Port");
	}
	else {
		vsdGUI.printf_center("USB Port Connected");
	}

	return (GetKeyPressF() != NO_PRESS);
}

static struct hwTest {
	const char *name;
	bool (*exec)(void);
} hwTests[] = {
		{"MCU", hwTestMCU},
		{"EEPROM", hwTestEeprom},
		{"OLED", hwTestOLED},
		{"Buttons", hwTestButtons},
		{"Buzzer", hwTestBuzzer},
		{"Wi-Fi", hwTestWiFi},
		{"NTC Probes", hwTestNTC},
		{"DS18B20 Probes", hwTestDS18B20},
		{"Outputs", hwTestOutputs},
		{"USB", hwTestUSB},
};

static struct hwTestState {
	bool testOngoing;
	unsigned int nextTest;
} hwTestState;

/* Callback function called when the user starts a test
 * Parameters: none
 * Returns: false
 */
static bool hwTestGo(void)
{
	hwTestState.testOngoing = true;
	return false;
}

/* Callback function called when the user skips a test
 * Parameters: none
 * Returns: false
 */
static bool hwTestSkip(void)
{
	hwTestState.nextTest++;
	return false;
}

static vsdMenuStrCb hwTestNavStrCbs[] = {
		{"Go", &hwTestGo},
		{"Skip", &hwTestSkip},
};

static vsdStrCbGroup hwTestNavigation = {
		hwTestNavStrCbs,
		sCOUNT(hwTestNavStrCbs),
		0
};

void hwTestSetup() {
#if defined(USBCON)
	USBDevice.init();
	USBDevice.attach();
#endif
	setup_lcd_2();
	InitOLED_128x64();
	ButtonsSetup();
	setupDrivers();
	setupBuzzer();
	SensorsSetup();
	Buzzer_wgen(500, 0, 1);
	WaitSynchDisplay();
	mGUI.firstPage();
	vsdGUI.ClrDisp();
	mGUI.drawBitmap(0, 12, 128 / 8, 39, SmartPID_128x39_1bit_horiz);
	delay(3000);
}

void hwTestLoop(void)
{
	unsigned int xCenter = mGUI.getWidth() / 2;
	WaitSynchDisplay();
	vsdGUI.ClrDisp();

	if (!hwTestState.testOngoing) {
		uint8_t kb;

		mGUI.firstPage();
		mGUI.setFont(u8g_font_6x12r);
		mGUI.setFontRefHeightText();
		mGUI.setFontPosTop();
		if (hwTestState.nextTest == sCOUNT(hwTests)) {
			vsdGUI.DrawTextCenterFrame(xCenter, 40, "Tests Completed");
			if (GetKeyPressL() == BUTTON_SS_LP) {
				hwTestState.nextTest = 0;
			}
			return;
		}
		mGUI.setPrintPos(xCenter, 0);
		vsdGUI.printf_center("Test SW v%d.%d-%03d", SPC_SW_MAJOR, SPC_SW_MINOR,
				SPC_SW_BUILD);
		mGUI.setPrintPos(xCenter, 20);
		vsdGUI.printf_center("%s", hwTests[hwTestState.nextTest].name);
		vsdGUI.DrawGroupBoxCentr(&hwTestNavigation, xCenter, 50);
		kb = GetKeyPressF();
		switch (kb) {
		case BUTTON_UP:
			DecVal(&(hwTestNavigation.curr_indx), hwTestNavigation.count);
			break;
		case BUTTON_DOWN:
			IncVal(&(hwTestNavigation.curr_indx), hwTestNavigation.count);
			break;
		case BUTTON_ENTER:
			hwTestNavigation.StrCb[hwTestNavigation.curr_indx].cbFunc();
			break;
		}
	}
	else {
		if (hwTests[hwTestState.nextTest].exec()) {
			hwTestState.testOngoing = false;
			hwTestState.nextTest++;
		}
	}
}
