#include "utlts.h"
#include "small_log.h"
#include "run_mode.h"
#include "storage_var.h"
#include "PID_control.h"

cSmLog SmLog;

//extern struct sProcStat mProcStat;

//******************************************************************************
void cSmLog::Clr(void)
{
  for(int i=0;i<(SM_LOG_LEN);i++){
  dSl.data1.t[i]=SML_EMPTY_VAL;
  dSl.data2.t[i]=SML_EMPTY_VAL;
}
}
//******************************************************************************
void cSmLog::Shift(sLogSample *ls)
{
  for(int i=0;i<(SM_LOG_LEN-1);i++){
    ls->t[i]=ls->t[i+1];
    ls->pwm[i]=ls->pwm[i+1];
  }
}
//******************************************************************************
void cSmLog::SmallLogTick(void)
{
  if(!mProcStat.enable) return;
//  if(dSl.counter++>=dSl.freq)

  if(PID_n1.compute_complete==1 || PID_n2.compute_complete==1)
  {
	PID_n1.compute_complete=PID_n2.compute_complete=0;
    dSl.counter=0;
    if(_stv.HW_var.mult_contr==SINGLE || _stv.HW_var.mult_contr==DUAL)
    {
      Shift(&dSl.data1);
      dSl.data1.t[SM_LOG_LEN-1]=IntToBint((int16_t)((*PID_n1.myInput)*10));
      dSl.data1.pwm[SM_LOG_LEN-1]=(int8_t)(*PID_n1.myOutput);
    }
    if(_stv.HW_var.mult_contr==DUAL)
    {
      Shift(&dSl.data2);
      dSl.data2.t[SM_LOG_LEN-1]=IntToBint((int16_t)((*PID_n2.myInput)*10));
      dSl.data2.pwm[SM_LOG_LEN-1]=(int8_t)(*PID_n2.myOutput);
    }
  }
}
//******************************************************************************
void SmallLogTick(void)
{
  SmLog.SmallLogTick(); 
}
//******************************************************************************

