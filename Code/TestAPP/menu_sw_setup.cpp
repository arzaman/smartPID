#include "vsd_GUI.h"
#include <delay.h>
#include "storage_var.h"
#include "ABM_Buzzer.h"
#include "ABM_Cfg.h"

int16_t tval;

const char *svControlMode[]={"Heating","Cooling","T-static"};   /*enum e_contr_mode{HEATING,COOLING,T_STATIC};*/
//const char *svHeatMode[]={"Electric","Gas"};                    /*enum e_heat_mode{ELECTRIC,GAS};*/
const char *svPID_Mode[]={"PID","ON/OFF"};                   /*enum e_cooling_mode{MODE_PID,MODE_ON_OFF};*/
const char *svMultiControl[]={"Single","Dual"};                 /*enum e_multi_control{SINGLE,DUAL};*/
const char *svOutDriver[]={"SSR","DC1","DC2","Relay1","Relay2","OFF"};  /*enum e_out_driver{OUT_SSR,OUT_DC1,OUT_DC2,OUT_REL1,OUT_REL2,N_DRIVERS,OUT_OFF};*/
const char *svProbeMode[]={"OFF","DS18B20","NTC"};              /*enum e_probe_mode{PROBE_OFF,PROBE_DS18B20,PROBE_NTC};*/
const char *svTempScale[]={"°C","°F"};                          /*enum e_temp_scale{T_CELSIUS,T_FAHRENHEIT};*/

/*
struct sHW_var{
	uint8_t contr_mode;
	uint8_t heat_mode;
	uint8_t cool_mode;
	uint8_t mult_contr;
	uint8_t out1_heat;
	uint8_t out1_cool;
	uint8_t pump1;
	uint8_t out2_heat;
	uint8_t out2_cool;
	uint8_t pump2;
	uint8_t probe1_mode;
	uint8_t probe2_mode;
}mHW_var;
*/

const sValueList menu_HW_setup_list[]={
{"Control Mode",	(char **)&svControlMode,	&_stv.HW_var.contr_mode,	0,	sCOUNT(svControlMode),	lSTR_LIST},
{"Heating Mode",	(char **)&svPID_Mode,		&_stv.HW_var.heat_mode,	    0,	sCOUNT(svPID_Mode),		lSTR_LIST},
{"Cooling Mode",	(char **)&svPID_Mode,   	&_stv.HW_var.cool_mode,	    0,	sCOUNT(svPID_Mode), 	lSTR_LIST},
{"Multi Control",	(char **)&svMultiControl,	&_stv.HW_var.mult_contr,	0,	sCOUNT(svMultiControl),	lSTR_LIST},
{"Out1 Heating",	(char **)&svOutDriver,		&_stv.HW_var.out1_heat,	    0,	sCOUNT(svOutDriver),	lSTR_LIST},
{"Out1 Cooling",	(char **)&svOutDriver,		&_stv.HW_var.out1_cool,	    0,	sCOUNT(svOutDriver),	lSTR_LIST},
//{"Pump1",			(char **)&svOutDriver,		&_stv.HW_var.pump1,	    	0,	sCOUNT(svOutDriver),	lSTR_LIST},
{"Out2 Heating",	(char **)&svOutDriver,		&_stv.HW_var.out2_heat,	    0,	sCOUNT(svOutDriver),	lSTR_LIST},
{"Out2 Cooling",	(char **)&svOutDriver,		&_stv.HW_var.out2_cool,	    0,	sCOUNT(svOutDriver),	lSTR_LIST},
//{"Pump2",			(char **)&svOutDriver,		&_stv.HW_var.pump2,	    	0,	sCOUNT(svOutDriver),	lSTR_LIST},
{"Temp Probe 1",	(char **)&svProbeMode,		&_stv.HW_var.probe1_mode,	0,	sCOUNT(svProbeMode),	lSTR_LIST},
{"Temp Probe 2",	(char **)&svProbeMode,		&_stv.HW_var.probe2_mode,	0,	sCOUNT(svProbeMode),	lSTR_LIST}
};

