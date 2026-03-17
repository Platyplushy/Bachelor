/*
 * blink_task.c
 *
 *  Created on: Mar 17, 2026
 *      Author: madsj
 */


#include "cmsis_os.h"
#include "main.h"
#include "blink_task.h"

static osThreadId_t blinkTaskHandle;

static void BlinkTask(void *argument)
{
    for(;;)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
        osDelay(500);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
        osDelay(500);
    }
}

void BlinkTask_Init(void)
{
    const osThreadAttr_t blinkTask_attributes = {
        .name = "blinkTask",
        .priority = (osPriority_t) osPriorityLow,
        .stack_size = 128 * 4
    };

    blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
}
