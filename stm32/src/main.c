#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"
#include "trf_scheduler.h"
#include "trf_gpio.h"
#include "trf_usart.h"
#include "trf_adc.h"

#include <string.h>
#include <stdio.h>

pin_t yellowled = {GPIOE, GPIO_PIN_1};

void flash_led(void) {
	GPIO_Pin_Toggle(yellowled);
}

void usart_sillyreceive(void) {
	static uint8_t receivedata[256];
	memset(receivedata, 0, 256);
	USART_Receive(receivedata, 256);
	USART_Transmit(receivedata, strlen(receivedata));
}

extern uint32_t *sample_buffer;

void print_data(void) {
	uint8_t data[256];
	memset(data, 0, 64);
	for (int i = 0; i < 10; i++) {
		sprintf(data, "%d: %d\r\n", i, ADC_Read(i));
		USART_Transmit(data, strlen(data));
	}
}

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();

	GPIO_Pin_InitOutput(yellowled);
	(void)SCH_AddTask(flash_led, 0, 500);
	(void)SCH_AddTask(usart_sillyreceive, 0, 10);
	(void)SCH_AddTask(print_data, 0, 1000);

	while (1) {
		SCH_DispatchTasks();
	}
}