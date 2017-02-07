
#include "ABM_Cfg.h"
#include "storage_var.h"
#include <eeprom.h>
#include <nvm.h>
#include <bod_feature.h>
#include <system_interrupt.h>
#include "delay.h"
#include <string.h>
#include <ThingSpeak.h>
#include <wdt.h>

struct s_stv _stv;

uint8_t ee_indx_stv,ee_pages_stv;

uint8_t eeprom_page_buf[EEPROM_PAGE_SIZE];

#define MARK_CORR_EE 0xA1B77822

void messInitEEprom(void);
void messInitFuseBit(void);
void messReboot(void);

static void trigger_wdt_reset()
{
#ifndef _WE
	struct wdt_conf config_wdt;


	wdt_get_config_defaults(&config_wdt);

	/* Set the Watchdog configuration settings */
	config_wdt.always_on      = false;
	config_wdt.clock_source   = GCLK_GENERATOR_0;
	config_wdt.timeout_period = WDT_PERIOD_2048CLK;

	/* Initialize and enable the Watchdog with the user settings */
	wdt_set_config(&config_wdt);
#endif
}

void configure_eeprom(void)
{
#ifndef _WE
	/* Setup EEPROM emulator service */
//! [init_eeprom_service]
	enum status_code error_code = eeprom_emulator_init();
//! [init_eeprom_service]
	struct nvm_fusebits m_fusebits;

	nvm_get_fuses(&m_fusebits);
	if((m_fusebits.bootloader_size != NVM_BOOTLOADER_SIZE_64) || (m_fusebits.eeprom_size != NVM_EEPROM_EMULATOR_SIZE_16384))
	{
	m_fusebits.bootloader_size = NVM_BOOTLOADER_SIZE_64;
	m_fusebits.eeprom_size = NVM_EEPROM_EMULATOR_SIZE_16384;
	nvm_set_fuses(&m_fusebits);
	messInitFuseBit();
	delay(2000);
	messReboot();
	delay(1000);
	trigger_wdt_reset();
	while(1){};
	}

//! [check_init_ok]
	if (error_code == STATUS_ERR_NO_MEMORY) {
		while (true) {
			/* No EEPROM section has been set in the device's fuses */
		}
	}
//! [check_init_ok]
//! [check_re-init]
	else if (error_code != STATUS_OK) {
		/* Erase the emulated EEPROM memory (assume it is unformatted or
		 * irrecoverably corrupt) */
		eeprom_emulator_erase_memory();
		eeprom_emulator_init();
	}
//! [check_re-init]
#endif
}

