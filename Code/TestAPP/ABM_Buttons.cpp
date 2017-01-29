
#include "ABM_Buttons.h"
#include "ABM_Buzzer.h"
#include "ABM_Cfg.h"
#include "utlts.h"


volatile TButton_status Buttons_status[N_BUTTONS];

mOneButton Button_UP(PIN_BUTTON_UP, &Buttons_status[BUTTON_UP], HIGH);
mOneButton Button_DOWN(PIN_BUTTON_DOWN, &Buttons_status[BUTTON_DOWN], HIGH);
mOneButton Button_ENTER(PIN_BUTTON_ENTER, &Buttons_status[BUTTON_ENTER], HIGH);
mOneButton Button_SS(PIN_BUTTON_SS, &Buttons_status[BUTTON_SS], HIGH);

uint8_t buttons_tick_count;

int autorepeat_factor;

//******************************************************************************
uint8_t GetKeyPressMask(uint8_t mask)
{
  uint8_t ret=NO_PRESS;
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
    if(mask & (1<<i)){
      if(Buttons_status[i].status.click==1 || Buttons_status[i].status.long_start==1)
      {
        ret=i | ((Buttons_status[i].status.long_start==1)?0x10:0);
        Buttons_status[i].status.click=0;
        Buttons_status[i].status.long_start=0;
        Buttons_status[i].status.press=0;
        Buttons_status[i].status.autorepeat=0;
        Buttons_status[i].status.long_press=0;
        autorepeat_factor=Buttons_status[i].status.autorepeat_factor;
      }
    }
    else{
      if(Buttons_status[i].status.press==1 || Buttons_status[i].status.autorepeat==1)
      {
        ret=i;
        Buttons_status[i].status.press=0;
        Buttons_status[i].status.autorepeat=0;
        autorepeat_factor=Buttons_status[i].status.autorepeat_factor;
      }
    }
  }
  return ret;
}
//******************************************************************************
uint8_t GetKeyPressF(void)
{
  uint8_t ret=NO_PRESS;
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
    if(Buttons_status[i].status.press==1 || Buttons_status[i].status.autorepeat==1)
	{
      ret=i;
      Buttons_status[i].status.press=0;
      Buttons_status[i].status.autorepeat=0;
      autorepeat_factor=Buttons_status[i].status.autorepeat_factor;
	}
  }
  return ret;
}
//******************************************************************************
uint8_t GetKeyPressL(void)
{
  uint8_t ret=NO_PRESS;
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
    if(Buttons_status[i].status.click==1 || Buttons_status[i].status.long_start==1)
	{
      ret=i | ((Buttons_status[i].status.long_start==1)?0x10:0);
      Buttons_status[i].status.click=0;
      Buttons_status[i].status.long_start=0;
      Buttons_status[i].status.press=0;
      Buttons_status[i].status.autorepeat=0;
//      Buttons_status[i].status.long_press=0;
      autorepeat_factor=Buttons_status[i].status.autorepeat_factor;
	}
  }
  return ret;
}
//******************************************************************************
/* Retrieve the mask of all keys currently pressed
 * Parameters: none
 * Return: bit mask where each bit position corresponds to a button as defined
 * in enum e_buttons (from BUTTON_UP to BUTTON_SS)
 */
uint8_t GetAllKeyPress(void)
{
  uint8_t ret=0;

  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
    if(Buttons_status[i].status.press==1
        || Buttons_status[i].status.click==1
        || Buttons_status[i].status.long_start==1
        || Buttons_status[i].status.long_press==1)
      ret|=(1<<i);
  }
  return ret;
}
//******************************************************************************
void ClearAllKeyPress(void)
{
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
  Buttons_status[i].status_val=0;
  }
}
//******************************************************************************
uint8_t CheckAllLongPress(void)
{
  uint8_t ret=NO_PRESS;
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
  if(Buttons_status[i].status.long_press==1) ret=i;
  }
