#ifndef _RUN_MODE_H_
#define _RUN_MODE_H_

enum e_trig_direct{
  TRD_HEAT,
  TRD_COOL
};

enum e_trig_timer{
  TRG_STOP,
  TRG_WAIT_TEMP_HOLD,
  TRG_TEMP_HOLD_Mess,
  TRG_TEMP_HOLD,
  TRG_TIME_COMPLETE_Mess,
  TRG_TIME_COMPLETE,
  TRG_TEMP_PRESET_Mess,
  TRG_TO_HEATER,
  TRG_TO_COOLING
};

struct sDT{
  uint8_t state;
  uint8_t direct;
  float themp_prev;
  float themp_min;
  float themp_max;
};


struct sProcStat{
  uint8_t contr_pos;
  uint8_t contr_mode;
  uint8_t mult_contr;
  uint8_t enable;
};

void RunBgTasks(void);
void StopProcess(void);

extern struct sProcStat mProcStat;
extern struct sDT DTh1,DTh2;
extern uint32_t down_timer1,down_timer2;
//extern uint8_t temp_trig1, temp_trig2;

#endif
