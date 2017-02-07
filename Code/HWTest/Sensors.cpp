
#include "ABM_Cfg.h"
#include "Sensors.h"
#include "storage_var.h"
#include "vsd_macros.h"
#include "pin_cfg.h"
#include "delay.h"
#include <system_interrupt.h>
#include <math.h>

const int timing_ds[] = {6, 64, 60, 10, 9, 55, 0, 480, 70, 410};
//const int overdriveT[] = {1.5, 7.5, 7.5, 2.5, 0.75, 7, 2.5, 70, 8.5, 40};

uint32_t NTC1_ADC_val, NTC2_ADC_val;

//uint8_t sensor1_mode=SENSOR_ONEWIRE, 
//uint8_t sensor2_mode=SENSOR_NTC;
uint16_t sensor1_count, sensor2_count;
uint8_t temp_sens1[9],temp_sens2[9];
float sensor1_fval,sensor2_fval;

#define wait_us(x) delayMicroseconds(x)

//******************************************************************************
void sync_ADC() {
  while (ADC->STATUS.bit.SYNCBUSY == 1) ;
}
//******************************************************************************
void writePin(PortGroup *port, uint8_t pin, uint8_t val)
{
if(val) port->OUTSET.reg = (uint32_t)(1<< pin);
else port->OUTCLR.reg = (uint32_t)(1<< pin);
}
//******************************************************************************
void ADC_CannelSet(uint8_t chn)
{
sync_ADC();
ADC->INPUTCTRL.bit.MUXPOS = chn;
}
//******************************************************************************
void ADC_Enable(void)
{
sync_ADC();
ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC
}
//******************************************************************************
void ADC_FakeConversion(void)
{
  sync_ADC();
  ADC->CTRLA.bit.ENABLE = 0x01;             // Enable ADC

  // Start conversion
  sync_ADC();
  ADC->SWTRIG.bit.START = 1;

  // Clear the Data Ready flag
  ADC->INTFLAG.bit.RESRDY = 1;
}
//******************************************************************************
void ADC_StartConversion(void)
{
  // Start conversion
  sync_ADC();
  ADC->SWTRIG.bit.START = 1;
}
//******************************************************************************
uint32_t ADC_ValueRead(void)
{
uint32_t ret_val;
  while ( ADC->INTFLAG.bit.RESRDY == 0 );   // Waiting for conversion to complete
  ret_val = ADC->RESULT.reg;
return ret_val;
}
//******************************************************************************
void ADC_Disable(void)
{
  sync_ADC();
  ADC->CTRLA.bit.ENABLE = 0x00;             // Disable ADC
  sync_ADC();
}
//******************************************************************************
void ADC_SetReference(void)
{
ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;      // Gain Factor Selection
ADC->REFCTRL.bit.REFSEL = ADC_REFCTRL_REFSEL_AREFA_Val;
}
//******************************************************************************
void SensorsSetup(void)
{
InitPin(NTC_probe1,INA);
InitPin(NTC_probe2,INA);
InitPin(ADC_REF,INA);
ADC_SetReference();
ADC_Enable();
}
//******************************************************************************
uint8_t onewire_readBit(void) 
{
    uint8_t result=0;
    
system_interrupt_enter_critical_section();
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,OUTL);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,OUTL);

    wait_us(6/*6*//*timing[0]*/);
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,IN);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,IN);
    wait_us(9/*9*//*timing[4]*/);
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) result|=ReadPin(DSwire1);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) result|=ReadPin(DSwire2)<<1;
system_interrupt_leave_critical_section();
       
    return result;
}
//******************************************************************************
void onewire_writeBit(uint8_t bit1,uint8_t bit2)
{
    bit1 = bit1 & 0x01;
    bit2 = bit2 & 0x01;
uint8_t n=0,b=0;
    
system_interrupt_enter_critical_section();
if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) {InitPin(DSwire1,OUTL); n++;}
if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) {InitPin(DSwire2,OUTL); n++;}
wait_us(6/*6*//*timing[0]*/);
if((bit1!=0) && (_stv.HW_var.probe1_mode==SENSOR_ONEWIRE)) {InitPin(DSwire1,IN); b++;}
if((bit2!=0) && (_stv.HW_var.probe2_mode==SENSOR_ONEWIRE)) {InitPin(DSwire2,IN); b++;}
if(n==b) {
system_interrupt_leave_critical_section();
return;
}
wait_us(60-3/*timing[2]*/);
if(/*(bit1==0) && */(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE)) {InitPin(DSwire1,IN);}
if(/*(bit2==0) && */(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE)) {InitPin(DSwire2,IN);}

#if 0
if (bit)
    {
//        InitPin(DSwire1,OUTL);
        wait_us(3/*6*//*timing[0]*/);
        InitPin(DSwire1,IN);
    }
    else
    {
//        InitPin(DSwire1,OUTL);
        wait_us(60/*timing[2]*/);
        InitPin(DSwire1,IN);
    }
