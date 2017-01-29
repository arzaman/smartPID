#include "ABM_Buttons.h"
#include "log.h"
#include "Sensors.h"
#include "PID_control.h"

#include "storage_var.h"
#include "vsd_GUI.h"
#include "utlts.h"
#include "run_mode.h"
#include "spcServer.h"

//extern Adafruit_SSD1306 display;
extern void WaitSynchDisplay(void);
//extern u8g_t u8g;
extern U8GLIB mGUI;
extern void draw_root_menu(void);
extern volatile uint32_t _seconds_timer;
//extern uint8_t /*sensor1_mode,*/ sensor2_mode;

extern void DrawProc(void);

//extern struct sProcStat mProcStat;

int test_count=0;
int timer_N1=0;

uint8_t indx_menu_n1;

const char deg_C[]="°C";

const char *str_MenuN1[3]={
  "Configuration",
  "Connectivity/log",
  "RUN mode"
};

vsdStringGroup gr_M1={
	(char**)&str_MenuN1,
	sizeof(str_MenuN1)/sizeof(str_MenuN1[0]),
	0
};

extern void MenuN1_SW_setup(void);
extern bool menu_Configuration(void);
extern bool menu_connect_log(void);
extern bool menu_recipe(void);
extern bool menu_run(void);
extern bool menu_status(void);

vsdMenuStrCb str_cbMenuN1[4]={
  {"Status Mode", &menu_status},
  {"Configuration", &menu_Configuration},
  {"Connectivity/log", &menu_connect_log},
  {"RUN Mode", &menu_run}
};


vsdStrCbGroup cbMenuN1={
  (vsdMenuStrCb*)str_cbMenuN1,
  sizeof(str_cbMenuN1)/sizeof(str_cbMenuN1[0]),
  0
};

void rem_MenuN1(void)
{
uint8_t kb;
kb=GetKeyPressF();
if(kb==BUTTON_UP)
	DecVal(&gr_M1.curr_indx, gr_M1.count);
else if(kb==BUTTON_DOWN)
	IncVal(&gr_M1.curr_indx, gr_M1.count);
else if(kb==BUTTON_ENTER) {}
mGUI.setFont(u8g_font_6x12r);
vsdGUI.DrawGroupBoxCentr(&gr_M1,74,32);
}

void MenuN1_SW_setup(void)
{
  WaitSynchDisplay();
//  mGUI.firstPage();
vsdGUI.ClrDisp();
mGUI.setFont(u8g_font_6x12);
mGUI.setPrintPos(5,8);

	vsdGUI.printf("SW_setup");
  delay(1000);
}

void MenuN1(void)
{
uint8_t kb;
kb=GetKeyPressL();
if(kb==BUTTON_UP)
	DecVal(&cbMenuN1.curr_indx, cbMenuN1.count);
else if(kb==BUTTON_DOWN)
	IncVal(&cbMenuN1.curr_indx, cbMenuN1.count);
else if(kb==BUTTON_ENTER){
	if(cbMenuN1.StrCb[cbMenuN1.curr_indx].cbFunc!=NULL)
			cbMenuN1.StrCb[cbMenuN1.curr_indx].cbFunc();
    }
mGUI.setFont(u8g_font_7x13r);
vsdGUI.DrawGroupBoxCentr(&cbMenuN1,63,26+5);

mGUI.setFont(u8g_font_8x13Br);
mGUI.setPrintPos(0,10);
vsdGUI.printf("SmartPID");

}


void DrawSeparateLine_N1_1(void)
{
  vsdGUI.SetBlinkPixelCount(0);
  vsdGUI.SetBlinkPixelStyle(lsDASHED_21);
  vsdGUI.DrawLineHRight(0,51,128);
}

void DrawDegVal(float term, int n)
{
if(n<1 || n>2) return;
vsdGUI.printf_n("T%d",n);
if(term<1000)  vsdGUI.printf(" %0.1f°C",term);
else vsdGUI.printf(" (fails)");
}

