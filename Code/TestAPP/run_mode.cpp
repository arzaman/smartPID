#include "vsd_GUI.h"
#include "storage_var.h"
#include "PID_control.h"
#include "Sensors.h"
#include "ABM_HW_switch.h"
#include "log.h"
#include "run_mode.h"
#include "small_log.h"

struct sProcStat mProcStat;
uint32_t down_timer1=0,down_timer2=0;
uint32_t up_timer1,up_timer2;
//uint8_t temp_trig1=TRG_STOP, temp_trig2=TRG_STOP;
//uint8_t temp_trig11=0, temp_trig21=0;
struct sDT DTh1,DTh2;


extern void setupDrivers(void);

//******************************************************************************
void DHtProc(int n)
{
  struct sDT *DTh;
  PIDe *PIDt;
  uint32_t *down_timer,*up_timer,timer_preset;
  
  if(n==0){
    DTh=&DTh1; PIDt=&PID_n1; 
    down_timer=&down_timer1;
    up_timer=&up_timer1;
    timer_preset=_stv.ProcPar.timer1_preset;
  }
  else if(n==1){
    DTh=&DTh2; PIDt=&PID_n2;
    down_timer=&down_timer2;
    up_timer=&up_timer2;
    timer_preset=_stv.ProcPar.timer2_preset;
  }
  else return;
  
  if(*(PIDt->myInput)>DTh->themp_min && *(PIDt->myInput)<DTh->themp_max)
  {
    if(DTh->state!=TRG_TEMP_HOLD && DTh->state!=TRG_TEMP_HOLD_Mess 
       && DTh->state!=TRG_TIME_COMPLETE && DTh->state!=TRG_TIME_COMPLETE_Mess){
         if(DTh->direct==TRD_HEAT){
           if(*(PIDt->myInput)>=PIDt->Setpoint){
             DTh->state=TRG_TEMP_HOLD_Mess;
             *down_timer=timer_preset;
             *up_timer=0;
           }
         }
         else{
           if(*(PIDt->myInput)<=PIDt->Setpoint){
             DTh->state=TRG_TEMP_HOLD_Mess;
             *down_timer=timer_preset;
             *up_timer=0;
           }
         }
       }
  }else{
    if(DTh->state==TRG_TEMP_HOLD)
    {
      DTh->state=TRG_TEMP_PRESET_Mess;
      if(*(PIDt->myInput)<PIDt->Setpoint) DTh->direct=TRD_HEAT;
      else DTh->direct=TRD_COOL;
      *down_timer=timer_preset;
      *up_timer=0;
    }
  }
  
  if(DTh->state==TRG_TEMP_HOLD){
    if((*down_timer)!=0){      
      if ((--(*down_timer))==0) {DTh->state=TRG_TIME_COMPLETE_Mess;}
    }
    //    (*up_timer)++;
  }
  
  if(DTh->state==TRG_TEMP_HOLD || DTh->state==TRG_TIME_COMPLETE){  
    (*up_timer)++;
  }
  
  DTh->themp_prev=*(PIDt->myInput);
  
}
//******************************************************************************
void DownTimersHandler(void)
{
  if(DTh1.state!=TRG_STOP) DHtProc(0);
  if(DTh2.state!=TRG_STOP) DHtProc(1);
}
//******************************************************************************
//******************************************************************************
void StopProcess(void)
{
  PID_n1.Enable=false;
  PID_n2.Enable=false;
  setupDrivers();
  mProcStat.enable=false;
}
//******************************************************************************
void ReinitOutDrivers(void)
{
  for(int i = 0; i<N_DRIVERS; i++) {
	DriveOut[i].drive_mode=OFF_DRIVER;
	DriveSwitch(i,CLR_OUT);
  }
  
  if(_stv.HW_var.out1_heat<N_DRIVERS) {
	DriveOut[_stv.HW_var.out1_heat].pwm_max=_stv.ProcPar.pwm_period;
	DriveOut[_stv.HW_var.out1_heat].pwm_comp=0;
	DriveOut[_stv.HW_var.out1_heat].pwm_count=0;
	DriveOut[_stv.HW_var.out1_heat].drive_mode=(_stv.HW_var.heat_mode==MODE_PID/*ELECTRIC*/)?PWM_OUT:SWITCH_OUT;
  }
  
  if(_stv.HW_var.out1_cool<N_DRIVERS) {
	DriveOut[_stv.HW_var.out1_cool].pwm_max=_stv.ProcPar.pwm_period;
	DriveOut[_stv.HW_var.out1_cool].pwm_comp=0;
	DriveOut[_stv.HW_var.out1_cool].pwm_count=0;
	DriveOut[_stv.HW_var.out1_cool].drive_mode=(_stv.HW_var.cool_mode==MODE_PID)?PWM_OUT:SWITCH_OUT;
  }
  
  if(_stv.HW_var.out2_heat<N_DRIVERS) {
	DriveOut[_stv.HW_var.out2_heat].pwm_max=_stv.ProcPar.pwm_period;
	DriveOut[_stv.HW_var.out2_heat].pwm_comp=0;
	DriveOut[_stv.HW_var.out2_heat].pwm_count=0;
	DriveOut[_stv.HW_var.out2_heat].drive_mode=(_stv.HW_var.heat_mode==MODE_PID/*ELECTRIC*/)?PWM_OUT:SWITCH_OUT;
  }
  
  if(_stv.HW_var.out2_cool<N_DRIVERS) {
	DriveOut[_stv.HW_var.out2_cool].pwm_max=_stv.ProcPar.pwm_period;
	DriveOut[_stv.HW_var.out2_cool].pwm_comp=0;
	DriveOut[_stv.HW_var.out2_cool].pwm_count=0;
	DriveOut[_stv.HW_var.out2_cool].drive_mode=(_stv.HW_var.cool_mode==MODE_PID)?PWM_OUT:SWITCH_OUT;
  }
}
//******************************************************************************
void InitSwitchMode(void)
{
  
  if(_stv.HW_var.contr_mode==HEATING){
    if(_stv.HW_var.heat_mode==MODE_PID) {
      SwitchMode[0].mode=SwitchMode[1].mode=mpsHEAT_PID;
    }
    else{
      SwitchMode[0].mode=SwitchMode[1].mode=mpsHEAT_ONOFF;
    }
  }else if(_stv.HW_var.contr_mode==COOLING)
    if(_stv.HW_var.cool_mode==MODE_PID) {
      SwitchMode[0].mode=SwitchMode[1].mode=mpsCOOLING_PID;
    }
    else{
      SwitchMode[0].mode=SwitchMode[1].mode=mpsCOOLING_ONOFF;
    }
  else{
    if(sensor1_fval<_stv.ProcPar.TPoint1){
      if(_stv.HW_var.heat_mode==MODE_PID) {
        SwitchMode[0].mode=mpsHEAT_PID;
      }
      else{
        SwitchMode[0].mode=mpsHEAT_ONOFF;
      }
    }else{
      if(_stv.HW_var.cool_mode==MODE_PID) {
        SwitchMode[0].mode=mpsCOOLING_PID;
      }
      else{
        SwitchMode[0].mode=mpsCOOLING_ONOFF;
      }
    }
    if(sensor2_fval<_stv.ProcPar.TPoint2){
      if(_stv.HW_var.heat_mode==MODE_PID) {
        SwitchMode[1].mode=mpsHEAT_PID;
      }
      else{
        SwitchMode[1].mode=mpsHEAT_ONOFF;
      }
    }else{
      if(_stv.HW_var.cool_mode==MODE_PID) {
        SwitchMode[1].mode=mpsCOOLING_PID;
      }
      else{
        SwitchMode[1].mode=mpsCOOLING_ONOFF;
      }
    }
    
  }
  
  SwitchMode[0].pwm=SwitchMode[1].pwm=0;
  SwitchMode[0].on_off=SwitchMode[1].on_off=onoffCLR;
  
}
//******************************************************************************
const sValueList menu_RunSetPoint_list[]={
  {"Set-Point 1",			NULL, 						&_stv.ProcPar.TPoint1, TEMP_C_MIN, TEMP_C_MAX, lFLOAT},
  {"Set-Point 2",			NULL, 						&_stv.ProcPar.TPoint2, TEMP_C_MIN, TEMP_C_MAX, lFLOAT},
};
//******************************************************************************
bool menu_run(void)
{
  
  vsdGUI.DrawValueList("Run Set-Point",(sValueList*)&menu_RunSetPoint_list,sCOUNT(menu_RunSetPoint_list));
  
  
  mProcStat.contr_mode=_stv.HW_var.contr_mode;
  mProcStat.mult_contr=_stv.HW_var.mult_contr;
  
  PID_n1.kp=_stv.ProcPar.PID1_Kp;
  PID_n1.ki=_stv.ProcPar.PID1_Ki;
  PID_n1.kd=_stv.ProcPar.PID1_Kd;
  
  PID_n2.kp=_stv.ProcPar.PID2_Kp;
  PID_n2.ki=_stv.ProcPar.PID2_Ki;
  PID_n2.kd=_stv.ProcPar.PID2_Kd;
  
  PID_n2.Setpoint=_stv.ProcPar.TPoint2;
  PID_n1.Setpoint=_stv.ProcPar.TPoint1;
  
  //  if(_stv.HW_var.contr_mode==)
  PID_n1.SetOutputLimits(-100,100);
  PID_n2.SetOutputLimits(-100,100);
  PID_n1.ITerm=0;
  PID_n2.ITerm=0;
  
  PID_n1.SetSampleTime(_stv.ProcPar.s_time);
  PID_n2.SetSampleTime(_stv.ProcPar.s_time);
  
  ReinitOutDrivers();
  
  PID_n1.manageOutputCB=PID_n2.manageOutputCB=NULL;
  PID_n1.Enable=PID_n2.Enable=false;
  
  DTh2.state=DTh1.state=TRG_STOP;
  if(_stv.HW_var.mult_contr==SINGLE || _stv.HW_var.mult_contr==DUAL)
  {
    DTh1.themp_max=_stv.ProcPar.TPoint1+_stv.ProcPar.resetDT1;
    DTh1.themp_min=_stv.ProcPar.TPoint1-_stv.ProcPar.resetDT1;
    DTh1.themp_prev=sensor1_fval;
    DTh1.state=TRG_WAIT_TEMP_HOLD;
    
    if(sensor1_fval<_stv.ProcPar.TPoint1) DTh1.direct=TRD_HEAT;
    else DTh1.direct=TRD_COOL;
    
    PID_n1.myInput=&sensor1_fval;
    PID_n1.SetMode(AUTOMATIC);
    PID_n1.manageOutputCB=MangeOutCn1;
    PID_n1.Enable=true;
  }
  if(_stv.HW_var.mult_contr==DUAL)
  {
    //    if(sensor2_fval<=PID_n2.Setpoint) temp_trig21=TRG_TO_HEATER; else temp_trig21=TRG_TO_COOLING;
    DTh2.themp_max=_stv.ProcPar.TPoint2+_stv.ProcPar.resetDT2;
    DTh2.themp_min=_stv.ProcPar.TPoint2-_stv.ProcPar.resetDT2;
    DTh2.themp_prev=sensor2_fval;
    DTh2.state=TRG_WAIT_TEMP_HOLD;
    
    if(sensor2_fval<_stv.ProcPar.TPoint2) DTh2.direct=TRD_HEAT;
    else DTh2.direct=TRD_COOL;
    
    DTh2.themp_prev=sensor2_fval;
    DTh2.state=TRG_WAIT_TEMP_HOLD;
    PID_n2.myInput=&sensor2_fval;
    PID_n2.SetMode(AUTOMATIC);
    PID_n2.manageOutputCB=MangeOutCn2;
    PID_n2.Enable=true;
  }
  
  InitSwitchMode();
  
  up_timer1=up_timer2=0;
  down_timer1=down_timer2=0;
  
  SmLog.Clr();
  logStart();
  mProcStat.contr_pos=0;
  mProcStat.enable=1;
  return false;
}
//******************************************************************************
bool menu_status(void)
{
  mProcStat.enable=2;
  return false;
}
//******************************************************************************
