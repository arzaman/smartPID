#ifndef _STORAGE_VAR
#define _STORAGE_VAR

#include <IPAddress.h>
#include <stdint.h>

#include "WiFi.h"

#define THINGSPEAK_KEY_LEN	16

struct s_stv{
  
  struct sHW_var{
    uint8_t contr_mode;
    uint8_t heat_mode;
    uint8_t cool_mode;
    uint8_t mult_contr;
    uint8_t out1_heat;
    uint8_t out1_cool;
    uint8_t out2_heat;
    uint8_t out2_cool;
    uint8_t probe1_mode;
    uint8_t probe2_mode;
  }HW_var;
  
  struct sUParam{
    float probe1_calibr;
    float probe2_calibr;
    uint8_t temper_unit;
  }UnitPar;
  
  struct sPParam{
    float TPoint1;
    float TPoint2;
    uint32_t timer1_preset;
    uint32_t timer2_preset;
    float PID1_Kp;
    float PID1_Ki;
    float PID1_Kd;
    float PID2_Kp;
    float PID2_Ki;
    float PID2_Kd;
    uint16_t s_time;
    uint16_t pwm_period;
    float hysteresis1;
    float hysteresis2;
    float resetDT1;
    float resetDT2;
  }ProcPar;
  
  struct sConLogP{
    uint8_t LogMode;
    uint8_t SampleTime;
    uint8_t ConnectMode;
    char SSID[WiFi::SSID_MAX_LEN + 1];	/* null-terminated string */
    char pwd[WiFi::PWD_MAX_LEN + 1];	/* null-terminated string */
    uint16_t localServerPort;
    IPAddress serverIP;
    uint32_t channelID;
    char userKey[THINGSPEAK_KEY_LEN + 1];	/* null-terminated string */
    uint32_t baud;
  }mConLogP;
  
  uint32_t stor_var_control;
};

extern struct s_stv _stv;
void configure_eeprom(void);
void StoreVarToEEprom(void);

/* Reset Wi-Fi network information stored in non-volatile memory
 * Wi-Fi network information consists in SSID and password for connecting to an
 * access point.
 */
void StvResetWiFiNwk(void);

void RestoreVarFromEEprom(void);

#endif
