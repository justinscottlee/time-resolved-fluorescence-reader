#include "trf_stepper.h"
#include "trf_system.h"

#include <stdlib.h>

#define TIM_CLK 275000000

TIM_HandleTypeDef htim16;

typedef struct stepper_motor_node {
    stepper_motor_t *motor;
    struct stepper_motor_node *next;
} stepper_motor_node_t;

stepper_motor_node_t *motor_ll_head = NULL;

void Stepper_Init(void) {
    __HAL_RCC_TIM16_CLK_ENABLE();

    HAL_NVIC_SetPriority(TIM16_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM16_IRQn);

    htim16.Instance = TIM16;
    htim16.Init.Prescaler = 0;
    htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim16.Init.Period = 10741;
    htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim16.Init.RepetitionCounter = 0;
    htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    TRF_Assert(HAL_TIM_Base_Init(&htim16) == HAL_OK);

    HAL_TIM_Base_Start_IT(&htim16);
}

// Update timer parameters to set steps/sec of stepper motor.
void Stepper_SetSpeed(uint32_t steps_per_second) {
    uint32_t prescaler = (TIM_CLK >> 16) / steps_per_second;
    uint32_t period = (TIM_CLK / (prescaler + 1) + steps_per_second >> 1) / steps_per_second - 1;
    __HAL_TIM_SET_PRESCALER(&htim16, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim16, period);
    __HAL_TIM_SET_COUNTER(&htim16, 0);
}

// Iterates through all initialized stepper motors setting the direction and step pins.
void Stepper_Tick(void) {
    stepper_motor_node_t *current_node = motor_ll_head;
    while(current_node != NULL) {
        stepper_motor_t *motor = current_node->motor;
        int delta_position = motor->target_position - motor->position;

        if (delta_position != 0) {
            bool direction = delta_position > 0;
            if (motor->dir_pin->last_state != direction) {
                GPIO_Pin_Write(motor->dir_pin, direction);
                current_node = current_node->next;
                continue;
            }

            GPIO_Pin_Toggle(motor->step_pin);
            motor->position += direction ? 1 : -1;
        }

        current_node = current_node->next;
    }
}

// Initialize the given stepper_motor_t struct and append it to the linked list of stepper motors.
void Stepper_Motor_Init(stepper_motor_t *stepper_motor, gpio_pin_t *step_pin, gpio_pin_t *dir_pin) {
    stepper_motor->step_pin = step_pin;
    stepper_motor->dir_pin = dir_pin;
    stepper_motor->position = 0;
    stepper_motor->target_position = 0;
    GPIO_Pin_InitOutput(stepper_motor->step_pin);
    GPIO_Pin_InitOutput(stepper_motor->dir_pin);
    GPIO_Pin_Write(stepper_motor->step_pin, 0);
    GPIO_Pin_Write(stepper_motor->dir_pin, 0);

    if (motor_ll_head == NULL) {
        motor_ll_head = malloc(sizeof(stepper_motor_node_t));
        motor_ll_head->motor = stepper_motor;
        motor_ll_head->next = NULL;
    } else {
        stepper_motor_node_t *stepper_motor_lliter = motor_ll_head;
        while (stepper_motor_lliter->next != NULL) {
            stepper_motor_lliter = stepper_motor_lliter->next;
        }
        stepper_motor_lliter->next = malloc(sizeof(stepper_motor_node_t));
        stepper_motor_lliter->next->motor = stepper_motor;
        stepper_motor_lliter->next->next = NULL;
    }
}

// Returns true if the stepper motor has reached its target position.
bool Stepper_Motor_TargetReached(stepper_motor_t *stepper_motor) {
    return stepper_motor->position == stepper_motor->target_position;
}