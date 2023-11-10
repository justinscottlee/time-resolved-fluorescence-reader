#include "trf_system.h"
#include "stm32h7xx_hal.h"

// Returns if condition is true. Initiates system reset if condition is false.
void TRF_Assert(bool condition) {
    if (condition) {
        return;
    } else {
        HAL_NVIC_SystemReset();
    }
}