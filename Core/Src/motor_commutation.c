#include "motor_commutation.h"

#include "hall_state_filter.h"
#include "myprint.h"
#include "pwm_control.h"

#define MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT   5.0f
#define MOTOR_COMMUTATION_TARGET_DUTY_PERCENT    60.0f
#define MOTOR_COMMUTATION_RAMP_STEP_PERCENT      0.5f
#define MOTOR_COMMUTATION_RAMP_INTERVAL_MS       100U

#define MOTOR_COMMUTATION_SECTOR_OFFSET          5U
#define MOTOR_COMMUTATION_INVERT_POLARITY        0U
#define MOTOR_COMMUTATION_MODE_HALL              0U
#define MOTOR_COMMUTATION_MODE_MOTOR1_SWEEP      1U
#define MOTOR_COMMUTATION_MODE                   MOTOR_COMMUTATION_MODE_HALL
#define MOTOR_COMMUTATION_SWEEP_INTERVAL_MS      2000U
#define MOTOR_COMMUTATION_SWEEP_START_STEP       1U
#define MOTOR_COMMUTATION_SWEEP_FIXED_DUTY       5.0f
#define MOTOR_COMMUTATION_ENABLE_RUNTIME_PRINTS  0U
#define MOTOR_COMMUTATION_BUTTON_PIN             GPIO_PIN_12
#define MOTOR_COMMUTATION_BUTTON_PORT            GPIOB
#define MOTOR_COMMUTATION_BUTTON_PRESSED_STATE   GPIO_PIN_RESET
#define MOTOR_COMMUTATION_BUTTON_DEBOUNCE_MS     30U

typedef enum {
    MOTOR_COMMUTATION_MOTOR_1 = 0,
    MOTOR_COMMUTATION_MOTOR_2 = 1,
    MOTOR_COMMUTATION_MOTOR_COUNT
} MotorCommutation_MotorIndex;

#define MOTOR_COMMUTATION_ACTIVE_HALL_MOTOR      MOTOR_COMMUTATION_MOTOR_1

typedef void (*MotorCommutation_SetPhaseStateFn)(uint32_t channel, PWM_PhaseState state, float duty);
typedef void (*MotorCommutation_ConfigurePwmFn)(void);

typedef struct {
    PWM_PhaseState phase_u;
    PWM_PhaseState phase_v;
    PWM_PhaseState phase_w;
} MotorCommutation_PhasePattern;

typedef struct {
    const char *name;
    HallStateFilter_MotorIndex hall_index;
    MotorCommutation_ConfigurePwmFn configure_pwm;
    MotorCommutation_SetPhaseStateFn set_phase_state;
} MotorCommutation_MotorConfig;

typedef struct {
    float duty_percent;
    uint8_t last_hall_code;
    uint8_t has_state;
    uint32_t last_ramp_tick_ms;
    uint8_t sweep_step;
    uint32_t last_sweep_tick_ms;
    uint8_t sweep_variant_index;
    uint8_t sweep_step_count_in_variant;
} MotorCommutation_Runtime;

typedef struct {
    const char *name;
    uint32_t channel_for_u;
    uint32_t channel_for_v;
    uint32_t channel_for_w;
    int8_t step_delta;
} MotorCommutation_SweepVariant;

static const MotorCommutation_PhasePattern kMotorCommutationTable[6] = {
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT},
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW},
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW},
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT},
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH},
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH}
};

static const MotorCommutation_MotorConfig kMotorCommutationMotorConfig[MOTOR_COMMUTATION_MOTOR_COUNT] = {
    {"M1", HALL_STATE_FILTER_MOTOR_1, PWM_TIM1_Configure3PhaseComplementary, PWM_TIM1_SetPhaseState},
    {"M2", HALL_STATE_FILTER_MOTOR_2, PWM_TIM8_Configure3PhaseComplementary, PWM_TIM8_SetPhaseState}
};

