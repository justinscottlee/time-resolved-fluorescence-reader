#include "trf_scheduler.h"
#include <stdlib.h>
#include "trf_system.h"
#include "stm32h7xx_hal.h"

typedef struct task {
    void (*FUNC)();
    uint32_t DELAY, PERIOD, READY;
} task_t;

task_t tasks[MAX_TASKS];

// Insert a task to the scheduler and return its index. If task list is full, force a failed assertion.
uint32_t SCH_AddTask(void (*func)(), uint32_t delay, uint32_t period) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].FUNC == NULL) {
            tasks[i].FUNC = func;
            tasks[i].DELAY = delay;
            tasks[i].PERIOD = period;
            tasks[i].READY = 0;
            return i;
        }
    }
    TRF_Assert(false);
}

// Remove a task from the scheduler based on its task list index.
void SCH_RemoveTask(uint32_t index) {
    tasks[index].FUNC = NULL;
}

// Clear all tasks from the scheduler.
void SCH_ClearTasks(void) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        SCH_RemoveTask(i);
    }
}

// Execute all tasks ready for execution.
void SCH_DispatchTasks(void) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].FUNC == NULL) {
            continue;
        }
        if (tasks[i].READY) {
            tasks[i].READY--;
            tasks[i].FUNC();
            if (tasks[i].PERIOD == 0) {
                SCH_RemoveTask(i);
            }
        }
    }
    HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}

// Decrement each task's delay and mark any ready for execution.
void SCH_Tick(void) {
    for (uint32_t i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].FUNC == NULL)
            continue;
        if (tasks[i].DELAY == 0) {
            tasks[i].READY++;
            tasks[i].DELAY = tasks[i].PERIOD - 1;
        } else {
            tasks[i].DELAY--;
        }
    }
}