#ifndef TRF_ADC_H_
#define TRF_ADC_H_

#include <stdint.h>

#define ADC_BUFFER_SIZE 2048

void ADC_Start(void);
void ADC_Stop(void);
void ADC_Init(void);
void ADC_SetSampleRate(uint32_t samples_per_second);
uint32_t ADC_Read(int index);

#endif