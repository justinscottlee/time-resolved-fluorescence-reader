#include "trf_system.h"
#include "stm32h7xx_hal.h"
#include "trf_stepper.h"

// Returns if condition is true. Initiates system reset if condition is false.
void TRF_Assert(bool condition) {
    if (condition) {
        return;
    } else {
        HAL_NVIC_SystemReset();
    }
}

void TRF_Init(void) {
	TRF_Assert(HAL_Init() == HAL_OK);
	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();
	Stepper_Init();	
}

extern TIM_HandleTypeDef htim16;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim == &htim16) {
        Stepper_Tick();
    }
}