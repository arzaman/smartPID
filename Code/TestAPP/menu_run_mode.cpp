#include "vsd_GUI.h"
#include "run_mode.h"
#include "small_log.h"
#include <U8glib.h>
#include "ABM_Buttons.h"
#include "ABM_Buzzer.h"
#include "ABM_HW_switch.h"
#include "log.h"
#include "storage_var.h"
#include "utlts.h"
#include "PID_control.h"
#include "Sensors.h"
#include "WiFi.h"

#define SMALL_LOG_CHART_HEIGHT 52
#define SMALL_LOG_OFFSET_X  15
#define SMALL_LOG_OFFSET_Y  1

extern uint32_t down_timer1,down_timer2;
extern uint32_t up_timer1,up_timer2;

uint16_t wait_blick;
uint8_t timer_mode=0;

/* Do preliminary operations for displaying a user message
 * This function draws a frame centered on the screen, in which the user message
 * is supposed to be displayed later by the calling code.
 *
 * Parameters:
 * - frame_height: height of the frame to be displayed, in pixels
 *
 * Return: none
 */
static void PrepareDrawMess(unsigned int frame_height)
{
  WaitSynchDisplay();
  vsdGUI.ClrDisp();
  mGUI.drawFrame(0, 32 - frame_height / 2, 128, frame_height);
  mGUI.setFont(u8g_font_6x12);
}

//******************************************************************************
void PrintUpDownTimer(uint8_t dir)
{
uint16_t x,y;
    x=mGUI.getPrintCol();
	y=mGUI.getPrintRow();
    mGUI.setFont(u8g_font_6x12_75r);
    mGUI.setPrintPos(x,y-2);
    if(dir==0) mGUI.write((char)0x3f); else mGUI.write((char)0x35);
}
//******************************************************************************
void DrawHeatOrCooling2(int n);

