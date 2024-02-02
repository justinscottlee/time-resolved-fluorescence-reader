#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"
#include "trf_scheduler.h"
#include "trf_gpio.h"
#include "trf_usart.h"

#include <string.h>

pin_t yellowled = {GPIOE, GPIO_PIN_1};

void flash_led(void) {
	GPIO_Pin_Toggle(yellowled);
}

uint8_t message[] = "Hello world!\r\n";

void usart_helloworld(void) {
	USART_Transmit(message, sizeof(message));
}

void usart_sillyreceive(void) {
	static uint8_t receivedata[256];
	memset(receivedata, 0, 256);
	USART_Receive(receivedata, 256);
	USART_Transmit(receivedata, strlen(receivedata));
}

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();
	GPIO_Init();
	USART_Init();

	GPIO_Pin_InitOutput(yellowled);
	(void)SCH_AddTask(flash_led, 0, 500);
	(void)SCH_AddTask(usart_helloworld, 0, 1000);
	(void)SCH_AddTask(usart_sillyreceive, 0, 10);

	while (1) {
		SCH_DispatchTasks();
	}
}