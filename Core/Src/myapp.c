/*
 * myapp.c
 *
 *  Created on: Mar 11, 2026
 *      Author: Gemini CLI
 */

#include "myapp.h"
#include "hall_debug.h"
#include "hall_probe.h"
#include "hall_state_filter.h"
#include "joystick.h"
#include "motor_commutation.h"
#include "myprint.h"
#include "pwm_control.h"
#include "reset_diag.h"
#include "rgb_lcd1602.h"
#include "temperature_sensor.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "stm32g4xx_nucleo.h"
#include "blink_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

static COM_InitTypeDef BspCOMInit;

#define MOTOR_CONTROL_TASK_STACK_WORDS 512U

static osThreadId_t s_motorControlTaskHandle;
static StaticTask_t s_motorControlTaskControlBlock;
static StackType_t s_motorControlTaskStack[MOTOR_CONTROL_TASK_STACK_WORDS];

static void MotorControlTask_Run(void *argument)
{
    (void)argument;

    for (;;) {
        HallStateFilter_Process();
        MotorCommutation_Process();
        osDelay(1U);
    }
}

static void MotorControlTask_Init(void)
{
    const osThreadAttr_t motorControlTaskAttributes = {
        .name = "motorControl",
        .priority = (osPriority_t)osPriorityHigh,
        .cb_mem = &s_motorControlTaskControlBlock,
        .cb_size = sizeof(s_motorControlTaskControlBlock),
        .stack_mem = s_motorControlTaskStack,
        .stack_size = MOTOR_CONTROL_TASK_STACK_WORDS * 4U
    };

    s_motorControlTaskHandle = osThreadNew(MotorControlTask_Run, NULL, &motorControlTaskAttributes);
    (void)s_motorControlTaskHandle;
}

void App_Init(void) {
    /* USER CODE BEGIN App_Init 1 */
    /* USER CODE END App_Init 1 */

    /* PWM Initialisering */
    PWM_Init();
    
    /* KJØR MASKINVARETEST: 20kHz, 50% duty, 120 graders faseforskyvning */
    /* BSP (Board Support Package) Initialisering */
    BSP_LED_Init(LED_GREEN);
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

    /* COM port initialisering (UART) */
    BspCOMInit.BaudRate   = 115200;
    BspCOMInit.WordLength = COM_WORDLENGTH_8B;
    BspCOMInit.StopBits   = COM_STOPBITS_1;
    BspCOMInit.Parity     = COM_PARITY_NONE;
    BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
    if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE) {
        Error_Handler();
    }

    MyPrint_Init();
    ResetDiag_PrintAndClear();
    Joystick_Init();
    RgbLcd1602_Init();
    RgbLcd1602_Clear();
    RgbLcd1602_SetCursor(0U, 0U);
    RgbLcd1602_Print("STM32 Bachelor");
    RgbLcd1602_SetCursor(0U, 1U);
    RgbLcd1602_Print("Motor control");
    HallDebug_Init();
    HallProbe_Init();
    HallStateFilter_Init();
    MotorCommutation_Init();
    TemperatureSensor_Init();

    /* USER CODE BEGIN App_Init 2 */
    /* USER CODE END App_Init 2 */
}

void App_StartTasks(void) {
    BlinkTask_Init();
    MotorControlTask_Init();
}

void App_Run(void) {
    /* USER CODE BEGIN App_Run 1 */
    /* USER CODE END App_Run 1 */
}