static const MotorCommutation_SweepVariant kMotorCommutationSweepVariants[] = {
    {"CH123 FWD", TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3,  1},
    {"CH123 REV", TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, -1},
    {"CH132 FWD", TIM_CHANNEL_1, TIM_CHANNEL_3, TIM_CHANNEL_2,  1},
    {"CH132 REV", TIM_CHANNEL_1, TIM_CHANNEL_3, TIM_CHANNEL_2, -1},
    {"CH213 FWD", TIM_CHANNEL_2, TIM_CHANNEL_1, TIM_CHANNEL_3,  1},
    {"CH213 REV", TIM_CHANNEL_2, TIM_CHANNEL_1, TIM_CHANNEL_3, -1},
    {"CH231 FWD", TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_1,  1},
    {"CH231 REV", TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_1, -1},
    {"CH312 FWD", TIM_CHANNEL_3, TIM_CHANNEL_1, TIM_CHANNEL_2,  1},
    {"CH312 REV", TIM_CHANNEL_3, TIM_CHANNEL_1, TIM_CHANNEL_2, -1},
    {"CH321 FWD", TIM_CHANNEL_3, TIM_CHANNEL_2, TIM_CHANNEL_1,  1},
    {"CH321 REV", TIM_CHANNEL_3, TIM_CHANNEL_2, TIM_CHANNEL_1, -1}
};

static const MotorCommutation_SweepVariant kMotorCommutationHallVariant = {
    "CH123",
    TIM_CHANNEL_1,
    TIM_CHANNEL_2,
    TIM_CHANNEL_3,
    1
};

static MotorCommutation_Runtime s_motorRuntime[MOTOR_COMMUTATION_MOTOR_COUNT];
static uint8_t s_motorCommutationEnabled = 0U;
static GPIO_PinState s_motorButtonStableState = GPIO_PIN_SET;
static GPIO_PinState s_motorButtonLastRawState = GPIO_PIN_SET;
static uint32_t s_motorButtonLastChangeTickMs = 0U;

static void MotorCommutation_ApplyPatternWithVariant(MotorCommutation_MotorIndex motor_index,
                                                     const MotorCommutation_PhasePattern *pattern,
                                                     const MotorCommutation_SweepVariant *variant,
                                                     const char *source,
                                                     uint8_t source_index);

static PWM_PhaseState MotorCommutation_ApplyPolarityVariant(PWM_PhaseState state)
{
    if (MOTOR_COMMUTATION_INVERT_POLARITY == 0U) {
        return state;
    }

    switch (state) {
        case PWM_PHASE_STATE_HIGH:
            return PWM_PHASE_STATE_LOW;
        case PWM_PHASE_STATE_LOW:
            return PWM_PHASE_STATE_HIGH;
        case PWM_PHASE_STATE_FLOAT:
        default:
            return state;
    }
}

static void MotorCommutation_DisableMotorOutputs(MotorCommutation_MotorIndex motor_index)
{
    const MotorCommutation_MotorConfig *motor = &kMotorCommutationMotorConfig[motor_index];

    motor->set_phase_state(TIM_CHANNEL_1, PWM_PHASE_STATE_FLOAT, 0.0f);
    motor->set_phase_state(TIM_CHANNEL_2, PWM_PHASE_STATE_FLOAT, 0.0f);
    motor->set_phase_state(TIM_CHANNEL_3, PWM_PHASE_STATE_FLOAT, 0.0f);
}

static void MotorCommutation_DisableAllOutputs(void)
{
    uint32_t i;

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        MotorCommutation_DisableMotorOutputs((MotorCommutation_MotorIndex)i);
    }
}

static void MotorCommutation_ResetRuntimeState(MotorCommutation_MotorIndex motor_index)
{
    s_motorRuntime[motor_index].duty_percent = MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT;
    s_motorRuntime[motor_index].has_state = 0U;
    s_motorRuntime[motor_index].last_hall_code = 0U;
    s_motorRuntime[motor_index].last_ramp_tick_ms = HAL_GetTick();
    s_motorRuntime[motor_index].sweep_step = MOTOR_COMMUTATION_SWEEP_START_STEP;
    s_motorRuntime[motor_index].last_sweep_tick_ms = HAL_GetTick();
    s_motorRuntime[motor_index].sweep_variant_index = 0U;
    s_motorRuntime[motor_index].sweep_step_count_in_variant = 0U;
}

