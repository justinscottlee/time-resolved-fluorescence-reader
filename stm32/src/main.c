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

#define MICROPLATE_WELL_ORIGIN_X 10000
#define MICROPLATE_WELL_ORIGIN_Y 5000
#define STEPS_PER_WELL_X 500
#define STEPS_PER_WELL_Y 250

typedef enum {
	STATE_AWAIT_JOB,
	STATE_HOME_AXES,
	STATE_MOVE_TO_WELL,
	STATE_CAPTURE_WAVEFORM,
	STATE_PROCESS_WAVEFORM,
	STATE_REPORT_RESULTS
} state_t;

void start_await_job(void);
void start_home_axes(void);
void start_move_to_well(int well_index);
void start_capture_waveform(void);
void start_process_waveform(void);
void start_report_results(void);

state_t current_state;

// { well_1_x, well_1_y, well_2_x, well_2_y, ..., well_n_x, well_n_y, 0xF0 }
uint8_t job_buffer[1024];
int job_buffer_index = 0;

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

// Capture a single waveform at the specified sample rate.
void capture_waveform(int sample_rate) {
	ADC_SetSampleRate(sample_rate);
	ADC_Start();
	SCH_AddTask(ADC_Stop, 20, 0);
}

// Home the X-axis stepper motor by 1 step.
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

	if (stepper_x.homed && stepper_y.homed) {
		start_move_to_well(0);
	}
}

// Home the Y-axis stepper motor by 1 step.
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

	if (stepper_x.homed && stepper_y.homed) {
		start_move_to_well(0);
	}
}

// Receive bytes from USART, until a 0xF0 byte is received, signaling the end of the job.
void receive_job(void) {
	int bytes_received = USART_Receive(&job_buffer[job_buffer_index], 8);
	job_buffer_index += bytes_received;
	if (bytes_received > 0) {
		uint8_t last_received_char = job_buffer[job_buffer_index - 1];
		if (last_received_char == 0xF0) {
			start_home_axes();
			return;
		}
	}
}

void wait_to_reach_well(void) {
	if (Stepper_Motor_TargetReached(&stepper_x) && Stepper_Motor_TargetReached(&stepper_y)) {
		start_capture_waveform();
	}
}

// Setup AWAIT_JOB state.
void start_await_job(void) {
	SCH_ClearTasks();
	current_state = STATE_AWAIT_JOB;
	SCH_AddTask(receive_job, 0, 1);
}

// Setup HOME_AXES state.
void start_home_axes(void) {
	SCH_ClearTasks();
	current_state = STATE_HOME_AXES;
	SCH_AddTask(home_x, 0, 1);
	SCH_AddTask(home_y, 0, 1);
}

// Setup MOVE_TO_WELL state.
void start_move_to_well(int well_index) {
	SCH_ClearTasks();
	current_state = STATE_MOVE_TO_WELL;
	stepper_x.target_position = MICROPLATE_WELL_ORIGIN_X + job_buffer[well_index * 2] * STEPS_PER_WELL_X;
	stepper_y.target_position = MICROPLATE_WELL_ORIGIN_Y + job_buffer[well_index * 2 + 1] * STEPS_PER_WELL_Y;
	move_to_well(job_buffer[well_index * 2], job_buffer[well_index * 2 + 1]);
	SCH_AddTask(wait_to_reach_well, 0, 1);
}

int main(void) {
	TRF_Init();

	Stepper_SetSpeed(10000);
	Stepper_Motor_Init(&stepper_x, &stepper_x_step_pin, &stepper_x_dir_pin);
	Stepper_Motor_Init(&stepper_y, &stepper_y_step_pin, &stepper_y_dir_pin);

	start_await_job();

	while (1) {
		SCH_DispatchTasks();
	}
}