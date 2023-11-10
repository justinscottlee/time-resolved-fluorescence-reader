#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"

#include "trf_system.h"
#include "trf_clock.h"

int main(void) {
	TRF_Assert(HAL_Init() == HAL_OK);

	TRF_Clock_Init();

	while (1) {
		
	}
}