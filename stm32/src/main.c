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
gpio_pin_t x_endstop = {GPIOG, GPIO_PIN_8};

stepper_motor_t stepper_y = {0};
gpio_pin_t stepper_y_step_pin = {GPIOG, GPIO_PIN_7};
gpio_pin_t stepper_y_dir_pin = {GPIOG, GPIO_PIN_4};
gpio_pin_t y_endstop = {GPIOD, GPIO_PIN_10};

// Task to flash the yellow LED.
void flash_led(void) {
	GPIO_Pin_Toggle(&yellowled);
}

// Task to home the X-axis stepper motor.
void home_x(void) {
	if (stepper_x.homed == false) {
		if (GPIO_Pin_Read(&x_endstop) == 0) {
			stepper_x.homed = true;
			stepper_x.position = 0;
			stepper_x.target_position = 0;
		}
		else if (stepper_x.position == stepper_x.target_position) {
			stepper_x.target_position -= 1;
		}
	}
}

// Task to home the Y-axis stepper motor.
void home_y(void) {
	if (stepper_y.homed == false) {
		if (GPIO_Pin_Read(&y_endstop) == 0) {
			stepper_y.homed = true;
			stepper_y.position = 0;
			stepper_y.target_position = 0;
		}
		else if (stepper_y.position == stepper_y.target_position) {
			stepper_y.target_position -= 1;
		}
	}
}

// Capture a single waveform at the specified sample rate.
void capture_waveform(int sample_rate) {
	ADC_SetSampleRate(sample_rate);
	ADC_Start();
	SCH_AddTask(ADC_Stop, 20, 0);
}

int main(void) {
	TRF_Init();

	Stepper_SetSpeed(10000);
	Stepper_Motor_Init(&stepper_x, &stepper_x_step_pin, &stepper_x_dir_pin);
	Stepper_Motor_Init(&stepper_y, &stepper_y_step_pin, &stepper_y_dir_pin);

	GPIO_Pin_InitOutput(&yellowled);
	(void)SCH_AddTask(flash_led, 0, 500);
	(void)SCH_AddTask(home_x, 0, 1);
	(void)SCH_AddTask(home_y, 0, 1);

	capture_waveform(400000);

	while (1) {
		SCH_DispatchTasks();
	}
}