#if (SAMD || SAMR21)
void SYSCTRL_Handler(void)
{
	if (SYSCTRL->INTFLAG.reg & SYSCTRL_INTFLAG_BOD33DET) {
		SYSCTRL->INTFLAG.reg |= SYSCTRL_INTFLAG_BOD33DET;
		eeprom_emulator_commit_page_buffer();
	}
}
#endif
static void configure_bod(void)
{
#if (SAMD || SAMR21)
	struct bod_config config_bod33;
	bod_get_config_defaults(&config_bod33);
	config_bod33.action = BOD_ACTION_INTERRUPT;
	/* BOD33 threshold level is about 3.2V */
	config_bod33.level = 48;
	bod_set_config(BOD_BOD33, &config_bod33);
	bod_enable(BOD_BOD33);

	SYSCTRL->INTENSET.reg |= SYSCTRL_INTENCLR_BOD33DET;
	system_interrupt_enable(SYSTEM_INTERRUPT_MODULE_SYSCTRL);
#endif

}
//******************************************************************************
void StoreVarToEEprom(void)
{
uint8_t *p=(uint8_t*)&_stv;
uint8_t eeprom_temp_buf[EEPROM_PAGE_SIZE];

for(uint8_t i=ee_indx_stv;i<ee_pages_stv;i++)
{
uint8_t nbytes=(((i-ee_indx_stv)+1)*EEPROM_PAGE_SIZE)>sizeof(_stv)? sizeof(_stv)%EEPROM_PAGE_SIZE : EEPROM_PAGE_SIZE;
memset(eeprom_page_buf,0,EEPROM_PAGE_SIZE);
memcpy(eeprom_page_buf,&p[i*(EEPROM_PAGE_SIZE)],nbytes);
eeprom_emulator_read_page(i,eeprom_temp_buf);
// check for modification
if(memcmp(eeprom_temp_buf,eeprom_page_buf,nbytes)!=0)
{
memset(eeprom_page_buf,0,EEPROM_PAGE_SIZE);
memcpy(eeprom_page_buf,&p[i*(EEPROM_PAGE_SIZE)],nbytes);
eeprom_emulator_write_page(i, eeprom_page_buf);
eeprom_emulator_commit_page_buffer();
}
}
}
//******************************************************************************
void StvResetWiFiNwk(void)
{
	_stv.mConLogP.SSID[0]='\0';
	_stv.mConLogP.pwd[0]='\0';
	StoreVarToEEprom();
}
//******************************************************************************
void RestoreVarFromEEprom(void)
{
uint8_t *p=(uint8_t*)&_stv;
for(uint8_t i=ee_indx_stv;i<ee_pages_stv;i++)
{
eeprom_emulator_read_page(i,eeprom_page_buf);
uint8_t nbytes=(((i-ee_indx_stv)+1)*EEPROM_PAGE_SIZE)>sizeof(_stv)? sizeof(_stv)%EEPROM_PAGE_SIZE : EEPROM_PAGE_SIZE;
memcpy(&p[i*(EEPROM_PAGE_SIZE)],eeprom_page_buf,nbytes);
}
}
//******************************************************************************
void CalculateIndexVar(void)
{
ee_indx_stv=0;
ee_pages_stv=(sizeof(_stv)+(EEPROM_PAGE_SIZE-1))/EEPROM_PAGE_SIZE;

}
//******************************************************************************
void SheckForInitStorVar(void)
{
//set_fuse_bit
//configure_nvm();
configure_eeprom();
configure_bod();

CalculateIndexVar();

RestoreVarFromEEprom();
if(_stv.stor_var_control==MARK_CORR_EE) return;
//store defaul values
messInitEEprom();
delay(2000);
_stv.HW_var.contr_mode=HEATING;
_stv.HW_var.cool_mode=MODE_ON_OFF;
_stv.HW_var.heat_mode=MODE_PID;
_stv.HW_var.mult_contr=SINGLE;
_stv.HW_var.out1_cool=OUT_REL1;
_stv.HW_var.out1_heat=OUT_SSR;
_stv.HW_var.out2_cool=OUT_OFF;
_stv.HW_var.out2_heat=OUT_OFF;
_stv.HW_var.probe1_mode=PROBE_DS18B20;
_stv.HW_var.probe2_mode=PROBE_OFF;

_stv.ProcPar.TPoint1=55;
_stv.ProcPar.TPoint2=40;
_stv.ProcPar.timer1_preset=5*60;
_stv.ProcPar.timer2_preset=5*60;
_stv.ProcPar.hysteresis1=2;
_stv.ProcPar.hysteresis2=2.5;

_stv.ProcPar.resetDT1=5;
_stv.ProcPar.resetDT2=7;

_stv.ProcPar.PID1_Kp=10;
_stv.ProcPar.PID1_Ki=0.2;
_stv.ProcPar.PID1_Kd=0;
_stv.ProcPar.PID2_Kp=10.1;
_stv.ProcPar.PID2_Ki=0.3;
_stv.ProcPar.PID2_Kd=0.1;

_stv.ProcPar.s_time=1500;
_stv.ProcPar.pwm_period=3500;

_stv.UnitPar.probe1_calibr=0;
_stv.UnitPar.probe2_calibr=0;
_stv.UnitPar.temper_unit=0;

_stv.mConLogP.LogMode=0;
_stv.mConLogP.SampleTime=15;
_stv.mConLogP.ConnectMode=WiFi::MODE_OFF;
_stv.mConLogP.SSID[0]='\0';
_stv.mConLogP.pwd[0]='\0';
_stv.mConLogP.localServerPort = 80;
_stv.mConLogP.serverIP = THINGSPEAK_IPADDRESS;
_stv.mConLogP.channelID=0;
_stv.mConLogP.userKey[0]='\0';
_stv.mConLogP.baud = WiFi::BAUD_RATE_DEFAULT;

_stv.stor_var_control=MARK_CORR_EE;

StoreVarToEEprom();

}
//******************************************************************************
