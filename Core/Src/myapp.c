/*
 * myapp.c
 *
 *  Created on: Mar 11, 2026
 *      Author: Gemini CLI
 */

#include "myapp.h"
#include "pwm_control.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "stm32g4xx_nucleo.h"
#include "blink_task.h"

static COM_InitTypeDef BspCOMInit;
static const float kPhaseDutyTable[12] = {
    50.0f, 70.0f, 84.6f, 90.0f, 84.6f, 70.0f,
    50.0f, 30.0f, 15.4f, 10.0f, 15.4f, 30.0f
};
static uint32_t s_lastPhaseUpdateMs = 0U;
static uint32_t s_phaseIndex = 0U;

void App_Init(void) {
    /* USER CODE BEGIN App_Init 1 */
	BlinkTask_Init();
    /* USER CODE END App_Init 1 */

    /* PWM Initialisering */
    PWM_Init();
    
    /* KJØR MASKINVARETEST: 20kHz, 50% duty, 120 graders faseforskyvning */
    PWM_TIM1_Configure3PhaseComplementary();
    PWM_SetMotor1PhaseDuties(kPhaseDutyTable[0], kPhaseDutyTable[4], kPhaseDutyTable[8]);

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

    /* USER CODE BEGIN App_Init 2 */
    /* USER CODE END App_Init 2 */
}

void App_Run(void) {
    /* USER CODE BEGIN App_Run 1 */
    uint32_t now = HAL_GetTick();

    if ((now - s_lastPhaseUpdateMs) >= 1U) {
        s_lastPhaseUpdateMs = now;
        s_phaseIndex = (s_phaseIndex + 1U) % 12U;

        PWM_SetMotor1PhaseDuties(
            kPhaseDutyTable[s_phaseIndex],
            kPhaseDutyTable[(s_phaseIndex + 4U) % 12U],
            kPhaseDutyTable[(s_phaseIndex + 8U) % 12U]
        );
    }
    /* USER CODE END App_Run 1 */
}
