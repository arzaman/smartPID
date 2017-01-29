
#include "PID_control.h"

//PID_VAR PIDvar[2];

PIDe PID_n1;
PIDe PID_n2;

void PID_tick(void)
{
  if(PID_n1.Enable==true)
    {
    if(PID_n1.Compute()==true)
      {
      PID_n1.SetOutputDrv();
      }
    }
  if(PID_n2.Enable==true)
    {
    if(PID_n2.Compute()==true)
      {
      PID_n2.SetOutputDrv();
      }
    }
}

void PIDe::SetOutputDrv(void)
{
  if(Output>0) {
    if(PositiveOutDrvSet!=NULL) PositiveOutDrvSet((int)Output); 
    if(NegativeOutDrvSet!=NULL) NegativeOutDrvSet(0); 
  }
  else if(Output<0) {
    if(PositiveOutDrvSet!=NULL) PositiveOutDrvSet(0); 
    if(NegativeOutDrvSet!=NULL) NegativeOutDrvSet((int)(0-Output)); 
  }else
  {
    if(PositiveOutDrvSet!=NULL) PositiveOutDrvSet(0); 
    if(NegativeOutDrvSet!=NULL) NegativeOutDrvSet(0); 
  }
}