/*
struct sUParam{
	float probe1_calibr;
	float probe2_calibr;
	uint8_t temper_unit;
}mUParam;
*/
const sValueList menu_UnitParam_list[]={
{"Temperature Unit",	(char **)&svTempScale,		&_stv.UnitPar.temper_unit,	0,	sCOUNT(svTempScale),	lSTR_LIST},
{"Probe1 Calibr.", 	NULL, 							&_stv.UnitPar.probe1_calibr,	-5,	5,						lFLOAT},
{"Probe2 Calibr.", 	NULL, 							&_stv.UnitPar.probe2_calibr,	-5,	5,						lFLOAT}
};
/*
struct sPParam{
	float Kp;
	float Ki;
	float Kd;
	uint16_t s_time;
	uint16_t pwm_period;
	float hyster;
}mPParam;
*/
const sValueList menu_ProcessParam_list[]={
{"Set-Point 1",			NULL, 						&_stv.ProcPar.TPoint1, TEMP_C_MIN, TEMP_C_MAX, lFLOAT},
{"Set-Point 2",			NULL, 						&_stv.ProcPar.TPoint2, TEMP_C_MIN, TEMP_C_MAX, lFLOAT},
{"Timer 1", 			NULL, 						&_stv.ProcPar.timer1_preset,    0,	32767,	lTIME_HMS},
{"Timer 2",	    		NULL, 						&_stv.ProcPar.timer2_preset,    0,	32767,	lTIME_HMS},
{"PID1 Kp",				NULL, 						&_stv.ProcPar.PID1_Kp,	        -100,	100,	lFLOAT},
{"PID1 Ki",				NULL, 						&_stv.ProcPar.PID1_Ki,	        -100,	100,	lFLOAT},
{"PID1 Kd",				NULL, 						&_stv.ProcPar.PID1_Kd,	        -100,	100,	lFLOAT},
{"Hysteresis 1",		NULL,						&_stv.ProcPar.hysteresis1,	    0,		5,		lFLOAT},
{"Reset DT 1",		    NULL,						&_stv.ProcPar.resetDT1,	        2,		20,		lFLOAT},
{"PID2 Kp",				NULL, 						&_stv.ProcPar.PID2_Kp,	        -100,	100,	lFLOAT},
{"PID2 Ki",				NULL, 						&_stv.ProcPar.PID2_Ki,	        -100,	100,	lFLOAT},
{"PID2 Kd",				NULL, 						&_stv.ProcPar.PID2_Kd,	        -100,	100,	lFLOAT},
{"Hysteresis 2",		NULL,						&_stv.ProcPar.hysteresis2,	    0,		5,		lFLOAT},
{"Reset DT 2",		    NULL,						&_stv.ProcPar.resetDT2,	        2,		20,		lFLOAT},
{"SAMPLE time",			NULL,						&_stv.ProcPar.s_time,	      1000, 	4000,	lUINT16},
{"PWM Period",			NULL,						&_stv.ProcPar.pwm_period,	   500,	   7000,	lUINT16},
};

extern bool menu_HW_setup(void);
extern bool menu_UnitParam(void);
extern bool menu_ProcessParam(void);

vsdMenuStrCb str_Setup[3]={
  {"HW setup", &menu_HW_setup},
  {"Unit parameter", &menu_UnitParam},
  {"Process parameter", &menu_ProcessParam}
};


vsdStrCbGroup SetupMenu={
  (vsdMenuStrCb*)str_Setup,
  sCOUNT(str_Setup),//sizeof(str_cbMenuN1)/sizeof(str_cbMenuN1[0]),
  0
};

bool menu_Configuration(void)
{
	vsdGUI.DrawListMenu("Configuration",&SetupMenu);
	return false;
}

void DrawMessIncorVal(void)
{
WaitSynchDisplay();
vsdGUI.ClrDisp();
mGUI.drawFrame(7, 20, 114, 24);
mGUI.setFont(u8g_font_6x12);
//mGUI.setPrintPos(4,8);
vsdGUI.DrawTextCenter(64,40,"VALUE NOT ALLOWED!");
//vsdGUI.DrawTextCenter(64,35,"in this configuration");

Buzzer_wgen(1000,0,1);
delay(1500);
}

struct s_stv _stv_bk;

