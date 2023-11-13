#ifndef TRF_SCHEDULER_H_
#define TRF_SCHEDULER_H_

#include <stdint.h>

#define MAX_TASKS 256

uint32_t SCH_AddTask(void (*func)(), uint32_t delay, uint32_t period);
void SCH_RemoveTask(uint32_t index);
void SCH_DispatchTasks(void);
void SCH_Tick(void);

#endif