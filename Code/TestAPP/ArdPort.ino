#include "Arduino.h"
#define APP_HWTEST
#if defined(APP_HWTEST)

#include "hwtest.h"

void setup()
{
	hwTestSetup();
}

void loop()
{
	hwTestLoop();
}

#else

#include "ABM_Buttons.h"
#include "ABM_Buzzer.h"
#include "ABM_Cfg.h"
#include <Adafruit_SSD1306.h>
#include <oled_dma.h>
#include <U8glib.h>
#include "log.h"
#include "vsd_GUI.h"
#include "Sensors.h"

#define SPC_SW_MAJOR	0
#define SPC_SW_MINOR	4
#define SPC_SW_BUILD	1

extern void WaitSynchDisplay(void);
extern void setup_lcd_2(void);
extern void SheckForInitStorVar(void);
extern void ButtonsSetup(void);
extern void setupDrivers(void);
extern void setupBuzzer(void);
extern void mainStateMachine1(void);

extern U8GLIB mGUI;
extern unsigned char SmartPID_128x39_1bit_horiz[];

void HelloWindow(void)
{
  uint8_t long_logo_time=0;
  uint32_t start;
  Buzzer_wgen(500,0,1);

  WaitSynchDisplay();
  mGUI.firstPage();
  vsdGUI.ClrDisp();
  mGUI.drawBitmap(0,12,128/8,39,SmartPID_128x39_1bit_horiz);  
  delay(3000);
  if(GetKeyPressF()!=NO_PRESS)
  {
  long_logo_time=1;
  delay(7000);
  }
  WaitSynchDisplay();
  vsdGUI.ClrDisp();

  mGUI.setFont(u8g_font_10x20r);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  mGUI.setPrintPos(63,0);
  vsdGUI.printf_center("SmartPID");

  mGUI.setFont(u8g_font_6x12r);
  mGUI.setPrintPos(63,25);
  vsdGUI.printf_center("SW version %d.%d-%03d", SPC_SW_MAJOR, SPC_SW_MINOR,
    SPC_SW_BUILD);
  mGUI.setPrintPos(63,34);
  vsdGUI.printf_center("HW version SPC1000 V5");
  mGUI.setPrintPos(63,43);
  vsdGUI.printf_center("Serial-%08X",0x12EF2371);
  mGUI.setPrintPos(63,54);
  vsdGUI.printf_center("www.smartpid.com");
  mGUI.setPrintPos(63,63);
  vsdGUI.printf_center("smartpid@arzaman.com");
  start = millis();
  logInit();
  while (millis() - start < 3000) {}
  if(long_logo_time==1)
  {
  delay(7000);
  }
  Buzzer_wgen(50,250,5);
}

void setup() {
  delay(1);

#if defined(USBCON)
  USBDevice.init();
  USBDevice.attach();
#endif

  setup_lcd_2();
  InitOLED_128x64();
  SheckForInitStorVar();
  ButtonsSetup();
  setupDrivers();
  setupBuzzer();
  SensorsSetup();
  HelloWindow();
  pinMode(TEST_PIN_PA13, OUTPUT);
}

void loop() {
  mainStateMachine1();
}

#endif
