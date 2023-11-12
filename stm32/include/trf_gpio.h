#ifndef TRF_GPIO_H_
#define TRF_GPIO_H_

#include "stm32h7xx_hal.h"
#include <stdbool.h>

typedef struct GPIO_Pin {
    GPIO_TypeDef *PORT;
    uint32_t PIN;
} pin_t;

void GPIO_Init(void);
void GPIO_Pin_InitOutput(pin_t pin);
void GPIO_Pin_InitInput(pin_t pin, uint32_t pull);
void GPIO_Pin_Write(pin_t pin, bool value);
void GPIO_Pin_Toggle(pin_t pin);
bool GPIO_Pin_Read(pin_t pin);

#endif