#endif
system_interrupt_leave_critical_section();
}
//******************************************************************************
void read_onewire_bit(uint8_t *data1, uint8_t *data2, uint8_t bit)
{
uint8_t bit_res=onewire_readBit();
  (*data1)=((*data1)&~(1<<bit)) | ((bit_res & 0x01)<<bit);
  (*data2)=((*data2)&~(1<<bit)) | (((bit_res>>1) & 0x01)<<bit);
}
//******************************************************************************
void write_onewire_bit(uint8_t data, uint8_t bit)
{
onewire_writeBit(data>>bit,data>>bit);
}
//******************************************************************************
float NTC_to_Temp(uint32_t in)
{
float tconv;
tconv=in;
// Convert value to resistance
tconv = 1023 / tconv - 1;
tconv = NTC_PULL_RESISTOR / tconv;

//Calculate temperature using the Beta Factor equation
float temperatura;
temperatura = tconv / NTC_RESISTANCE;     // (R/Ro)
temperatura = log(temperatura); // ln(R/Ro)
temperatura /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
temperatura += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
temperatura = 1.0 / temperatura;                 // Invert the value
temperatura -= 273.15;                         // Convert it to Celsius
if (_stv.UnitPar.temper_unit == T_CELSIUS)
	return temperatura;
else
	return CELSIUS_TO_FAHRENHEIT(temperatura);
}

/* Convert a raw value read from DS18B20 to a temperature value
 * Parameters:
 * - in: raw 16-bit value read from DS18B20
 * Returns: temperature value, in either Celsius or Fahrenheit depending on the
 * current user setting
 */
float DS18B20_Temp(uint16_t in)
{
  float val;

  val = in >> 4;
  val += (double)(in & 0x0f) * 0.0625;
  if (_stv.UnitPar.temper_unit == T_CELSIUS)
    return val;
  else
    return CELSIUS_TO_FAHRENHEIT(val);
}
//******************************************************************************
void MathSensorsValues(void)
{
  uint16_t tsens;
  if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) {
  sensor1_fval = DS18B20_Temp(*((uint16_t*)temp_sens1));
  }
  if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) {
  sensor2_fval = DS18B20_Temp(*((uint16_t*)temp_sens2));
  }
  if(_stv.HW_var.probe1_mode==SENSOR_NTC) {
  sensor1_fval=NTC_to_Temp(NTC1_ADC_val);
  }
  if(_stv.HW_var.probe2_mode==SENSOR_NTC) {
  sensor2_fval=NTC_to_Temp(NTC2_ADC_val);
  }
}
//******************************************************************************
void SensorTick(void)
{
if((_stv.HW_var.probe1_mode!=SENSOR_OFF) || (_stv.HW_var.probe2_mode!=SENSOR_OFF))
{
sensor1_count++;
if(sensor1_count>=_stv.ProcPar.s_time) sensor1_count=0;
{
  if(sensor1_count==0) {
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,IN);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,IN);
  }
  else if(sensor1_count==1) {
    if(_stv.HW_var.probe1_mode==SENSOR_NTC) {
      InitPin(DSwire1,IN); InitPin(NTC_probe1,INA);             // Prepare conversion for NTC1
      ADC_CannelSet(NTC_ADC_CH_probe1); ADC_FakeConversion();
    }
  }
  else if(sensor1_count==2) {
    if(_stv.HW_var.probe1_mode==SENSOR_NTC) ADC_StartConversion(); // Start conversion for NTC1
  }
  else if(sensor1_count>=3 && sensor1_count<11) {write_onewire_bit(SKIP_ROM,(sensor1_count-3));}
  else if(sensor1_count>=11 && sensor1_count<19) {write_onewire_bit(0x44,(sensor1_count-11));}

  else if(sensor1_count==100) {
      if(_stv.HW_var.probe1_mode==SENSOR_NTC) NTC1_ADC_val=ADC_ValueRead(); // Get conversion from NTC1 probe
      if(_stv.HW_var.probe2_mode==SENSOR_NTC)
      InitPin(DSwire2,IN); InitPin(NTC_probe2,INA);             // Prepare conversion for NTC1
      ADC_CannelSet(NTC_ADC_CH_probe2); ADC_FakeConversion();
  }
  else if(sensor1_count==101) {
      if(_stv.HW_var.probe2_mode==SENSOR_NTC) ADC_StartConversion(); // Start conversion for NTC2
  }
  else if(sensor1_count==200) {
      if(_stv.HW_var.probe2_mode==SENSOR_NTC) NTC2_ADC_val=ADC_ValueRead(); // Get conversion from NTC2 probe
  }
  else if(sensor1_count==899) {
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,OUTL);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,OUTL);
  }
  else if(sensor1_count==900) {
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,IN);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,IN);
  }
  else if(sensor1_count>=902 && sensor1_count<910) {write_onewire_bit(SKIP_ROM,(sensor1_count-902));}
  else if(sensor1_count>=910 && sensor1_count<918) {write_onewire_bit(0xBE,(sensor1_count-910));}
  else if(sensor1_count>=920 && sensor1_count<992) {
    int bit_offs=sensor1_count-920;
    read_onewire_bit((uint8_t*)&temp_sens1[bit_offs/8],(uint8_t*)&temp_sens2[bit_offs/8],bit_offs%8);
  }
  else if(sensor1_count==992){
  MathSensorsValues();
  }
  else if(sensor1_count==999) {
    if(_stv.HW_var.probe1_mode==SENSOR_ONEWIRE) InitPin(DSwire1,OUTL);
    if(_stv.HW_var.probe2_mode==SENSOR_ONEWIRE) InitPin(DSwire2,OUTL);
  }
   
}
}
}

