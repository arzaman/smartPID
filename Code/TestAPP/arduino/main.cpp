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

#if defined(MAKEFILE_BUILD) || defined(__ICCARM__)

#include "ArdPort.ino"

#include "../ABM_Port.h"
    
/* Default Arduino systick handler */
extern void SysTick_DefaultHandler(void);

//******************************************************************************
void SysTick_Handler(void)
{
  if (sysTickHook())
    return;
  SysTick_DefaultHandler();
}
//******************************************************************************
static void (*usb_isr)(void) = NULL;

void USB_Handler(void)
{
  if (usb_isr)
    usb_isr();
}

void USB_SetHandler(void (*new_usb_isr)(void))
{
  usb_isr = new_usb_isr;
}

int main( void )
{
  setup();
  for (;;)
  {
    loop();
  }

  return 0;
}

#endif
