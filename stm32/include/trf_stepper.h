#ifndef TRF_STEPPER_H_
#define TRF_STEPPER_H_

#include "trf_gpio.h"

typedef struct stepper_motor {
    gpio_pin_t *step_pin;
    gpio_pin_t *dir_pin;
    int32_t position;
    int32_t target_position;
    bool homed;
} stepper_motor_t;

void Stepper_Init(void);
void Stepper_SetSpeed(uint32_t steps_per_second);
void Stepper_Tick(void);
void Stepper_Motor_Init(stepper_motor_t *stepper_motor, gpio_pin_t *step_pin, gpio_pin_t *dir_pin);
bool Stepper_Motor_TargetReached(stepper_motor_t *stepper_motor);

#endif