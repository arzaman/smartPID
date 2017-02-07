

#include "ABM_HW_switch.h"
#include "storage_var.h"
#include "PID_control.h"
//#include "ABM_Port.h"

#define OUTPUT_INPUT 4

TDrive DriveOut[N_DRIVERS];

_s_mode_switch SwitchMode[2];

#ifdef _WE
void digitalWrite_alt(uint32_t pin, uint32_t val);
#else
extern const PinDescription g_APinDescription[];
#endif
//******************************************************************************
#ifndef _WE
void digitalWrite_alt( uint32_t ulPin, uint32_t ulVal )
{
  // Handle the case the pin isn't usable as PIO
  if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
  {
    return ;
  }
  
  // Enable pull-up resistor
  //  PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_PULLEN) ;
  
  switch ( ulVal )
  {
  case LOW:
    PORT->Group[g_APinDescription[ulPin].ulPort].OUTCLR.reg = (1ul << g_APinDescription[ulPin].ulPin) ;
    break ;
    
  default:
    PORT->Group[g_APinDescription[ulPin].ulPort].OUTSET.reg = (1ul << g_APinDescription[ulPin].ulPin) ;
    break ;
  }
  
  return ;
}
#endif
//******************************************************************************
void pinMode_alt( uint32_t ulPin, uint32_t ulMode )
{
#ifndef _WE
  if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
  {
    return ;
  }
  switch ( ulMode )
  {
  case OUTPUT_INPUT:
    // Set pin to input mode
    PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg=(uint8_t)(PORT_PINCFG_INEN) ;
    //     PORT->Group[g_APinDescription[ulPin].ulPort].DIRCLR.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    PORT->Group[g_APinDescription[ulPin].ulPort].DIRSET.reg = (uint32_t)(1<<g_APinDescription[ulPin].ulPin) ;
    break ;
  }
#endif
}
//******************************************************************************
void DriveSwitch(uint8_t n_drv,enum e_switch_var s)
{
  if(s==CLR_OUT)
  {
    DriveOut[n_drv].drive_set_count=0;
    digitalWrite_alt(DriveOut[n_drv].pin,(DriveOut[n_drv].drive_inverse==false)?LOW:HIGH);
  }
  else
  {
    DriveOut[n_drv].drive_clr_count=0;
    digitalWrite_alt(DriveOut[n_drv].pin,(DriveOut[n_drv].drive_inverse==false)?HIGH:LOW);
  }
}
//******************************************************************************
void ToggleDriveSwitch(uint8_t m_indx)
{
  switch(m_indx)
  {
  case    0:  {digitalWrite_alt(DriveOut[OUT_SSR].pin,(digitalRead(DriveOut[OUT_SSR].pin)==0)?1:0); break;}
  case    1:  {digitalWrite_alt(DriveOut[OUT_DC1].pin,(digitalRead(DriveOut[OUT_DC1].pin)==0)?1:0); break;}
  case    2:  {digitalWrite_alt(DriveOut[OUT_DC2].pin,(digitalRead(DriveOut[OUT_DC2].pin)==0)?1:0); break;}
  case    3:  {digitalWrite_alt(DriveOut[OUT_REL1].pin,(digitalRead(DriveOut[OUT_REL1].pin)==0)?1:0); break;}
  case    4:  {digitalWrite_alt(DriveOut[OUT_REL2].pin,(digitalRead(DriveOut[OUT_REL2].pin)==0)?1:0); break;}
  }
}
//******************************************************************************
void tickDrivers(void)
{
  for(int i=0;i<N_DRIVERS;i++)
  {
    switch(DriveOut[i].drive_mode)
    {
    case OFF_DRIVER: break;
    case SWITCH_OUT:
      {
        if(DriveOut[i].drive_set_count!=0) if ((--DriveOut[i].drive_set_count)==0) DriveSwitch(i,SET_OUT);
        if(DriveOut[i].drive_clr_count!=0) if ((--DriveOut[i].drive_clr_count)==0) DriveSwitch(i,CLR_OUT);
      }break;
    case PWM_OUT:
      {
        if(DriveOut[i].pwm_count>=DriveOut[i].pwm_max) DriveOut[i].pwm_count=0;
        if(DriveOut[i].pwm_count>=DriveOut[i].pwm_comp) DriveSwitch(i,CLR_OUT); else DriveSwitch(i,SET_OUT);
        DriveOut[i].pwm_count++;
      }break;
    }
  }
}
//******************************************************************************
void setupDrivers(void)
{
  DriveOut[OUT_REL1].pin=PIN_RELAY1;
  DriveOut[OUT_REL2].pin=PIN_RELAY2;
  DriveOut[OUT_DC1].pin=PIN_FET1;
  DriveOut[OUT_DC2].pin=PIN_FET2;
  DriveOut[OUT_SSR].pin=PIN_SSR1;
//  DriveOut[OUT_RES1].pin=PIN_RES1;
  
  for(int i=0;i<N_DRIVERS;i++)
  {
    DriveOut[i].pin_mode=OUTPUT;
    //  pinMode(DriveOut[i].pin, OUTPUT);
    pinMode_alt(DriveOut[i].pin, OUTPUT_INPUT);
    digitalWrite_alt(DriveOut[i].pin,LOW);
    DriveOut[i].pwm_comp=0;
    DriveOut[i].pwm_count=0;
    DriveOut[i].pwm_max=0;
    DriveOut[i].drive_inverse=false;
    DriveOut[i].drive_mode=SWITCH_OUT;
    DriveOut[i].drive_set_count=0;
    DriveOut[i].drive_clr_count=0;
  }
  
}
//******************************************************************************
void SwitchForON_OFF_pos(uint8_t pidn,TDrive *drv, uint8_t lock)
{
PIDe *pid;
_s_mode_switch *switch_mode;

float hmin,hmax,half_h;

switch_mode=&SwitchMode[pidn];

if(pidn==PID_1) {
  pid=&PID_n1;
  half_h=(float)_stv.ProcPar.hysteresis1/2;
}
else {
  pid=&PID_n2;
  half_h=(float)_stv.ProcPar.hysteresis2/2;
}

if(lock==1) {drv->drive_clr_count=1; return;}

//half_h=(float)_stv.ProcPar.hyster/2;
hmin=*(pid->mySetpoint)-half_h;
hmax=*(pid->mySetpoint)+half_h;
if((digitalRead(drv->pin)==0))
{
  if(*(pid->myInput)<=hmin) {drv->drive_set_count=1; switch_mode->on_off=onoffSET; switch_mode->mode=mpsHEAT_ONOFF;}
}
else
{
  if(*(pid->myInput)>=hmax) {drv->drive_clr_count=1; switch_mode->on_off=onoffCLR; switch_mode->mode=mpsHEAT_ONOFF;}
}
}
//******************************************************************************
void SwitchForON_OFF_neg(uint8_t pidn,TDrive *drv, uint8_t lock)
{
PIDe *pid;
_s_mode_switch *switch_mode;

float hmin,hmax,half_h;

switch_mode=&SwitchMode[pidn];

if(pidn==PID_1) {
  pid=&PID_n1; 
  half_h=(float)_stv.ProcPar.hysteresis1/2;
}
else {
  pid=&PID_n2;
  half_h=(float)_stv.ProcPar.hysteresis2/2;
}

if(lock==1) {drv->drive_clr_count=1; return;}

//half_h=(float)_stv.ProcPar.hyster/2;
hmin=*(pid->mySetpoint)-half_h;
hmax=*(pid->mySetpoint)+half_h;
if((digitalRead(drv->pin)==0))
{
  if(*(pid->myInput)>=hmax) {drv->drive_set_count=1; switch_mode->on_off=onoffSET; switch_mode->mode=mpsCOOLING_ONOFF;}
}
else
{
  if(*(pid->myInput)<=hmin) {drv->drive_clr_count=1; switch_mode->on_off=onoffCLR; switch_mode->mode=mpsCOOLING_ONOFF;}
}
}
//******************************************************************************
void ManageOutAllCh(float in,uint8_t heat_n, uint8_t cool_n, uint8_t PID_n)
{
  float positive,negative;
  positive=(in>=0?in:0);
  negative=(in<=0?0-in:0);
  TDrive *out_heater=0, *out_cooling=0;
  uint8_t lock=0;
  
  if(heat_n < N_DRIVERS)
  {
	if(_stv.HW_var.contr_mode==HEATING || _stv.HW_var.contr_mode==T_STATIC)
	{
	  out_heater=&DriveOut[heat_n];
	  if(_stv.HW_var.heat_mode==MODE_PID/*ELECTRIC*/) {
		out_heater->pwm_comp=(uint32_t)((out_heater->pwm_max*positive)/100);
		if(0!=(uint8_t)positive || SwitchMode[PID_n].pwm!=0){
		SwitchMode[PID_n].mode=mpsHEAT_PID; SwitchMode[PID_n].pwm=(uint8_t)positive;}
	  }
	  else{
		if(cool_n < N_DRIVERS && negative>0 && _stv.HW_var.cool_mode==MODE_PID) lock=1;
		SwitchForON_OFF_pos(PID_n,out_heater,lock);
	  }
	}
  }

  if(cool_n < N_DRIVERS)
  {
	if(_stv.HW_var.contr_mode==COOLING || _stv.HW_var.contr_mode==T_STATIC)
	{
	  out_cooling=&DriveOut[cool_n];
	  if(_stv.HW_var.cool_mode==MODE_PID) {
		out_cooling->pwm_comp=(uint32_t)((out_cooling->pwm_max*negative)/100);
		if(0!=(uint8_t)negative || SwitchMode[PID_n].pwm!=0){
		SwitchMode[PID_n].mode=mpsCOOLING_PID; SwitchMode[PID_n].pwm=(uint8_t)negative;}
	  }
	  else{
		if(heat_n < N_DRIVERS && positive>0 && _stv.HW_var.heat_mode==MODE_PID) lock=1;
		SwitchForON_OFF_neg(PID_n,out_cooling,lock);
        }
	}
  }
}
//******************************************************************************
void MangeOutCn1(float in)
{
ManageOutAllCh(in,_stv.HW_var.out1_heat,_stv.HW_var.out1_cool,PID_1);
}
//******************************************************************************
void MangeOutCn2(float in)
{
ManageOutAllCh(in,_stv.HW_var.out2_heat,_stv.HW_var.out2_cool,PID_2);
}
//******************************************************************************
