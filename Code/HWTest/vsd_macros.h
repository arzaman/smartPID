#ifndef __VSD_MACROS
#define __VSD_MACROS
/*SAMD*/
/*macros for fast work GPIO and optimizing memory*/
/*(VSD)VoblNet vadim8097@gmail.com*/

#define PORT_A PORT->Group[0]
#define PORT_B PORT->Group[1]

#define _write_pin_(port,pin,val)\
{if(val) port.OUTSET.reg = (uint32_t)(1<< pin);\
else port.OUTCLR.reg = (uint32_t)(1<< pin);}

#define WritePin(x,val) _write_pin_(x,val)
#define WritePin_dir(port,pin,val) _write_pin_(port,pin,val)

#define _read_pin_(port,pin)\
((port.IN.reg & (uint32_t)(1<< pin))==0?0:1) 
#define ReadPin(x) _read_pin_(x)

#define _ddr_init_INPU(port,pin)\
{port.PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);\
  port.DIRCLR.reg = (uint32_t)(1<<pin);\
    port.OUTSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_INPD(port,pin)\
{port.PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);\
  port.DIRCLR.reg = (uint32_t)(1<<pin);\
    port.OUTCLR.reg = (uint32_t)(1<<pin);}

#define _ddr_init_IN(port,pin)\
{port.PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);\
  port.DIRCLR.reg = (uint32_t)(1<<pin);}

#define _ddr_init_INOUT(port,pin)\
{port.PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);\
  port.DIRSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUT(port,pin)\
{port.PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  port.DIRSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUTH(port,pin)\
{port.PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  port.DIRSET.reg = (uint32_t)(1<<pin);\
    port.OUTSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUTL(port,pin)\
{port.PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  port.DIRSET.reg = (uint32_t)(1<<pin);\
    port.OUTCLR.reg = (uint32_t)(1<<pin);}

//void InitINA(PortGroup port,int pin)

#define _ddr_init_INA(port,pin) \
{\
  if(pin&1)\
      {\
        uint32_t temp ;\
        temp = (port.PMUX[pin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;\
        port.PMUX[pin >> 1].reg = temp|PORT_PMUX_PMUXO( PIO_ANALOG ) ;\
        port.PINCFG[pin].reg |= PORT_PINCFG_PMUXEN ;\
      }\
      else\
      {\
        uint32_t temp ;\
        temp = (port.PMUX[pin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;\
        port.PMUX[pin >> 1].reg = temp|PORT_PMUX_PMUXE( PIO_ANALOG ) ;\
        port.PINCFG[pin].reg |= PORT_PINCFG_PMUXEN ;\
      }\
}\

        
#define _llr_init_(port,pin,dd) _ddr_init_##dd(port,pin)
#define _llr_init_n(port,pin,ddn) _llr_init_(port,pin,ddn)
#define InitPin(x,ddn) _llr_init_n(x,ddn)
#define InitPinDef(x) _llr_init_(x)

/*
#define _write_pin_(port,pin,dd,val)\
{if(val) PORT->Group[PORT##port].OUTSET.reg = (uint32_t)(1<< pin);\
else PORT->Group[PORT##port].OUTCLR.reg = (uint32_t)(1<< pin);}

#define WritePin(x,val) _write_pin_(x,val)

#define _read_pin_(port,pin,dd)\
((PORT->Group[PORT##port].IN.reg & (uint32_t)(1<< pin))==0?0:1) 
#define ReadPin(x) _read_pin_(x)

#define _ddr_init_INPU(port,pin)\
{PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);\
  PORT->Group[port].DIRCLR.reg = (uint32_t)(1<<pin);\
    PORT->Group[port].OUTSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_INPD(port,pin)\
{PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN|PORT_PINCFG_PULLEN);\
  PORT->Group[port].DIRCLR.reg = (uint32_t)(1<<pin);\
    PORT->Group[port].OUTCLR.reg = (uint32_t)(1<<pin);}

#define _ddr_init_IN(port,pin)\
{PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);\
  PORT->Group[port].DIRCLR.reg = (uint32_t)(1<<pin);}

#define _ddr_init_INOUT(port,pin)\
{PORT->Group[port].PINCFG[pin].reg=(uint8_t)(PORT_PINCFG_INEN);\
  PORT->Group[port].DIRSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUT(port,pin)\
{PORT->Group[port].PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  PORT->Group[port].DIRSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUTH(port,pin)\
{PORT->Group[port].PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  PORT->Group[port].DIRSET.reg = (uint32_t)(1<<pin);\
    PORT->Group[port].OUTSET.reg = (uint32_t)(1<<pin);}

#define _ddr_init_OUTL(port,pin)\
{PORT->Group[port].PINCFG[pin].reg&=~(uint8_t)(PORT_PINCFG_INEN);\
  PORT->Group[port].DIRSET.reg = (uint32_t)(1<<pin);\
    PORT->Group[port].OUTCLR.reg = (uint32_t)(1<<pin);}

#define _llr_init_(port,pin,dd) _ddr_init_##dd(PORT##port,pin)
#define _llr_init_n(port,pin,ddf,ddn) _llr_init_(port,pin,ddn)
#define InitPin(x,ddn) _llr_init_n(x,ddn)
#define InitPinDef(x) _llr_init_(x)
*/

#endif