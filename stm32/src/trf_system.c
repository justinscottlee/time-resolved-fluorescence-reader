#include "trf_system.h"
#include "stm32h7xx_hal.h"
#include "trf_stepper.h"

TIM_HandleTypeDef htim14;

// Returns if condition is true. Initiates system reset if condition is false.
void TRF_Assert(bool condition) {
    if (condition) {
        return;
    } else {
        HAL_NVIC_SystemReset();
    }
}

void LED_PWM_Init(void) {
    int prescaler = 20;
    int period = 65475;
    int pulse = 16384;

    __HAL_RCC_TIM14_CLK_ENABLE();

    htim14.Instance = TIM14;
    htim14.Init.Prescaler = prescaler;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = period;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    TRF_Assert(HAL_TIM_Base_Init(&htim14) == HAL_OK);
    TRF_Assert(HAL_TIM_PWM_Init(&htim14) == HAL_OK);

    TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.Pulse = pulse;
    TIM_OC_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
    TRF_Assert(HAL_TIM_PWM_ConfigChannel(&htim14, &TIM_OC_InitStruct, TIM_CHANNEL_1) == HAL_OK);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM14;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

// Initializes all the hardware peripherals.
void TRF_Init(void) {
	TRF_Assert(HAL_Init() == HAL_OK);
	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();
	Stepper_Init();	
    LED_PWM_Init();
}

extern TIM_HandleTypeDef htim16;
// Timer callback to tick the stepper motor logic for steps/sec control.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim16) {
        Stepper_Tick();
    }
}