#ifndef _SMALL_LOG_H_
#define _SMALL_LOG_H_

#define SM_LOG_LEN 90
#define SML_EMPTY_VAL 0

struct sLogSample{
  uint16_t t[SM_LOG_LEN];
  int8_t pwm[SM_LOG_LEN];
};

class cSmLog{
  
  void Shift(sLogSample *ls);
  
public:
  
  struct __data_sm_log{
    sLogSample data1;
    sLogSample data2;
    //  uint16_t temp_chn1[SM_LOG_LEN];
    //  int8_t pwm_chn1[SM_LOG_LEN];
    //  uint16_t temp_chn2[SM_LOG_LEN];
    //  int8_t pwm_chn2[SM_LOG_LEN];
    uint8_t enable;
    uint8_t counter;
    uint8_t freq;
  } dSl;
  
  void Clr(void);
  void SmallLogTick(void);
  
};

extern cSmLog SmLog;

#endif