static uint8_t MotorCommutation_ProcessSoftStart(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];

    if (runtime->duty_percent >= MOTOR_COMMUTATION_TARGET_DUTY_PERCENT) {
        return 0U;
    }

    if ((now - runtime->last_ramp_tick_ms) < MOTOR_COMMUTATION_RAMP_INTERVAL_MS) {
        return 0U;
    }

    runtime->last_ramp_tick_ms = now;
    runtime->duty_percent += MOTOR_COMMUTATION_RAMP_STEP_PERCENT;
    if (runtime->duty_percent > MOTOR_COMMUTATION_TARGET_DUTY_PERCENT) {
        runtime->duty_percent = MOTOR_COMMUTATION_TARGET_DUTY_PERCENT;
    }

    return 1U;
}

static uint8_t MotorCommutation_GetCommandSector(uint8_t hall_sector)
{
    if ((hall_sector < 1U) || (hall_sector > 6U)) {
        return 0U;
    }

    return (uint8_t)(((hall_sector - 1U + MOTOR_COMMUTATION_SECTOR_OFFSET) % 6U) + 1U);
}

static void MotorCommutation_ApplyPattern(MotorCommutation_MotorIndex motor_index,
                                          const MotorCommutation_PhasePattern *pattern,
                                          const char *source,
                                          uint8_t source_index)
{
    MotorCommutation_ApplyPatternWithVariant(motor_index,
                                             pattern,
                                             &kMotorCommutationHallVariant,
                                             source,
                                             source_index);
}

static void MotorCommutation_ApplyPatternWithVariant(MotorCommutation_MotorIndex motor_index,
                                                     const MotorCommutation_PhasePattern *pattern,
                                                     const MotorCommutation_SweepVariant *variant,
                                                     const char *source,
                                                     uint8_t source_index)
{
    const MotorCommutation_MotorConfig *motor = &kMotorCommutationMotorConfig[motor_index];
    const float duty = s_motorRuntime[motor_index].duty_percent;
    const PWM_PhaseState phase_u = MotorCommutation_ApplyPolarityVariant(pattern->phase_u);
    const PWM_PhaseState phase_v = MotorCommutation_ApplyPolarityVariant(pattern->phase_v);
    const PWM_PhaseState phase_w = MotorCommutation_ApplyPolarityVariant(pattern->phase_w);

    motor->set_phase_state(variant->channel_for_u, phase_u, duty);
    motor->set_phase_state(variant->channel_for_v, phase_v, duty);
    motor->set_phase_state(variant->channel_for_w, phase_w, duty);

#if MOTOR_COMMUTATION_ENABLE_RUNTIME_PRINTS
    MyPrint_Printf("%s %s %s step=%u duty=%.1f\r\n",
                   motor->name,
                   source,
                   variant->name,
                   source_index,
                   (double)duty);
#else
    (void)source;
    (void)source_index;
#endif
}

static void MotorCommutation_ApplySector(MotorCommutation_MotorIndex motor_index, uint8_t command_sector)
{
    if ((command_sector < 1U) || (command_sector > 6U)) {
        MotorCommutation_DisableMotorOutputs(motor_index);
        return;
    }

    MotorCommutation_ApplyPattern(motor_index,
                                  &kMotorCommutationTable[command_sector - 1U],
                                  "COMM",
                                  command_sector);
}

