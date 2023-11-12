#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"
#include "trf_scheduler.h"
#include "trf_gpio.h"

pin_t yellowled = {GPIOE, GPIO_PIN_1};

void flash_led(void) {
	GPIO_Pin_Toggle(yellowled);
}

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();
	GPIO_Init();

	GPIO_Pin_InitOutput(yellowled);
	SCH_AddTask(flash_led, 0, 500);

	while (1) {
		SCH_DispatchTasks();
	}
}