void DrawShortVisualProc(int n)
{
uint16_t x,y;
  //  mGUI.setFont(u8g_font_courB10);
  mGUI.setFont(u8g_font_6x12);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  mGUI.setPrintPos(1,0);
//  vsdGUI.printf_n("PID-%d",n+1);
  vsdGUI.printf_n("CH-%d",n+1);
  
  //  mGUI.setFont(u8g_font_6x12);
  //  mGUI.setFont(u8g_font_courB10);
  //  mGUI.setFont(u8g_font_6x13B);
  //  mGUI.setFont(u8g_font_9x18B);
//  mGUI.setFont(u8g_font_profont22);
  if(n==0)
  {
    //  vsdGUI.printf("%0.1f°C->%0.1f°C",*(PID_n1.myInput),PID_n1.Setpoint);
    mGUI.setFont(u8g_font_6x12);
    mGUI.setPrintPos(1,19);
    vsdGUI.printf("T1");
    mGUI.setPrintPos(1,37);
    vsdGUI.printf("SP");
    
    mGUI.setFont(u8g_font_profont22);
    mGUI.setPrintPos(103,30);
    vsdGUI.printf_aright("%0.1f°C",*(PID_n1.myInput));
    mGUI.setPrintPos(103,48);
    vsdGUI.printf_aright("%0.1f°C",PID_n1.Setpoint);
    
    if(timer_mode==0){
    vsdGUI.DrawTimeHMS(37,12,
                       (down_timer1!=0)?down_timer1:_stv.ProcPar.timer1_preset,
                       u8g_font_freedoomr10r);
    }
    else{
      vsdGUI.DrawTimeHMS(37,12,up_timer1,u8g_font_freedoomr10r);
    }
    PrintUpDownTimer(timer_mode);
  }
  else
  {
    //  vsdGUI.printf("%0.1f°C->%0.1f°C",*(PID_n2.myInput),PID_n2.Setpoint);
    mGUI.setFont(u8g_font_6x12);
    mGUI.setPrintPos(1,19);
    vsdGUI.printf("T2");
    mGUI.setPrintPos(1,37);
    vsdGUI.printf("SP");

    mGUI.setFont(u8g_font_profont22);
    mGUI.setPrintPos(103,30);
    vsdGUI.printf_aright("%0.1f°C",*(PID_n2.myInput));
    mGUI.setPrintPos(103,48);
    vsdGUI.printf_aright("%0.1f°C",PID_n2.Setpoint);
    
//    DrawTimerStatus(n);
    
    if(timer_mode==0){
    vsdGUI.DrawTimeHMS(37,12,
                       (down_timer2!=0)?down_timer2:_stv.ProcPar.timer2_preset,
                       u8g_font_freedoomr10r);
    }
    else{
      vsdGUI.DrawTimeHMS(37,12,up_timer2,u8g_font_freedoomr10r);
    }
    PrintUpDownTimer(timer_mode);
  }
  if ((wiFi.status == WiFi::STATUS_STA)
      && (wiFi.staStatus == WiFi::STATUS_CONNECTED)) {
    mGUI.setFont(u8g_font_04b_03);
    mGUI.setPrintPos(92, 5);
    mGUI.print("Wi-Fi");
    if ((_stv.mConLogP.LogMode & LOG_WIFI)
        && (logServerStatus == LOG_SERVER_STATUS_OK)
        && (logChanStatus == LOG_CHAN_STATUS_OK)) {
      mGUI.setPrintPos(115, 5);
      mGUI.print("Log");
    }
  }
  DrawOutputs(110,10);
  DrawHeatOrCooling2(n);
  wait_blick++;
}
//******************************************************************************
void CalculateMinMaxRound(uint16_t *min, uint16_t *max, sLogSample *slog, uint16_t t_point=0)
{
  uint16_t tmin,tmax;
  tmin=0xffff;
  tmax=0;
  for(int i=0;i<SM_LOG_LEN;i++)
  {
	if(slog->t[i]!=SML_EMPTY_VAL){
	  if(slog->t[i]>tmax) tmax=slog->t[i];
	  if(slog->t[i]<tmin) tmin=slog->t[i];
	}
  }
  if(t_point!=0)
  {
	if(t_point>tmax) tmax=t_point;
	if(t_point<tmin) tmin=t_point;
  }
  
  tmax+=100-((tmax%100)==0?100:(tmax%100));
  tmin-=tmin%100;
  
  if(tmax==tmin)
  {
	tmax+=100;
	tmin-=100;
  }
  
  *min=tmin;
  *max=tmax;
}
//******************************************************************************
void DrawChartBasic(uint8_t n)
{
  sLogSample *dl;
  int16_t pr_y=1024,cur_y;
  uint16_t min,max,t_point,t_input;
  float t_fpoint,t_finput;
  
  mGUI.setFont(u8g_font_5x7);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  
  
  dl=(n==0)?&(SmLog.dSl.data1):&(SmLog.dSl.data2);
  
  t_fpoint=(n==0)?*PID_n1.mySetpoint:*PID_n2.mySetpoint;
  t_point=IntToBint((int16_t)t_fpoint*10);
  
  t_finput=(n==0)?*PID_n1.myInput:*PID_n2.myInput;
  t_input=IntToBint((int16_t)(t_finput*10));
  
  CalculateMinMaxRound(&min,&max,dl,t_point);
  
  cur_y=(SMALL_LOG_CHART_HEIGHT-((((SMALL_LOG_CHART_HEIGHT*256)/(max-min))*(t_point-min))/256))+SMALL_LOG_OFFSET_Y;
  vsdGUI.SetBlinkPixelStyle(lsDOTED_13);
  vsdGUI.DrawLineHRight(SMALL_LOG_OFFSET_X-1,cur_y,SM_LOG_LEN+2);
  
  uint8_t
	y_max=SMALL_LOG_OFFSET_Y,
	y_min=SMALL_LOG_OFFSET_Y+SMALL_LOG_CHART_HEIGHT-4;
  
  cur_y=((SMALL_LOG_CHART_HEIGHT-((((SMALL_LOG_CHART_HEIGHT*256)/(max-min))*(t_input-min))/256))+SMALL_LOG_OFFSET_Y)-2;
  
  mGUI.setPrintPos(127,cur_y);
  vsdGUI.printf_aright("%0.1f",t_finput);
  
  if((cur_y)>(y_max+7)){
	mGUI.setPrintPos(127,y_max);
	vsdGUI.printf_aright("%d.0",BintToInt(max)/10);
  }
  if((cur_y+7)<y_min){
	mGUI.setPrintPos(127,y_min);
	vsdGUI.printf_aright("%d.0",BintToInt(min)/10);
  }
  //  mGUI.setPrintPos(127,55);
  //  vsdGUI.printf_aright("°C");
  
  for(int i=0;i<SM_LOG_LEN;i++)
  {
	if(dl->t[i]!=SML_EMPTY_VAL){
	  cur_y=(SMALL_LOG_CHART_HEIGHT-((((SMALL_LOG_CHART_HEIGHT*256)/(max-min))*(dl->t[i]-min))/256))+SMALL_LOG_OFFSET_Y;
	  //	cur_y=(55-(dl->t[i]/10/2));
	  mGUI.drawLine(i+SMALL_LOG_OFFSET_X,
					pr_y==1024?cur_y:pr_y,
					i+SMALL_LOG_OFFSET_X+1,
					cur_y);
	  pr_y=cur_y;
	}
  }
}
//******************************************************************************
void DrawChartOutput(uint8_t n)
{
  sLogSample *dl=(n==0)?&(SmLog.dSl.data1):&(SmLog.dSl.data2);
  PIDe *pid=(n==0)?&PID_n1:&PID_n2;
  
  
}
//******************************************************************************
extern unsigned char icon_heater_16x11[];
extern unsigned char icon_cooler_16x11[];
//extern unsigned char icon_pump_17x15[];

