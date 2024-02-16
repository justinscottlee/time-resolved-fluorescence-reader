#ifndef TRF_GPIO_H_
#define TRF_GPIO_H_

#include "stm32h7xx_hal.h"
#include <stdbool.h>

typedef struct gpio_pin {
    GPIO_TypeDef *PORT;
    uint32_t PIN;
    bool last_state;
} gpio_pin_t;

void GPIO_Init(void);
void GPIO_Pin_InitOutput(gpio_pin_t *pin);
void GPIO_Pin_InitInput(gpio_pin_t *pin, uint32_t pull);
void GPIO_Pin_Write(gpio_pin_t *pin, bool state);
void GPIO_Pin_Toggle(gpio_pin_t *pin);
bool GPIO_Pin_Read(gpio_pin_t *pin);

#endif