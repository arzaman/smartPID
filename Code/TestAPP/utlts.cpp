
#include "utlts.h"
#include <math.h>
#include "ABM_Buttons.h"

uint16_t IntToBint(int16_t in)
{
return (30000+in);
}

int16_t BintToInt(uint16_t in)
{
return ((int)in-(int)30000);
}
  
void IncVal(uint8_t *val, uint8_t lim)
{
if(*val>=(lim-1)) *val=0;
else (*val)++;
}

int IncValLim(int val, int max)
{
if(val>=max) val=max;
else val+=autorepeat_factor;
return val>=max?max:val;
}

float IncValLim(float val, int max)
{
int tint;
float ret;
if(val>=max) return max;
else{
tint=(val+((val<0)?-0.05:0.05))*10;
//tint*=autorepeat_factor;
ret=(tint+1*autorepeat_factor)/10.;
return ret>=max?max:ret;
}
}

void DecVal(uint8_t *val, uint8_t lim)
{
if(*val==0) *val=(lim-1);
else (*val)--;
}

int DecValLim(int val, int min)
{
if(val<=min) val=min;
else val-=autorepeat_factor;
return val<=min?min:val;
}

float DecValLim(float val, int min)
{
int tint;
float ret;
if(val<=min) return min;
else{
tint=(val+((val<0)?-0.05:0.05))*10;
//tint*=autorepeat_factor;
ret=(tint-1*autorepeat_factor)/10.;
return ret<=min?min:ret;
}
}