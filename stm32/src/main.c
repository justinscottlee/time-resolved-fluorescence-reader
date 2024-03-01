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

void stepper_test1(void) {
	stepper_x.target_position += 1;
}

void stepper_test2(void) {
	stepper_x.target_position -= 50;
}

void print_adc_buffer(void) {
	uint8_t buffer[16*2048];
	int size = 0;
	for (int i = 0; i < 2048; i++) {
		size += sprintf(buffer + size, "%d\r\n", ADC_Read(i));
	}
	for (int i = 0; i < size; i += 32) {
		USART_Transmit(buffer + i, 32);
	}
}

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();
	GPIO_Init();
	USART_Init();
	ADC_Init();
	Stepper_Init();	
	Stepper_SetSpeed(100);
	ADC_SetSampleRate(500000);

	Stepper_Motor_Init(&stepper_x, &stepper_x_step_pin, &stepper_x_dir_pin);

	GPIO_Pin_InitOutput(&yellowled);
	(void)SCH_AddTask(flash_led, 0, 500);
	(void)SCH_AddTask(print_adc_buffer, 1000, 0);

	while (1) {
		SCH_DispatchTasks();
	}
}