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

// CALIBRATION CONSTANTS
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
void start_measure_wells(void);
void start_move_to_well(void);
void start_capture_waveform(void);
void start_process_waveform(void);
void start_report_results(void);

state_t current_state;

// { well_1_x, well_1_y, well_2_x, well_2_y, ..., well_n_x, well_n_y, 0xF0 }
uint8_t job_buffer[1024];
int job_buffer_index = 0;

uint32_t concentration_buffer[512];

int current_well;

int led_power_level;

gpio_pin_t yellowled = {GPIOE, GPIO_PIN_1};

stepper_motor_t stepper_x = {0};
gpio_pin_t stepper_x_step_pin = {GPIOG, GPIO_PIN_5};
gpio_pin_t stepper_x_dir_pin = {GPIOG, GPIO_PIN_6};
gpio_pin_t x_endstop = {GPIOG, GPIO_PIN_8};

stepper_motor_t stepper_y = {0};
gpio_pin_t stepper_y_step_pin = {GPIOG, GPIO_PIN_7};
gpio_pin_t stepper_y_dir_pin = {GPIOG, GPIO_PIN_4};
gpio_pin_t y_endstop = {GPIOD, GPIO_PIN_10};

gpio_pin_t led_power_selector_1000mA = {GPIOE, GPIO_PIN_13};
gpio_pin_t led_power_selector_100mA = {GPIOE, GPIO_PIN_15};
gpio_pin_t led_power_selector_10mA = {GPIOE, GPIO_PIN_14};
gpio_pin_t led_power_selector_1mA = {GPIOE, GPIO_PIN_12};

// Task to flash the yellow LED.
void flash_led(void) {
	GPIO_Pin_Toggle(&yellowled);
}

// Capture a single waveform at the specified sample rate.
void capture_waveform(void) {
	ADC_Start();
	SCH_AddTask(ADC_Stop, 25, 0);
}

void check_waveform_clipped(void) {
	disable_led();
	bool clipped = false;
	for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
		if (ADC_Read(i) < 512) {
			clipped = true;
			break;
		}
	}
	if (clipped) {
		if (led_power_level == 0) {
			concentration_buffer[current_well] = -1;
			current_well++;
			start_move_to_well();
		} else {
			led_power_level--;
			start_capture_waveform();
		}
	}
	else {
		start_process_waveform();
	}
}

void select_power_level() {
	GPIO_Pin_Write(&led_power_selector_1000mA, led_power_level == 3);
	GPIO_Pin_Write(&led_power_selector_100mA, led_power_level == 2);
	GPIO_Pin_Write(&led_power_selector_10mA, led_power_level == 1);
	GPIO_Pin_Write(&led_power_selector_1mA, led_power_level == 0);
}

void disable_led() {
	GPIO_Pin_Write(&led_power_selector_1000mA, 0);
	GPIO_Pin_Write(&led_power_selector_100mA, 0);
	GPIO_Pin_Write(&led_power_selector_10mA, 0);
	GPIO_Pin_Write(&led_power_selector_1mA, 0);
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
		start_measure_wells();
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
		start_measure_wells();
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
		led_power_level = 3;
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

// Setup MEASURE_WELLS superstate.
void start_measure_wells(void) {
	current_well = 0;
	start_move_to_well();
}

// Setup MOVE_TO_WELL state.
void start_move_to_well() {
	SCH_ClearTasks();
	current_state = STATE_MOVE_TO_WELL;
	if (job_buffer[current_well * 2] == 0xF0) {
		start_report_results();
		return;
	}
	stepper_x.target_position = MICROPLATE_WELL_ORIGIN_X + job_buffer[current_well * 2] * STEPS_PER_WELL_X;
	stepper_y.target_position = MICROPLATE_WELL_ORIGIN_Y + job_buffer[current_well * 2 + 1] * STEPS_PER_WELL_Y;
	SCH_AddTask(wait_to_reach_well, 0, 1);
}

// Setup CAPTURE_WAVEFORM state.
void start_capture_waveform() {
	SCH_ClearTasks();
	current_state = STATE_CAPTURE_WAVEFORM;
	select_power_level(led_power_level);
	SCH_AddTask(capture_waveform, 25, 0);
	SCH_AddTask(check_waveform_clipped, 50, 0);
}

// Setup PROCESS_WAVEFORM state.
void start_process_waveform(void) {
	SCH_ClearTasks();
	current_state = STATE_PROCESS_WAVEFORM;
	// find zero value of waveform.
	// integrate over waveform from zero part to 2ms.
	// save the value to concentration_buffer.
	current_well++;
	start_move_to_well();
}

// Setup REPORT_RESULTS state.
void start_report_results(void) {
	SCH_ClearTasks();
	current_state = STATE_REPORT_RESULTS;
	USART_Transmit(concentration_buffer, 4 * current_well);
	start_await_job();
}

int main(void) {
	TRF_Init();

	Stepper_SetSpeed(10000);
	Stepper_Motor_Init(&stepper_x, &stepper_x_step_pin, &stepper_x_dir_pin);
	Stepper_Motor_Init(&stepper_y, &stepper_y_step_pin, &stepper_y_dir_pin);

	GPIO_Pin_InitOutput(&led_power_selector_1000mA);
	GPIO_Pin_InitOutput(&led_power_selector_100mA);
	GPIO_Pin_InitOutput(&led_power_selector_10mA);
	GPIO_Pin_InitOutput(&led_power_selector_1mA);
	disable_led();

	start_await_job();

	while (1) {
		SCH_DispatchTasks();
	}
}