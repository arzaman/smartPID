#ifndef _ARDBUZZER_H_
#define _ARDBUZZER_H_

//#include "ABM_Port.h"

typedef struct TBuzzer_state
{
//  RGB_state state_1;
  unsigned int time_state_1;
//  RGB_state state_2;
  unsigned int time_state_2;
  unsigned int n_cycles;
  unsigned char state_mach;
  unsigned int timer_state_1;
  unsigned int timer_state_2;
//  RGB_state end_state;
}__TBuzzer;

enum mStateRGB{
  stBuz_off=0,
  stBuz_st1,
  stBuz_st2,
};

void Buzzer_wgen(unsigned int t_st1, unsigned int t_st2, unsigned int n_cycles);

#endif