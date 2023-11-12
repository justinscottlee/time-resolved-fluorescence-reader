#include "trf_gpio.h"

void GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
}

void GPIO_Pin_InitOutput(pin_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.PORT, &GPIO_InitStruct);
    GPIO_Pin_Write(pin, false);
}

void GPIO_Pin_InitInput(pin_t pin, uint32_t pull) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.PORT, &GPIO_InitStruct);
}

void GPIO_Pin_Write(pin_t pin, bool value) {
    HAL_GPIO_WritePin(pin.PORT, pin.PIN, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIO_Pin_Toggle(pin_t pin) {
    HAL_GPIO_TogglePin(pin.PORT, pin.PIN);
}

bool GPIO_Pin_Read(pin_t pin) {
    return HAL_GPIO_ReadPin(pin.PORT, pin.PIN) == GPIO_PIN_SET;
}