#ifndef _ARDBUTTONS_H_
#define _ARDBUTTONS_H_

#include <OneButton.h>

/*
typedef struct _TButton_status{
  union{
  uint8_t status_val;
  struct{
  uint8_t press:1;
  uint8_t click:1;
  uint8_t duble_click:1;
  uint8_t long_start:1;
  uint8_t long_stop:1;
  uint8_t long_press:1;
  }status;
};
}TButton_status;

enum e_button_status{
BTN_NO_PRESS=0,
BTN_PRESS,
BTN_CLICK,
BTN_DOUBLE_CLICK,
BTN_LONG_START,
BTN_LONG_STOP,
BTN_LONG_PRES
};
*/

extern volatile TButton_status Buttons_status[];
extern int autorepeat_factor;

uint8_t GetStatusButtons(uint8_t n);
uint8_t GetKeyPressF(void);
uint8_t GetKeyPressL(void);
uint8_t GetKeyPressMask(uint8_t mask);
uint8_t GetAllKeyPress(void);
void ClearAllKeyPress(void);
uint8_t CheckAllLongPress(void);
uint8_t CheckFreeAllKey(void);
void WaitFreeAllKey(void);

#endif