return ret;
}
//******************************************************************************
uint8_t CheckFreeAllKey(void)
{
  uint8_t ret=NO_PRESS;
  for(uint8_t i=0;i<N_BUTTONS;i++)
  {
    if(Buttons_status[i].status.level==0)
    {
//    ClearAllKeyPress();
    ret=i;
    }
  }
return ret;
}
//******************************************************************************
void WaitFreeAllKey(void)
{
  while(CheckFreeAllKey()!=NO_PRESS){};
  delay(100);
  ClearAllKeyPress();
}
//******************************************************************************
uint8_t GetKeyAdvance(void)
{
  
}
//******************************************************************************
uint8_t GetStatusButtons(uint8_t n)
{
  if(n>N_BUTTONS) return BTN_NO_PRESS;
  
  TButton_status bstat;
  bstat.status_val=Buttons_status[n].status_val;
  Buttons_status[n].status_val=0;
  
  if(bstat.status.click) return BTN_CLICK;
  if(bstat.status.duble_click) return BTN_DOUBLE_CLICK;
  if(bstat.status.long_start) return BTN_LONG_START;
  if(bstat.status.long_stop) return BTN_LONG_STOP;
  if(bstat.status.long_press) return BTN_LONG_PRES;
  
  return BTN_NO_PRESS;  
}
//******************************************************************************
void Button_UP_click(void)
{
  Buttons_status[BUTTON_UP].status.click=1;
}
void Button_UP_doubleclick(void)
{
  Buttons_status[BUTTON_UP].status.duble_click=1;
}
void Button_UP_longPressStart(void)
{
  Buttons_status[BUTTON_UP].status.long_start=1;
}
void Button_UP_longPressStop(void)
{
  Buttons_status[BUTTON_UP].status.long_stop=1;
}
void Button_UP_longPress(void)
{
  Buttons_status[BUTTON_UP].status.long_press=1;
}
//******************************************************************************
void Button_DOWN_click(void)
{
  Buttons_status[BUTTON_DOWN].status.click=1;
}
void Button_DOWN_doubleclick(void)
{
  Buttons_status[BUTTON_DOWN].status.duble_click=1;
}
void Button_DOWN_longPressStart(void)
{
  Buttons_status[BUTTON_DOWN].status.long_start=1;
}
void Button_DOWN_longPressStop(void)
{
  Buttons_status[BUTTON_DOWN].status.long_stop=1;
}
void Button_DOWN_longPress(void)
{
  Buttons_status[BUTTON_DOWN].status.long_press=1;
}
//******************************************************************************
void Button_ENTER_click(void)
{
  Buttons_status[BUTTON_ENTER].status.click=1;
}
void Button_ENTER_doubleclick(void)
{
  Buttons_status[BUTTON_ENTER].status.duble_click=1;
}
void Button_ENTER_longPressStart(void)
{
  Buttons_status[BUTTON_ENTER].status.long_start=1;
}
void Button_ENTER_longPressStop(void)
{
  Buttons_status[BUTTON_ENTER].status.long_stop=1;
}
void Button_ENTER_longPress(void)
{
  Buttons_status[BUTTON_ENTER].status.long_press=1;
}
//******************************************************************************
void Button_SS_click(void)
{
  Buttons_status[BUTTON_SS].status.click=1;
}
void Button_SS_doubleclick(void)
{
  Buttons_status[BUTTON_SS].status.duble_click=1;
}
void Button_SS_longPressStart(void)
{
  Buttons_status[BUTTON_SS].status.long_start=1;
}
void Button_SS_longPressStop(void)
{
  Buttons_status[BUTTON_SS].status.long_stop=1;
}
void Button_SS_longPress(void)
{
  Buttons_status[BUTTON_SS].status.long_press=1;
}
//******************************************************************************

//******************************************************************************
void ButtonsSetup(void) {
  /*
  Button_UP.attachClick(Button_UP_click);
  Button_UP.attachDoubleClick(Button_UP_doubleclick);
  Button_UP.attachLongPressStart(Button_UP_longPressStart);
  Button_UP.attachLongPressStop(Button_UP_longPressStop);
  Button_UP.attachDuringLongPress(Button_UP_longPress);
  
  Button_DOWN.attachClick(Button_DOWN_click);
  Button_DOWN.attachDoubleClick(Button_DOWN_doubleclick);
  Button_DOWN.attachLongPressStart(Button_DOWN_longPressStart);
  Button_DOWN.attachLongPressStop(Button_DOWN_longPressStop);
  Button_DOWN.attachDuringLongPress(Button_DOWN_longPress);
  
  Button_ENTER.attachClick(Button_ENTER_click);
  Button_ENTER.attachDoubleClick(Button_ENTER_doubleclick);
  Button_ENTER.attachLongPressStart(Button_ENTER_longPressStart);
  Button_ENTER.attachLongPressStop(Button_ENTER_longPressStop);
  Button_ENTER.attachDuringLongPress(Button_ENTER_longPress);
  
  Button_SS.attachClick(Button_SS_click);
  Button_SS.attachDoubleClick(Button_SS_doubleclick);
  Button_SS.attachLongPressStart(Button_SS_longPressStart);
  Button_SS.attachLongPressStop(Button_SS_longPressStop);
  Button_SS.attachDuringLongPress(Button_SS_longPress);
  */
} // setup

void buttons_tick(void)
{
  buttons_tick_count=(buttons_tick_count+1)%10;
  switch(buttons_tick_count)
  {
  case 0: Button_UP.tick(); break;
  case 2: Button_DOWN.tick(); break;
  case 3: Button_ENTER.tick(); break;
  case 4: Button_SS.tick(); break;
  }
  
}

void ButtonBeep(void)
{
  Buzzer_wgen(50,10,1);
}
