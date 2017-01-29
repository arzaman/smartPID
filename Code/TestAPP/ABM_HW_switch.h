#ifndef _ARDHW_SWITCH_H_
#define _ARDHW_SWITCH_H_

#include <stdint.h>

#include "ABM_Cfg.h"

typedef struct _TDrive{
  uint8_t pin;
  uint8_t pin_mode;
  uint8_t drive_mode;
  bool drive_inverse;
  uint32_t pwm_count;
  uint32_t pwm_comp;
  uint32_t pwm_max;
  uint32_t drive_set_count;
  uint32_t drive_clr_count;
}TDrive;

enum mode_pid_switch{
    mpsOFF,
    mpsHEAT_PID,
    mpsCOOLING_PID,
    mpsHEAT_ONOFF,
    mpsCOOLING_ONOFF
};

enum onoffSETs{
  onoffSET,
  onoffCLR
};

struct _s_mode_switch{
  uint8_t mode;
  uint8_t pwm;
  uint8_t on_off;
};

extern _s_mode_switch SwitchMode[];
extern TDrive DriveOut[];
extern void DriveSwitch(uint8_t n_drv,enum e_switch_var s);
void ToggleDriveSwitch(uint8_t m_indx);
extern void MangeOutCn1(float in);
extern void MangeOutCn2(float in);

#endif