void DrawHeatOrCooling(int n)
{
  mGUI.setFont(u8g_font_6x12);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  //  mGUI.drawStr270(2,40,"100%");
  
  if(SwitchMode[n].mode==mpsHEAT_PID || SwitchMode[n].mode==mpsHEAT_ONOFF)
  {  
    if(SwitchMode[n].mode==mpsHEAT_PID) 
    {
      char *str_pwm = vsdGUI.sb_printf("%d%%",SwitchMode[n].pwm);
      mGUI.drawStr270(2,38,str_pwm);
    }
    else mGUI.drawStr270(2,40, (SwitchMode[n].on_off==onoffSET)?" ON":"OFF");
    mGUI.drawBitmap(1,42,2,11,icon_heater_16x11);
  }
  else if(SwitchMode[n].mode==mpsCOOLING_PID || SwitchMode[n].mode==mpsCOOLING_ONOFF)
  {
    if(SwitchMode[n].mode==mpsCOOLING_PID) 
    {
      char *str_pwm = vsdGUI.sb_printf("%d%%",SwitchMode[n].pwm);
      mGUI.drawStr270(2,38,str_pwm);
    }
    else mGUI.drawStr270(2,40, (SwitchMode[n].on_off==onoffSET)?" ON":"OFF");
    mGUI.drawBitmap(1,42,2,11,icon_cooler_16x11);
  }
  //  mGUI.drawBitmap(1,20,2,11,icon_cooler_16x11);
  //  mGUI.drawBitmap(1,35,3,15,icon_pump_17x15);
}
//******************************************************************************
void DrawHeatOrCooling2(int n)
{
  mGUI.setFont(u8g_font_6x12);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  //  mGUI.drawStr270(2,40,"100%");
  
  if(SwitchMode[n].mode==mpsHEAT_PID || SwitchMode[n].mode==mpsHEAT_ONOFF)
  {  
    if(SwitchMode[n].mode==mpsHEAT_PID) 
    {
      mGUI.setPrintPos(15,53);
      vsdGUI.printf("%d%%",SwitchMode[n].pwm);
      mGUI.setPrintPos(125,53);
      vsdGUI.printf_aright("PID-mode");
      //      char *str_pwm = vsdGUI.sb_printf("%d",SwitchMode[n].pwm);
      //      mGUI.setPrintPos(15,53);
      //      mGUI.print(str_pwm);
    }
    else {
      mGUI.setPrintPos(15,53);
      vsdGUI.printf((SwitchMode[n].on_off==onoffSET)?" ON":"OFF");
      mGUI.setPrintPos(125,53);
      vsdGUI.printf_aright("ON/OFF-mode");
    }
    mGUI.drawBitmap(0,52,2,11,icon_heater_16x11);
  }
  else if(SwitchMode[n].mode==mpsCOOLING_PID || SwitchMode[n].mode==mpsCOOLING_ONOFF)
  {
    if(SwitchMode[n].mode==mpsCOOLING_PID) 
    {
      mGUI.setPrintPos(15,53);
      vsdGUI.printf("%d%%",SwitchMode[n].pwm);
      mGUI.setPrintPos(125,53);
      vsdGUI.printf_aright("PID-mode");
      //      char *str_pwm = vsdGUI.sb_printf("%d",SwitchMode[n].pwm);
      //      mGUI.setPrintPos(15,53);
      //      mGUI.print(str_pwm);
    }
    else {
      mGUI.setPrintPos(15,53);
      vsdGUI.printf((SwitchMode[n].on_off==onoffSET)?" ON":"OFF");
      mGUI.setPrintPos(125,53);
      vsdGUI.printf_aright("ON/OFF-mode");
    }
    mGUI.drawBitmap(0,52,2,11,icon_cooler_16x11);
  }
  //  mGUI.drawBitmap(1,20,2,11,icon_cooler_16x11);
  //  mGUI.drawBitmap(1,35,3,15,icon_pump_17x15);
}
//******************************************************************************
void DrawChartProc(uint8_t n)
{
  mGUI.drawHLine(SMALL_LOG_OFFSET_X-1,SMALL_LOG_OFFSET_Y+SMALL_LOG_CHART_HEIGHT+1,SM_LOG_LEN+2);
  mGUI.drawVLine(SMALL_LOG_OFFSET_X-1,SMALL_LOG_OFFSET_Y,SMALL_LOG_CHART_HEIGHT+1);
  DrawChartBasic(n);
  //  mGUI.setFont(u8g_font_5x7);
  //  mGUI.setFontRefHeightText();
  //  mGUI.setFontPosTop();
  
  //  mGUI.setPrintPos(0,55);
  //  vsdGUI.printf("PID-%d",n+1);
  mGUI.setPrintPos(3,12);
  mGUI.setFont(u8g_font_courB10);
  vsdGUI.printf_n("%d",n+1);
  
  mGUI.setFont(u8g_font_5x7);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  mGUI.setPrintPos(104,55);
  vsdGUI.printf_aright("SP=%0.1f°C",(n==0)?*PID_n1.mySetpoint:*PID_n2.mySetpoint);
  DrawChartOutput(n);
  DrawHeatOrCooling(n);
}
//******************************************************************************
void MultiPageProcProcess(void)
{
  
  if(_stv.HW_var.mult_contr==SINGLE)
  {
	switch(mProcStat.contr_pos)
	{
	case 0:{
	  DrawShortVisualProc(0);
	}break;
	case 1:{
	  DrawChartProc(0);
	}break;
	}
  }
  else
  {
    switch(mProcStat.contr_pos)
	{
    case 0:{
      DrawShortVisualProc(0);
    }break;
    case 1:{
      DrawChartProc(0);
    }break;
    case 2:{
      DrawShortVisualProc(1);
    }break;
    case 3:{
      DrawChartProc(1);
	}break;
	}
  }
  
  
}
//******************************************************************************
void DrawMessPresetTpoint(void)
{
  PrepareDrawMess(24);
  vsdGUI.DrawTextCenter(64,40,"counter reset");
  Buzzer_wgen(500,500,10);
  delay(3000);
}
//******************************************************************************
void DrawMessGoalTpoint(void)
{
  PrepareDrawMess(24);
  vsdGUI.DrawTextCenter(64,40,"set point reached"); 
  Buzzer_wgen(500,500,20);
  delay(3000);
}
//******************************************************************************
void DrawMessTimeExpired(void)
{
  PrepareDrawMess(24);
  vsdGUI.DrawTextCenter(64,40,"counter time expired");
  Buzzer_wgen(1000,1000,50);
  delay(5000);
}
//******************************************************************************
void CheckDownTimerMess(void)
{
  if(DTh1.state==TRG_TEMP_HOLD_Mess) {DTh1.state=TRG_TEMP_HOLD; DrawMessGoalTpoint();}
  if(DTh2.state==TRG_TEMP_HOLD_Mess) {DTh2.state=TRG_TEMP_HOLD; DrawMessGoalTpoint();}
  if(DTh1.state==TRG_TIME_COMPLETE_Mess) {DTh1.state=TRG_TIME_COMPLETE; DrawMessTimeExpired();}
  if(DTh2.state==TRG_TIME_COMPLETE_Mess) {DTh2.state=TRG_TIME_COMPLETE; DrawMessTimeExpired();}
  if(DTh1.state==TRG_TEMP_PRESET_Mess) {DTh1.state=TRG_WAIT_TEMP_HOLD/*TRG_TEMP_HOLD*/; DrawMessPresetTpoint();}
  if(DTh2.state==TRG_TEMP_PRESET_Mess) {DTh2.state=TRG_WAIT_TEMP_HOLD/*TRG_TEMP_HOLD*/; DrawMessPresetTpoint();}
  
}
//******************************************************************************
extern void StopProcess(void);
//******************************************************************************
void DrawProc(void)
{
  uint8_t kb;
  //  kb=GetKeyPressL();
  //  kb=GetKeyPressF();
  kb=GetKeyPressMask(1<<BUTTON_ENTER | 1<<BUTTON_SS);
  //  if(kb==BUTTON_UP)
  //	DecVal(&mProcStat.contr_pos, _stv.HW_var.mult_contr==SINGLE?2:4);
  //  else 
  if(kb==BUTTON_ENTER) {IncVal(&mProcStat.contr_pos, _stv.HW_var.mult_contr==SINGLE?2:4);}
  else if(kb==BUTTON_UP/* || kb==BUTTON_UP_LP*/)
  {
    if(mProcStat.contr_pos<=1)
      *PID_n1.mySetpoint=IncValLim(*PID_n1.mySetpoint,TEMP_C_MAX);
    else *PID_n2.mySetpoint=IncValLim(*PID_n2.mySetpoint,TEMP_C_MAX);
  }
  else if(kb==BUTTON_DOWN/* || kb==BUTTON_DOWN_LP*/)
  {
    if(mProcStat.contr_pos<=1)
      *PID_n1.mySetpoint=DecValLim(*PID_n1.mySetpoint,TEMP_C_MIN);
    else *PID_n2.mySetpoint=DecValLim(*PID_n2.mySetpoint,TEMP_C_MIN);
  }
  else if(kb==BUTTON_ENTER_LP){
    //    ClearAllKeyPress();
    if(mProcStat.contr_pos<=1){
      if(down_timer1!=0) down_timer1=vsdGUI.ChangeVarHMSw(down_timer1,20,48*60*60);
      else _stv.ProcPar.timer1_preset=vsdGUI.ChangeVarHMSw(_stv.ProcPar.timer1_preset,20,48*60*60);
    }else{
      if(down_timer2!=0) down_timer2=vsdGUI.ChangeVarHMSw(down_timer2,20,48*60*60);
      else _stv.ProcPar.timer2_preset=vsdGUI.ChangeVarHMSw(_stv.ProcPar.timer2_preset,20,48*60*60);
    }
  }
  else if(kb==BUTTON_SS_LP) {StopProcess();}
  else if(kb==BUTTON_SS) {timer_mode=(timer_mode==0)?1:0;}
  
  CheckDownTimerMess();
  MultiPageProcProcess();
  
}
//******************************************************************************
void DrawOutputPos(int n)
{
  mGUI.drawStr(105,8+n*8,">");
}
//******************************************************************************
void DrawTempStatus(void)
{
//  mGUI.setFont(u8g_font_7x13);
  ////  mGUI.setFont(u8g_font_6x10);
  //  mGUI.setFont(u8g_font_fixed_v0);
//  mGUI.setFont(u8g_font_courR10);

//  mGUI.setFont(u8g_font_profont15);     u8g_font_profont22
  mGUI.setFont(u8g_font_profont15);

  mGUI.setPrintPos(5,22);
  if(_stv.HW_var.probe1_mode!=0)
	if(sensor1_fval<1000){
	  vsdGUI.printf("T1 %0.1f°C",sensor1_fval);
	}else{
	  vsdGUI.printf("T1 (fails)");
	}
  else {vsdGUI.printf("T1 (OFF)");}

  mGUI.setPrintPos(5,37);
  if(_stv.HW_var.probe2_mode!=0)
	if(sensor2_fval<1000){
	  vsdGUI.printf("T2 %0.1f°C",sensor2_fval);
	}else{
	  vsdGUI.printf("T2 (fails)");
    }  
  else {vsdGUI.printf("T2 (OFF)");}
  
}
//******************************************************************************
extern void DrawTime(void);