void DrawTempN1(void)
{
  mGUI.setFont(u8g_font_6x12);
////  mGUI.setFont(u8g_font_6x10);
//  mGUI.setFont(u8g_font_fixed_v0);
//  mGUI.setFont(u8g_font_courB10);
mGUI.setPrintPos(68,62);
if(_stv.HW_var.probe1_mode!=0 && _stv.HW_var.probe2_mode!=0)
{
if((_seconds_timer%10)<5) {DrawDegVal(sensor1_fval,1);}
else {DrawDegVal(sensor2_fval,2);}
}
else if(_stv.HW_var.probe1_mode!=0) {DrawDegVal(sensor1_fval,1);}
else if(_stv.HW_var.probe2_mode!=0) {DrawDegVal(sensor2_fval,2);}
else {vsdGUI.printf_n("T"); vsdGUI.printf(" all OFF");}
}
#if 0
void DrawTempN1(void)
{
  mGUI.setFont(u8g_font_6x12);
////  mGUI.setFont(u8g_font_6x10);
//  mGUI.setFont(u8g_font_fixed_v0);
//  mGUI.setFont(u8g_font_courB10);
mGUI.setPrintPos(68,62);
if(sensor1_mode!=0 && sensor2_mode!=0)
{
if((_seconds_timer%10)<5) vsdGUI.printf("T1 %0.1f°C",sensor1_fval);
else vsdGUI.printf("T2 %0.1f°C",sensor2_fval);
}
else if(sensor1_mode!=0) vsdGUI.printf("T1 %0.1f°C",sensor1_fval);
else if(sensor2_mode!=0) vsdGUI.printf("T2 %0.1f°C",sensor1_fval);
else vsdGUI.printf("T all OFF");
}

void DrawTempN1(void)
{
  mGUI.setFont(u8g_font_6x12);
////  mGUI.setFont(u8g_font_6x10);
//  mGUI.setFont(u8g_font_fixed_v0);
//  mGUI.setFont(u8g_font_courB10);
  mGUI.setPrintPos(38,63);
  vsdGUI.printf("%0.1f°C",sensor1_fval);
  //  /*mGUI.print("t1-");*/ mGUI.print(sensor1_fval,1); mGUI.print(deg_C);
  mGUI.setPrintPos(86,63);
  vsdGUI.printf("%0.1f°C",sensor2_fval);
//  /*mGUI.print("t2-");*/ mGUI.print(sensor2_fval,1); mGUI.print(deg_C);
}
#endif
void DrawTime(void)
{
//mGUI.setFont(u8g_font_courB10);
//mGUI.setPrintPos(1,63);
//vsdGUI.printf("%d:%02d",_seconds_timer/60,_seconds_timer%60);
//timer_N1++;
vsdGUI.DrawTimeHMS(1,65,_seconds_timer,u8g_font_freedoomr10r);
}

void DrawProcessStatus(void)
{
  mGUI.setFont(u8g_font_6x12);
  mGUI.setPrintPos(4,8);
  vsdGUI.printf("%s",mProcStat.enable?"in process":"stop process");
}

void DrawStatus(void);

void RunBgTasks(void)
{
  spcServerProcess();
  if (mProcStat.enable == 1) {
    logProcess();
  }
}

void mainStateMachine1(void)
{
  WaitSynchDisplay();
  mGUI.firstPage();

if(mProcStat.enable==0)
{
  mGUI.setFont(u8g_font_5x7);
//  mGUI.setFont(u8g_font_micro);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  mGUI.setPrintPos(0,3);
//  mGUI.print("d-"); mGUI.print(sizeof(double)); mGUI.print(" f-"); mGUI.print(sizeof(float));
//  mGUI.setPrintPos(30,50);
//  mGUI.setFont(u8g_font_6x12);
//  mGUI.setFont(u8g_font_6x10);
//  mGUI.setFont(u8g_font_courB10);
//  mGUI.print("temp1-"); mGUI.print((int16_t)(temp_sens1[0] | temp_sens1[1]<<8)>>4); mGUI.print(deg_C);
//  mGUI.print("temp1-"); mGUI.print(sensor1_fval,1); mGUI.print(deg_C);
//  mGUI.setPrintPos(30,60);
//  mGUI.print("temp2-"); mGUI.print((int16_t)(temp_sens2[0] | temp_sens2[1]<<8)>>4); mGUI.print(deg_C);
//  mGUI.print("temp2-"); mGUI.print(sensor2_fval,1); mGUI.print(deg_C);

//  vsdGUI.DrawTextCenterFrame(128/2,20,"centr",(enum line_style)((test_count++)%(lsDASHED_32+1)),60);
//  gr_M1.curr_indx=(gr_M1.curr_indx+1)%4;
//  MenuN1();
//  DrawTempN1();
//  DrawTime();
//  DrawSeparateLine_N1_1();

//  DrawOutputs();
  MenuN1();
//  DrawProcessStatus();
}
else if(mProcStat.enable==1)
{
DrawProc();
}
else
{
DrawStatus();
}
RunBgTasks();
}