void check_HW_online(int pass)
{
if(pass==0) {_stv_bk=_stv; return;}

  uint8_t colision=0;

if(_stv.HW_var.mult_contr==SINGLE)
  {
  if(_stv_bk.HW_var.mult_contr==DUAL) {
    /* The user passed from dual to single control: if any of the second channel
     * drivers are enabled, quietly disable them. */
    _stv.HW_var.out2_heat=N_DRIVERS;
    _stv.HW_var.out2_cool=N_DRIVERS;
    _stv.HW_var.probe2_mode=PROBE_OFF;
    _stv_bk.HW_var.out2_heat=N_DRIVERS;
    _stv_bk.HW_var.out2_cool=N_DRIVERS;
    _stv_bk.HW_var.probe2_mode=PROBE_OFF;
  } else if((_stv.HW_var.out2_heat!=OUT_OFF) || (_stv.HW_var.out2_cool!=OUT_OFF)
      || (_stv.HW_var.probe2_mode!=PROBE_OFF))
    /* The user enabled a driver for the second channel while in single control
     * mode: alert. */
    colision=1;
  }

if(_stv.HW_var.contr_mode==HEATING)
  {
  if(((_stv.HW_var.out1_cool!=OUT_OFF) || (_stv.HW_var.out2_cool!=OUT_OFF)) && (_stv_bk.HW_var.contr_mode==HEATING)) colision=1;
  _stv_bk.HW_var.out1_cool=_stv.HW_var.out1_cool=OUT_OFF;
  _stv_bk.HW_var.out2_cool=_stv.HW_var.out2_cool=OUT_OFF;
  }
if(_stv.HW_var.contr_mode==COOLING)
  {
  if(((_stv.HW_var.out1_heat!=OUT_OFF) || (_stv.HW_var.out2_heat!=OUT_OFF)) && (_stv_bk.HW_var.contr_mode==COOLING)) colision=1;
  _stv_bk.HW_var.out1_heat=_stv.HW_var.out1_heat=OUT_OFF;
  _stv_bk.HW_var.out2_heat=_stv.HW_var.out2_heat=OUT_OFF;
  }

if(_stv.HW_var.out1_cool!=N_DRIVERS){
  if(_stv.HW_var.out1_cool==_stv.HW_var.out1_heat) {colision=1; /*_stv.HW_var.out1_heat=N_DRIVERS;*/}
  if(_stv.HW_var.out1_cool==_stv.HW_var.out2_cool) {colision=1; /*_stv.HW_var.out2_cool=N_DRIVERS;*/}
  if(_stv.HW_var.out1_cool==_stv.HW_var.out2_heat) {colision=1; /*_stv.HW_var.out2_heat=N_DRIVERS;*/}
}
if(_stv.HW_var.out1_heat!=N_DRIVERS){
  if(_stv.HW_var.out1_heat==_stv.HW_var.out2_cool) {colision=1; /*_stv.HW_var.out2_cool=N_DRIVERS;*/}
  if(_stv.HW_var.out1_heat==_stv.HW_var.out2_heat) {colision=1; /*_stv.HW_var.out2_heat=N_DRIVERS;*/}
}
if(_stv.HW_var.out2_heat!=N_DRIVERS){
  if(_stv.HW_var.out2_heat==_stv.HW_var.out2_cool) {colision=1; /*_stv.HW_var.out2_cool=N_DRIVERS;*/}
}
if(colision==1) {_stv=_stv_bk; DrawMessIncorVal();}
}

bool menu_HW_setup(void)
{
//_stv.ProcPar.Kp=100; _stv.ProcPar.Ki=0.2; _stv.ProcPar.Kd=0; _stv.ProcPar.s_time=1500; _stv.ProcPar.pwm_period=3500;
//_stv.ProcPar.hyster=5;
  vsdGUI.DrawValueList("HW setup",(sValueList*)&menu_HW_setup_list,sCOUNT(menu_HW_setup_list),check_HW_online);

StoreVarToEEprom();
return false;
}

bool menu_UnitParam(void)
{
	 vsdGUI.DrawValueList("Unit parameter",(sValueList*)&menu_UnitParam_list,sCOUNT(menu_UnitParam_list));
StoreVarToEEprom();
return false;
}

bool menu_ProcessParam(void)
{
	 vsdGUI.DrawValueList("Process parameter",(sValueList*)&menu_ProcessParam_list,sCOUNT(menu_ProcessParam_list));
     StoreVarToEEprom();
     return false;
}