static void MotorCommutation_ProcessMotor1Sweep(uint32_t now)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1];
    const MotorCommutation_SweepVariant *variant =
        &kMotorCommutationSweepVariants[runtime->sweep_variant_index];
    uint8_t duty_changed = 0U;

    MotorCommutation_DisableMotorOutputs(MOTOR_COMMUTATION_MOTOR_2);

    if ((now - runtime->last_sweep_tick_ms) < MOTOR_COMMUTATION_SWEEP_INTERVAL_MS) {
        duty_changed = MotorCommutation_ProcessSoftStart(MOTOR_COMMUTATION_MOTOR_1, now);
        if (duty_changed != 0U) {
            MotorCommutation_ApplyPatternWithVariant(MOTOR_COMMUTATION_MOTOR_1,
                                                     &kMotorCommutationTable[runtime->sweep_step - 1U],
                                                     variant,
                                                     "SWEEP",
                                                     runtime->sweep_step);
        }
        return;
    }

    runtime->last_sweep_tick_ms = now;

    if ((runtime->sweep_step < 1U) || (runtime->sweep_step > 6U)) {
        runtime->sweep_step = MOTOR_COMMUTATION_SWEEP_START_STEP;
    }

    runtime->duty_percent = MOTOR_COMMUTATION_SWEEP_FIXED_DUTY;
    runtime->last_ramp_tick_ms = now;
    MotorCommutation_ApplyPatternWithVariant(MOTOR_COMMUTATION_MOTOR_1,
                                             &kMotorCommutationTable[runtime->sweep_step - 1U],
                                             variant,
                                             "SWEEP",
                                             runtime->sweep_step);
    MyPrint_Printf("M1 SWEEP: %s step=%u duty=%.1f\r\n",
                   variant->name,
                   (unsigned)runtime->sweep_step,
                   (double)runtime->duty_percent);

    runtime->sweep_step_count_in_variant++;
    if (runtime->sweep_step_count_in_variant >= 6U) {
        runtime->sweep_step_count_in_variant = 0U;
        runtime->sweep_variant_index =
            (uint8_t)((runtime->sweep_variant_index + 1U) %
                      (sizeof(kMotorCommutationSweepVariants) / sizeof(kMotorCommutationSweepVariants[0])));
        variant = &kMotorCommutationSweepVariants[runtime->sweep_variant_index];
        runtime->sweep_step = MOTOR_COMMUTATION_SWEEP_START_STEP;
        MyPrint_Printf("M1 SWEEP variant=%s\r\n", variant->name);
        return;
    }

    if (variant->step_delta > 0) {
        runtime->sweep_step = (uint8_t)((runtime->sweep_step % 6U) + 1U);
    } else {
        runtime->sweep_step = (runtime->sweep_step == 1U) ? 6U : (uint8_t)(runtime->sweep_step - 1U);
    }
}

static void MotorCommutation_SetEnabled(uint8_t enabled)
{
    uint32_t i;

    if (enabled == s_motorCommutationEnabled) {
        return;
    }

    s_motorCommutationEnabled = enabled;

    if (s_motorCommutationEnabled == 0U) {
        MotorCommutation_DisableAllOutputs();
        MyPrint_Print("Motors disabled: PB12 toggled OFF\r\n");
        return;
    }

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        MotorCommutation_ResetRuntimeState((MotorCommutation_MotorIndex)i);
    }

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_MOTOR1_SWEEP) {
        s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].duty_percent = MOTOR_COMMUTATION_SWEEP_FIXED_DUTY;
        MotorCommutation_DisableMotorOutputs(MOTOR_COMMUTATION_MOTOR_2);
        MotorCommutation_ApplyPatternWithVariant(
            MOTOR_COMMUTATION_MOTOR_1,
            &kMotorCommutationTable[s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].sweep_step - 1U],
            &kMotorCommutationSweepVariants[s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].sweep_variant_index],
            "SWEEP",
            s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].sweep_step);
        MyPrint_Printf("Motors enabled: M1 sweep ON, variant=%s duty=%.1f step=%u\r\n",
                       kMotorCommutationSweepVariants[s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].sweep_variant_index].name,
                       (double)s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].duty_percent,
                       (unsigned)s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].sweep_step);
        s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].last_sweep_tick_ms = HAL_GetTick();
        return;
    }

    MyPrint_Printf("Motors enabled: %s hall ON, soft-start %.1f%% -> %.1f%%\r\n",
                   kMotorCommutationMotorConfig[MOTOR_COMMUTATION_ACTIVE_HALL_MOTOR].name,
                   (double)MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT,
                   (double)MOTOR_COMMUTATION_TARGET_DUTY_PERCENT);
}

static void MotorCommutation_ProcessButton(uint32_t now)
{
    const GPIO_PinState raw_state = HAL_GPIO_ReadPin(MOTOR_COMMUTATION_BUTTON_PORT, MOTOR_COMMUTATION_BUTTON_PIN);

    if (raw_state != s_motorButtonLastRawState) {
        MyPrint_Printf("PB12 raw=%u\r\n", (unsigned)((raw_state == GPIO_PIN_SET) ? 1U : 0U));
        s_motorButtonLastRawState = raw_state;
        s_motorButtonLastChangeTickMs = now;
    }

    if ((now - s_motorButtonLastChangeTickMs) < MOTOR_COMMUTATION_BUTTON_DEBOUNCE_MS) {
        return;
    }

    if (raw_state == s_motorButtonStableState) {
        return;
    }

    s_motorButtonStableState = raw_state;
    MyPrint_Printf("PB12 debounced=%u\r\n", (unsigned)((s_motorButtonStableState == GPIO_PIN_SET) ? 1U : 0U));
    if (s_motorButtonStableState == MOTOR_COMMUTATION_BUTTON_PRESSED_STATE) {
        MyPrint_Print("PB12 press accepted\r\n");
        MotorCommutation_SetEnabled((uint8_t)(s_motorCommutationEnabled == 0U));
    }
}

