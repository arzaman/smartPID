#ifndef _ARD_PORT_
#define _ARD_PORT_

//#include "Arduino.h"
#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "ABM_Cfg.h"

//typedef bool boolean;

#define itsc_func()

#define __builtin_constant_p(x) 1

#define __inline__ inline
#ifndef NULL
  #define NULL          _NULL
#endif /* NULL */

//extern void yield(void);
extern "C" int sysTickHook(void);
void svcHook(void);
void pendSVHook(void);
void serialEventRun(void);

//void setup(void);
#ifdef __cplusplus
extern "C" {
#endif

void init(void);
void SysTick_DefaultHandler(void);

#ifdef __cplusplus
}
#endif

//void tickReset(void);
//void yield(void);



#endif
