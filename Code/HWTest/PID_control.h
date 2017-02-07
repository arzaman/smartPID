#ifndef __PID_CONTROL_H
#define __PID_CONTROL_H

#include <PID_v1.h>
#include "ABM_Port.h"

class PIDe : public PID{

//friend unsigned long PID::SampleTime;

public:
//friend class PID;

float defInput;
float Input;
float Output;
float Setpoint;
bool Enable;

float f_min,f_max;
int SampleTime;

PIDe() : PID(&Input,&Output,&Setpoint,10,0.1,0,DIRECT){
  Enable=false;
  defInput=1;
//  Input=&defInput;
  Setpoint=90;
  PositiveOutDrvSet=NULL;
  NegativeOutDrvSet=NULL;
  SetSampleTime(1500);
  SetOutputLimits(-100,100);
};

void SetOutputLimits(float min, float max)
{
PID::SetOutputLimits(f_min=min,f_max=max);
}

void SetSampleTime(int stime)
{
PID::SetSampleTime(SampleTime=stime);
}

void SetOutputDrv(void);
void (*PositiveOutDrvSet)(int);
void (*NegativeOutDrvSet)(int);

};

void PID_tick(void);

extern PIDe PID_n1;
extern PIDe PID_n2;

#endif