static void MotorCommutation_ProcessMotor(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    HallStateFilter_State state;
    const MotorCommutation_MotorConfig *motor = &kMotorCommutationMotorConfig[motor_index];
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];
    uint8_t command_sector;
    uint8_t duty_changed;

    duty_changed = MotorCommutation_ProcessSoftStart(motor_index, now);

    if (HallStateFilter_GetLatestState(motor->hall_index, &state) == 0U) {
        return;
    }

    if ((runtime->has_state != 0U) && (state.hall_code == runtime->last_hall_code)) {
        if (duty_changed != 0U) {
            command_sector = MotorCommutation_GetCommandSector(state.sector);
            MotorCommutation_ApplySector(motor_index, command_sector);
        }
        return;
    }

    runtime->last_hall_code = state.hall_code;
    runtime->has_state = 1U;

    command_sector = MotorCommutation_GetCommandSector(state.sector);
    MotorCommutation_ApplySector(motor_index, command_sector);
}

void MotorCommutation_Init(void)
{
    uint32_t i;

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        kMotorCommutationMotorConfig[i].configure_pwm();
    }
    MotorCommutation_DisableAllOutputs();

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        MotorCommutation_ResetRuntimeState((MotorCommutation_MotorIndex)i);
    }

    s_motorCommutationEnabled = 0U;
    s_motorButtonStableState = HAL_GPIO_ReadPin(MOTOR_COMMUTATION_BUTTON_PORT, MOTOR_COMMUTATION_BUTTON_PIN);
    s_motorButtonLastRawState = s_motorButtonStableState;
    s_motorButtonLastChangeTickMs = HAL_GetTick();

    MyPrint_Printf("Motor commutation ready: mode=%u cfg=%s off=%u inv=%u duty=%.1f->%.1f M1=PC7/PC1/PC9 M2=PC2/PC3/PC8\r\n",
                   (unsigned)MOTOR_COMMUTATION_MODE,
                   kMotorCommutationHallVariant.name,
                   (unsigned)MOTOR_COMMUTATION_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_INVERT_POLARITY,
                   (double)MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT,
                   (double)MOTOR_COMMUTATION_TARGET_DUTY_PERCENT);
}

void MotorCommutation_Process(void)
{
    const uint32_t now = HAL_GetTick();

    MotorCommutation_ProcessButton(now);

    if (s_motorCommutationEnabled == 0U) {
        return;
    }

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_MOTOR1_SWEEP) {
        MotorCommutation_ProcessMotor1Sweep(now);
        return;
    }

    if (MOTOR_COMMUTATION_ACTIVE_HALL_MOTOR == MOTOR_COMMUTATION_MOTOR_1) {
        MotorCommutation_ProcessMotor(MOTOR_COMMUTATION_MOTOR_1, now);
        MotorCommutation_DisableMotorOutputs(MOTOR_COMMUTATION_MOTOR_2);
        return;
    }

    MotorCommutation_DisableMotorOutputs(MOTOR_COMMUTATION_MOTOR_1);
    MotorCommutation_ProcessMotor(MOTOR_COMMUTATION_MOTOR_2, now);
}

void MotorCommutation_SetDuty(float duty_percent)
{
    uint32_t i;

    if (duty_percent < 0.0f) {
        duty_percent = 0.0f;
    }
    if (duty_percent > 100.0f) {
        duty_percent = 100.0f;
    }

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        s_motorRuntime[i].duty_percent = duty_percent;
    }
}

uint8_t MotorCommutation_UsesHallInputs(void)
{
    return (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_HALL) ? 1U : 0U;
}
