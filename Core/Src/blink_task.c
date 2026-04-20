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

static void BlinkTask_InitPin(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);

    gpio_init.Pin = GPIO_PIN_10;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio_init);
}

static void BlinkTask(void *argument)
{
    for(;;)
    {
        osDelay(500);

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
