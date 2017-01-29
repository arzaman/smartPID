#ifndef __SENSORS_H
#define __SENSORS_H

#include <stdint.h>

#define SKIP_ROM          0xCC

enum e_sens_mode{
  SENSOR_OFF=0,
  SENSOR_ONEWIRE,
  SENSOR_NTC
};

extern uint8_t temp_sens1[],temp_sens2[];
extern float sensor1_fval,sensor2_fval;

void SensorTick(void);
void SensorsSetup(void);

#endif
