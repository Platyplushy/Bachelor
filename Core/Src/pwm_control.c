/*
 * pwm_control.c
 *
 *  Created on: Mar 2, 2026
 */

#include "pwm_control.h"

static uint32_t PWM_GetTimClockHz(void) {
    uint32_t pclk = HAL_RCC_GetPCLK2Freq();
    uint32_t ppre = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;

    if (ppre >= 4) {
        return pclk * 2U;
    }

    return pclk;
}

static uint8_t PWM_ComputeDeadtimeDTG(uint32_t deadtime_ns, uint32_t tim_clk_hz) {
    uint64_t ticks = ((uint64_t)deadtime_ns * (uint64_t)tim_clk_hz + 1000000000ULL - 1ULL)
                   / 1000000000ULL;

    if (ticks <= 127ULL) {
        return (uint8_t)ticks;
    }

    if (ticks <= 190ULL) {
        uint64_t dtg = ((ticks + 1ULL) / 2ULL) - 64ULL;
        if (dtg > 31ULL) {
            dtg = 31ULL;
        }
        return (uint8_t)(0x80U | (uint8_t)dtg);
    }

    if (ticks <= 504ULL) {
        uint64_t dtg = ((ticks + 7ULL) / 8ULL) - 32ULL;
        if (dtg > 31ULL) {
            dtg = 31ULL;
        }
        return (uint8_t)(0xC0U | (uint8_t)dtg);
    }

    if (ticks <= 1008ULL) {
        uint64_t dtg = ((ticks + 15ULL) / 16ULL) - 32ULL;
        if (dtg > 31ULL) {
            dtg = 31ULL;
        }
        return (uint8_t)(0xE0U | (uint8_t)dtg);
    }

    return 0xFFU;
}

static void PWM_ApplyDeadtime(TIM_HandleTypeDef *htim, uint32_t deadtime_ns) {
    TIM_BreakDeadTimeConfigTypeDef bdtr = {0};
    uint32_t tim_clk_hz = PWM_GetTimClockHz();
    uint8_t dtg = PWM_ComputeDeadtimeDTG(deadtime_ns, tim_clk_hz);

    bdtr.OffStateRunMode = TIM_OSSR_ENABLE;
    bdtr.OffStateIDLEMode = TIM_OSSI_ENABLE;
    bdtr.LockLevel = TIM_LOCKLEVEL_OFF;
    bdtr.DeadTime = dtg;
    bdtr.BreakState = TIM_BREAK_DISABLE;
    bdtr.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    bdtr.BreakFilter = 0;
    bdtr.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
    bdtr.Break2State = TIM_BREAK2_DISABLE;
    bdtr.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    bdtr.Break2Filter = 0;
    bdtr.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
    bdtr.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

    (void)HAL_TIMEx_ConfigBreakDeadTime(htim, &bdtr);
}

void PWM_Init(void) {
    PWM_ApplyDeadtime(&htim1, 400U);
    PWM_ApplyDeadtime(&htim8, 400U);

    PWM_SetMotor1Duty(0.0f);
    PWM_SetMotor2Duty(0.0f);
}

void PWM_TIM1_Configure3PhaseComplementary(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    const uint32_t period = 4249U;
    const uint32_t pulse = (period + 1U) / 2U;

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

    __HAL_TIM_DISABLE(&htim1);

    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
    htim1.Init.Period = period;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
        Error_Handler();
    }

    PWM_ApplyDeadtime(&htim1, 400U);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pulse);
    HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE);

    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
        Error_Handler();
    }

    __HAL_TIM_MOE_ENABLE(&htim1);
}

/**
 * @brief Starter PWM på kanal 1 til 3 for TIM1.
 */
void PWM_Start(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    
    // Starte komplementære kanaler (N-utganger) siden TIM1 er en Advanced Timer
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
}

void PWM_StartMotor1(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

    __HAL_TIM_MOE_ENABLE(&htim1);
}

void PWM_StartMotor2(void) {
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);

    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);

    __HAL_TIM_MOE_ENABLE(&htim8);
}

/**
 * @brief Setter duty cycle (0-100%) for en kanal på TIM1.
 * @param channel TIM_CHANNEL_1, TIM_CHANNEL_2 eller TIM_CHANNEL_3
 * @param duty Prosentverdi (0.0 til 100.0)
 */
void PWM_Set_DutyCycle(uint32_t channel, float duty) {
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 100.0f) duty = 100.0f;

    // ARR er 8499, så pulsverdien beregnes ut fra dette.
    // Puls = (ARR + 1) * duty / 100
    uint32_t pulse = (uint32_t)(((float)(htim1.Instance->ARR + 1) * duty) / 100.0f);
    
    __HAL_TIM_SET_COMPARE(&htim1, channel, pulse);
}

static void PWM_SetAllChannels(TIM_HandleTypeDef *htim, float duty) {
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 100.0f) duty = 100.0f;

    uint32_t pulse = (uint32_t)(((float)(htim->Instance->ARR + 1) * duty) / 100.0f);

    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, pulse);
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, pulse);
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, pulse);
}

void PWM_SetMotor1Duty(float duty) {
    PWM_SetAllChannels(&htim1, duty);
}

void PWM_SetMotor1PhaseDuties(float duty_a, float duty_b, float duty_c) {
    PWM_Set_DutyCycle(TIM_CHANNEL_1, duty_a);
    PWM_Set_DutyCycle(TIM_CHANNEL_2, duty_b);
    PWM_Set_DutyCycle(TIM_CHANNEL_3, duty_c);
}

void PWM_SetMotor2Duty(float duty) {
    PWM_SetAllChannels(&htim8, duty);
}

void PWM_HardwareTest_3Phase(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* Stopp TIM1 før rekonfigurering */
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

    /* Sett Deadtime til 1.0 us for sikkerhet under maskinvaretest */
    PWM_ApplyDeadtime(&htim1, 1000U);

    /* Konfigurer Fase A (0 grader): Høy mellom 0 og 4250 */
    sConfigOC.OCMode = TIM_OCMODE_ASYMMETRIC_PWM2;
    sConfigOC.Pulse = 0;       /* CCR1 */
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 4250); /* CCR2 definerer slutten */

    /* Konfigurer Fase B (120 grader): Høy mellom 2833 og 7083 */
    sConfigOC.OCMode = TIM_OCMODE_ASYMMETRIC_PWM2;
    sConfigOC.Pulse = 2833;    /* CCR3 */
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 7083); /* CCR4 definerer slutten */

    /* Konfigurer Fase C (240 grader): Høy når CNT < 1416 ELLER CNT > 5666 (Wrappet puls) */
    sConfigOC.OCMode = TIM_OCMODE_ASYMMETRIC_PWM1;
    sConfigOC.Pulse = 1416;    /* CCR5 */
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_5);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_6, 5666); /* CCR6 definerer starten */

    /* Start PWM med komplementære utganger */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

    /* Aktiver Main Output Enable (MOE) */
    __HAL_TIM_MOE_ENABLE(&htim1);
}