uint8_t output_pos_count=0;

void DrawStatus(void)
{
  uint8_t kb;
  kb=GetKeyPressMask(/*1<<BUTTON_ENTER |*/ 1<<BUTTON_SS);
  
  //  if(kb==BUTTON_ENTER) {IncVal(&mProcStat.contr_pos, _stv.HW_var.mult_contr==SINGLE?2:4);}
  if(kb==BUTTON_UP/* || kb==BUTTON_UP_LP*/)
  {
    DecVal(&output_pos_count,5);
  }
  else if(kb==BUTTON_DOWN/* || kb==BUTTON_DOWN_LP*/)
  {
    IncVal(&output_pos_count,5);
  }
  else if(kb==BUTTON_ENTER){
    
    ToggleDriveSwitch(output_pos_count);
    
    //    ClearAllKeyPress();
    //    if(mProcStat.contr_pos<=1){
    //      if(down_timer1!=0) down_timer1=vsdGUI.ChangeVarHMSw(down_timer1,20,48*60*60);
    //      else _stv.ProcPar.timer1_preset=vsdGUI.ChangeVarHMSw(_stv.ProcPar.timer1_preset,20,48*60*60);
    //    }else{
    //      if(down_timer2!=0) down_timer2=vsdGUI.ChangeVarHMSw(down_timer2,20,48*60*60);
    //      else _stv.ProcPar.timer2_preset=vsdGUI.ChangeVarHMSw(_stv.ProcPar.timer2_preset,20,48*60*60);
    //    }
  }
  else if(kb==BUTTON_SS_LP) {
    mProcStat.enable=0;
    ClearAllKeyPress();
  }
  
  DrawTime();
  DrawOutputs(110,8);
  DrawOutputPos(output_pos_count);
  DrawTempStatus();
  
}
//******************************************************************************
