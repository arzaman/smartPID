
#include "ABM_Buzzer.h"
#include "ABM_Cfg.h"
#include "wiring_constants.h"
#include "wiring_digital.h"

TBuzzer_state Buzzer_state;

//******************************************************************************
void tickBuzzer(void)
{
  if(Buzzer_state.state_mach==stBuz_off) return;
  if(Buzzer_state.state_mach==stBuz_st1)
  {
    if(Buzzer_state.time_state_1<=(++(Buzzer_state.timer_state_1)))
    {
      Buzzer_state.state_mach=stBuz_st2; digitalWrite(PIN_BUZZER,LOW); Buzzer_state.timer_state_1=0;
    }
    else
    {
    }
    return;
  }
  if(Buzzer_state.state_mach==stBuz_st2)
  {
    if(Buzzer_state.time_state_2<=(++(Buzzer_state.timer_state_2)))
    {
      if(Buzzer_state.n_cycles>0) if ((--Buzzer_state.n_cycles)==0) {Buzzer_state.state_mach=stBuz_off; digitalWrite(PIN_BUZZER,LOW); return;}
      Buzzer_state.state_mach=stBuz_st1; digitalWrite(PIN_BUZZER,HIGH); Buzzer_state.timer_state_2=0;
    }
    else
    {
    }
    return;
  }
}
//******************************************************************************
void setupBuzzer(void)
{
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER,LOW);
  Buzzer_state.n_cycles=0;
  Buzzer_state.state_mach=0;
  Buzzer_state.timer_state_1=0;
  Buzzer_state.timer_state_2=0;
  Buzzer_state.time_state_1=0;
  Buzzer_state.time_state_2=0;
}
//******************************************************************************
void Buzzer_wgen(unsigned int t_st1,
                     unsigned int t_st2, 
                     unsigned int n_cycles)
{
//  _CLI();
//  Buzzer_state.state_1=st1;
  Buzzer_state.time_state_1=t_st1;
//  Buzzer_state.state_2=st2;
  Buzzer_state.time_state_2=t_st2;
  Buzzer_state.n_cycles=n_cycles;
  Buzzer_state.state_mach=stBuz_st1;
  Buzzer_state.timer_state_1=0;
  Buzzer_state.timer_state_2=0;
//  Buzzer_state.end_state=end_st;

  digitalWrite(PIN_BUZZER,HIGH);
  
//  _SEI();
}
//******************************************************************************
