#ifndef _UTLTS_H_
#define _UTLTS_H_

#include <stdint.h>

void IncVal(uint8_t *val, uint8_t lim);
void DecVal(uint8_t *val, uint8_t lim);
int IncValLim(int val, int max);
float IncValLim(float val, int max);
int DecValLim(int val, int min);
float DecValLim(float val, int min);
uint16_t IntToBint(int16_t in);
int16_t BintToInt(uint16_t in);

#endif
