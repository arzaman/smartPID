#ifndef __SENSORS_H
#define __SENSORS_H

#include <stdint.h>

#include "ABM_Cfg.h"
#include "storage_var.h"

#define SKIP_ROM          0xCC

enum e_sens_mode{
  SENSOR_OFF=0,
  SENSOR_ONEWIRE,
  SENSOR_NTC
};

extern uint8_t temp_sens1[],temp_sens2[];
extern float sensor1_fval,sensor2_fval;

#define TemperatureMin()	((_stv.UnitPar.temper_unit == T_CELSIUS) ? \
		TEMP_C_MIN : TEMP_F_MIN)
#define TemperatureMax()	((_stv.UnitPar.temper_unit == T_CELSIUS) ? \
		TEMP_C_MAX : TEMP_F_MAX)
#define TemperatureFormat()	((_stv.UnitPar.temper_unit == T_CELSIUS) ? \
		"%0.1f°C" : "%0.1f°F")

void SensorTick(void);
void SensorsSetup(void);

#endif
