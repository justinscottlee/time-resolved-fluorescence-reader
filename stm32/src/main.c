#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"
#include "trf_scheduler.h"
#include "trf_gpio.h"
#include "trf_usart.h"
#include "trf_adc.h"
#include "trf_stepper.h"

#include <string.h>
#include <stdio.h>

gpio_pin_t yellowled = {GPIOE, GPIO_PIN_1};

stepper_motor_t stepper_x = {0};
gpio_pin_t stepper_x_step_pin = {GPIOG, GPIO_PIN_5};
gpio_pin_t stepper_x_dir_pin = {GPIOG, GPIO_PIN_6};

void flash_led(void) {
	GPIO_Pin_Toggle(&yellowled);
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

void stepper_test1(void) {
	stepper_x.position = 400;
}

void stepper_test2(void) {
	stepper_x.target_position = -400;
}

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();
	Stepper_Init();
	Stepper_SetSpeed(200);

	Stepper_Motor_Init(&stepper_x, &stepper_x_step_pin, &stepper_x_dir_pin);

	GPIO_Pin_InitOutput(&yellowled);
	(void)SCH_AddTask(flash_led, 0, 500);
	(void)SCH_AddTask(usart_sillyreceive, 0, 10);
	(void)SCH_AddTask(stepper_test1, 0, 2000);
	(void)SCH_AddTask(stepper_test2, 1000, 2000);

	while (1) {
		SCH_DispatchTasks();
	}
}