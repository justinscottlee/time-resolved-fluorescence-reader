#include "stm32h7xx_it.h"
#include "stm32h7xx_hal.h"
#include "trf_scheduler.h"

void NMI_Handler(void) {
  while (1) {

  }
}

void HardFault_Handler(void){
	while (1) {

	}
}

void MemManage_Handler(void) {
	while (1) {

	}
}

void BusFault_Handler(void) {
	while (1) {

	}
}

void UsageFault_Handler(void) {
	while (1) {

	}
}

void SVC_Handler(void) {

}

void DebugMon_Handler(void) {

}

void PendSV_Handler(void) {

}

void SysTick_Handler(void) {
	HAL_IncTick();
	SCH_Tick();
}

extern ADC_HandleTypeDef hadc1;
void ADC_IRQHandler(void) {
	HAL_ADC_IRQHandler(&hadc1);
}

extern UART_HandleTypeDef huart3;
void USART3_IRQHandler(void) {
	HAL_UART_IRQHandler(&huart3);
}