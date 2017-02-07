#ifndef _ARDCFG_H_
#define _ARDCFG_H_

#define PIN_BUTTON_UP       14/*S1, PA02, 14         | A0               |  PA02*/
#define PIN_BUTTON_DOWN     17/*S2, PA04, 17         | A3               |  PA04*/
#define PIN_BUTTON_ENTER    8/*S3, PA06, 8          | ~8               |  PA06*/
#define PIN_BUTTON_SS       9/*S4, PA07, 9          | ~9               |  PA07*/

#define PIN_BUZZER      2/*PA14, 2          | 2                |  PA14*/

#define PIN_RELAY1      6/*PA20,  6          | ~6               |  PA20*/
#define PIN_RELAY2      7/*PA21, 7          | 7                |  PA21*/
#define PIN_FET1        26/*PA27, 26         |                  |  PA27*/
#define PIN_FET2        27/*PA28, 27         |                  |  PA28*/
#define PIN_SSR1         22/*PA12, 22         | 1                |  PA12*/
#define PIN_RES1        23/*PB10, 23         | 4                |  PB10*/

#define PIN_SENS1       3/*PA09, 3          | ~3               |  PA09*/
#define PIN_SENS2       4/*PA08, 4          | ~4               |  PA08*/
#define PIN_NTC1        15/*PB08, 15         | A1               |  PB08*/
#define PIN_NTC2        16/*PB09, 16         | A2               |  PB09*/

#define NTC_ADC_CH_probe1 2
#define NTC_ADC_CH_probe2 3

#define NTC_PULL_RESISTOR 5100
#define NTC_RESISTANCE 10000
#define BCOEFFICIENT 3977
#define TEMPERATURENOMINAL 25

#define TEST_PIN_PA13 38

#define HW_SERIAL_NUM_LEN	8
#define HW_SERIAL_NUM_ADDR	(0x4000 - HW_SERIAL_NUM_LEN)

#define CELSIUS_TO_FAHRENHEIT(celsius)		(32 + 1.8 * (celsius))
#define FAHRENHEIT_TO_CELSIUS(fahrenheit)	(0.555555555F * ((fahrenheit) - 32))

#define DELTA_CELSIUS_TO_FAHRENHEIT(celsius)	(1.8 * (celsius))
#define DELTA_FAHRENHEIT_TO_CELSIUS(fahrenheit)	(0.555555555F * (fahrenheit))

#define TEMP_C_MIN	-40
#define TEMP_C_MAX	120

#define TEMP_F_MIN	CELSIUS_TO_FAHRENHEIT(TEMP_C_MIN)
#define TEMP_F_MAX	CELSIUS_TO_FAHRENHEIT(TEMP_C_MAX)

#define RESET_TEMP_C_MIN	2
#define RESET_TEMP_C_MAX	20

#define RESET_TEMP_F_MIN	DELTA_CELSIUS_TO_FAHRENHEIT(RESET_TEMP_C_MIN)
#define RESET_TEMP_F_MAX	DELTA_CELSIUS_TO_FAHRENHEIT(RESET_TEMP_C_MAX)

#define HYST_TEMP_C_MAX	5

#define HYST_TEMP_F_MAX	DELTA_CELSIUS_TO_FAHRENHEIT(HYST_TEMP_C_MAX)

//#define N_BUTTONS       4
enum e_buttons{
BUTTON_UP,
BUTTON_DOWN,
BUTTON_ENTER,
BUTTON_SS,
N_BUTTONS,
NO_PRESS,
BUTTON_UP_LP=0x10,
BUTTON_DOWN_LP,
BUTTON_ENTER_LP,
BUTTON_SS_LP,
};

//#define N_DRIVERS       #
/*
enum e_drivers{
RELAY1,
RELAY2,
FET1,
FET2,
SSR1,
RES1,
N_DRIVERS
};
*/

enum e_drivers_mode{
PWM_OUT,
SWITCH_OUT,
OFF_DRIVER
};

enum e_switch_var{
CLR_OUT,
SET_OUT
};

enum e_contr_mode{HEATING,COOLING,T_STATIC};
//enum e_heat_mode{ELECTRIC,GAS};
enum e_cooling_mode{MODE_PID,MODE_ON_OFF,MODE_DISABLE};
enum e_multi_control{SINGLE,DUAL};
enum e_out_driver{OUT_SSR,OUT_DC1,OUT_DC2,OUT_REL1,OUT_REL2,N_DRIVERS,OUT_OFF=N_DRIVERS};
enum e_probe_mode{PROBE_OFF,PROBE_DS18B20,PROBE_NTC};
enum e_temp_scale{T_CELSIUS,T_FAHRENHEIT};

enum e_pid_num{PID_1,PID_2};

#endif
