/*
 * pwm_control.c
 *
 *  Created on: Mar 2, 2026
 */

#include "pwm_control.h"

#define PWM_TEST_STATE_COUNT            15U
#define PWM_TEST_STEP_FREQUENCY_HZ      300000U
#define PWM_TEST_TIM_PERIOD             ((170000000U / PWM_TEST_STEP_FREQUENCY_HZ) - 1U)
#define PWM_TEST_HIGH_A_PIN             GPIO_PIN_8
#define PWM_TEST_HIGH_B_PIN             GPIO_PIN_9
#define PWM_TEST_HIGH_C_PIN             GPIO_PIN_10
#define PWM_TEST_LOW_A_PIN              GPIO_PIN_13
#define PWM_TEST_LOW_B_PIN              GPIO_PIN_14
#define PWM_TEST_LOW_C_PIN              GPIO_PIN_15

typedef struct {
    GPIO_PinState high_a;
    GPIO_PinState high_b;
    GPIO_PinState high_c;
} PWM_TestState;

static const PWM_TestState kPwmTestStateTable[PWM_TEST_STATE_COUNT] = {
    {GPIO_PIN_SET,   GPIO_PIN_RESET, GPIO_PIN_SET},
    {GPIO_PIN_SET,   GPIO_PIN_RESET, GPIO_PIN_SET},
    {GPIO_PIN_SET,   GPIO_PIN_RESET, GPIO_PIN_SET},
    {GPIO_PIN_SET,   GPIO_PIN_RESET, GPIO_PIN_SET},
    {GPIO_PIN_SET,   GPIO_PIN_RESET, GPIO_PIN_SET},
    {GPIO_PIN_SET,   GPIO_PIN_SET,   GPIO_PIN_RESET},
    {GPIO_PIN_SET,   GPIO_PIN_SET,   GPIO_PIN_RESET},
    {GPIO_PIN_SET,   GPIO_PIN_SET,   GPIO_PIN_RESET},
    {GPIO_PIN_SET,   GPIO_PIN_SET,   GPIO_PIN_RESET},
    {GPIO_PIN_SET,   GPIO_PIN_SET,   GPIO_PIN_RESET},
    {GPIO_PIN_RESET, GPIO_PIN_SET,   GPIO_PIN_SET},
    {GPIO_PIN_RESET, GPIO_PIN_SET,   GPIO_PIN_SET},
    {GPIO_PIN_RESET, GPIO_PIN_SET,   GPIO_PIN_SET},
    {GPIO_PIN_RESET, GPIO_PIN_SET,   GPIO_PIN_SET},
    {GPIO_PIN_RESET, GPIO_PIN_SET,   GPIO_PIN_SET}
};

static volatile uint32_t s_pwmTestStateIndex = 0U;

static void PWM_TestApplyHighSideState(uint32_t state_index) {
    const PWM_TestState *state = &kPwmTestStateTable[state_index % PWM_TEST_STATE_COUNT];
    uint32_t set_mask_a = 0U;
    uint32_t reset_mask_a = PWM_TEST_HIGH_A_PIN | PWM_TEST_HIGH_B_PIN | PWM_TEST_HIGH_C_PIN;
    uint32_t set_mask_b = 0U;
    uint32_t reset_mask_b = PWM_TEST_LOW_A_PIN | PWM_TEST_LOW_B_PIN | PWM_TEST_LOW_C_PIN;

    if (state->high_a == GPIO_PIN_SET) {
        set_mask_a |= PWM_TEST_HIGH_A_PIN;
    } else {
        set_mask_b |= PWM_TEST_LOW_A_PIN;
    }
    if (state->high_b == GPIO_PIN_SET) {
        set_mask_a |= PWM_TEST_HIGH_B_PIN;
    } else {
        set_mask_b |= PWM_TEST_LOW_B_PIN;
    }
    if (state->high_c == GPIO_PIN_SET) {
        set_mask_a |= PWM_TEST_HIGH_C_PIN;
    } else {
        set_mask_b |= PWM_TEST_LOW_C_PIN;
    }

    GPIOA->BSRR = ((reset_mask_a & ~set_mask_a) << 16U) | set_mask_a;
    GPIOB->BSRR = ((reset_mask_b & ~set_mask_b) << 16U) | set_mask_b;
}

static void PWM_TestConfigurePins(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_DeInit(GPIOA, PWM_TEST_HIGH_A_PIN | PWM_TEST_HIGH_B_PIN | PWM_TEST_HIGH_C_PIN);
    HAL_GPIO_DeInit(GPIOB, PWM_TEST_LOW_A_PIN | PWM_TEST_LOW_B_PIN | PWM_TEST_LOW_C_PIN);

    HAL_GPIO_WritePin(GPIOA, PWM_TEST_HIGH_A_PIN | PWM_TEST_HIGH_B_PIN | PWM_TEST_HIGH_C_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, PWM_TEST_LOW_A_PIN | PWM_TEST_LOW_B_PIN | PWM_TEST_LOW_C_PIN, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = PWM_TEST_HIGH_A_PIN | PWM_TEST_HIGH_B_PIN | PWM_TEST_HIGH_C_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PWM_TEST_LOW_A_PIN | PWM_TEST_LOW_B_PIN | PWM_TEST_LOW_C_PIN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

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
    /* Stopp TIM1 før rekonfigurering */
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
    HAL_TIM_Base_Stop_IT(&htim1);
    __HAL_TIM_MOE_DISABLE(&htim1);
    __HAL_TIM_DISABLE(&htim1);
    __HAL_TIM_DISABLE_IT(&htim1, TIM_IT_UPDATE);
    htim1.Instance->CCER = 0U;
    htim1.Instance->CCMR1 = 0U;
    htim1.Instance->CCMR2 = 0U;
    htim1.Instance->CCMR3 = 0U;

    PWM_TestConfigurePins();

    htim1.Init.Prescaler = 0U;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = PWM_TEST_TIM_PERIOD;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0U;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    s_pwmTestStateIndex = 0U;
    PWM_TestApplyHighSideState(s_pwmTestStateIndex);
    __HAL_TIM_SET_COUNTER(&htim1, 0U);
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

    if (HAL_TIM_Base_Start_IT(&htim1) != HAL_OK) {
        Error_Handler();
    }
}

void PWM_TIM1_UpdateIRQHandler(void) {
    if ((__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET) &&
        (__HAL_TIM_GET_IT_SOURCE(&htim1, TIM_IT_UPDATE) != RESET)) {
        __HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
        s_pwmTestStateIndex = (s_pwmTestStateIndex + 1U) % PWM_TEST_STATE_COUNT;
        PWM_TestApplyHighSideState(s_pwmTestStateIndex);
    }
}
