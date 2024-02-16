#ifndef TRF_ADC_H_
#define TRF_ADC_H_

#include <stdint.h>

void ADC_Init(void);
uint32_t ADC_Read(int index);

#endif