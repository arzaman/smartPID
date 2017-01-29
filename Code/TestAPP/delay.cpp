/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "delay.h"
#include "Arduino.h"
#include "log.h"
#include "vsd_macros.h"
#include "PID_control.h"
#include "spcServer.h"
#include "WiFi.h"

extern void frame_buffer_refresh(void);
extern void buttons_tick(void);
extern void tickBuzzer(void);
extern void tickDrivers(void);
extern void SensorTick(void);
extern void SmallLogTick(void);
extern void DownTimersHandler(void);

#ifdef __cplusplus
extern "C" {
#endif
  
  
/** Tick Counter united by ms */
static volatile uint32_t _ulTickCount=0 ;
volatile uint32_t _seconds_timer=0 ;
static volatile uint16_t _div_timer=0 ;


uint32_t millis( void )
{
// todo: ensure no interrupts
  return _ulTickCount ;
}

// Interrupt-compatible version of micros
// Theory: repeatedly take readings of SysTick counter, millis counter and SysTick interrupt pending flag.
// When it appears that millis counter and pending is stable and SysTick hasn't rolled over, use these
// values to calculate micros. If there is a pending SysTick, add one to the millis counter in the calculation.
uint32_t micros( void )
{
  uint32_t ticks, ticks2;
  uint32_t pend, pend2;
  uint32_t count, count2;

  ticks2  = SysTick->VAL;
  pend2   = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)  ;
  count2  = _ulTickCount ;

  do
  {
    ticks=ticks2;
    pend=pend2;
    count=count2;
    ticks2  = SysTick->VAL;
    pend2   = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)  ;
    count2  = _ulTickCount ;
  } while ((pend != pend2) || (count != count2) || (ticks < ticks2));

  return ((count+pend) * 1000) + (((SysTick->LOAD  - ticks)*(1048576/(VARIANT_MCK/1000000)))>>20) ;
  // this is an optimization to turn a runtime division into two compile-time divisions and
  // a runtime multiplication and shift, saving a few cycles
}

void delay( uint32_t ms )
{
  if ( ms == 0 )
  {
    return ;
  }

  uint32_t start = _ulTickCount ;

  do
  {
    yield();
  } while ( _ulTickCount - start < ms ) ;
}

#include "Reset.h" // for tickReset()

void SysTick_DefaultHandler(void)
{
  // Increment tick count each ms
  WritePin_dir(PORT_A,13,1);
  _ulTickCount++;
  frame_buffer_refresh();
  buttons_tick();
  tickBuzzer();
  tickDrivers();
  SensorTick();
  PID_tick();
  SmallLogTick();
  WiFiTick();
  logTick();
  spcServerTick();
  tickReset();
  if(_div_timer>=1000) {_div_timer=0; _seconds_timer++; DownTimersHandler();}
  else _div_timer++;

  WritePin_dir(PORT_A,13,0);
}

#ifdef __cplusplus
}
#endif
