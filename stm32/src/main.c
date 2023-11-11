#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"
#include "trf_scheduler.h"

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	Clock_Init();

	while (1) {
		SCH_DispatchTasks();
	}
}