#include "motor_commutation.h"

#include "hall_state_filter.h"
#include "myprint.h"
#include "pwm_control.h"

#define MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT   15.0f
#define MOTOR_COMMUTATION_MOTOR2_START_DUTY_PERCENT 20.0f
#define MOTOR_COMMUTATION_MOTOR2_DUTY_BIAS_PERCENT 5.3f
#define MOTOR_COMMUTATION_MOTOR1_REVERSE_START_DUTY_PERCENT 16.0f
#define MOTOR_COMMUTATION_MOTOR1_REVERSE_RPM_SLEW_STEP_PERCENT 1.0f
#define MOTOR_COMMUTATION_REFERENCE_RPM          220.0f
#define MOTOR_COMMUTATION_REFERENCE_SPEED_KMH    7.0f
#define MOTOR_COMMUTATION_TARGET_SPEED_KMH       7.0f
#define MOTOR_COMMUTATION_REFERENCE_DUTY_PERCENT 60.0f
#define MOTOR_COMMUTATION_MAX_DUTY_PERCENT       90.0f
#define MOTOR_COMMUTATION_TARGET_RPM             ((MOTOR_COMMUTATION_TARGET_SPEED_KMH * MOTOR_COMMUTATION_REFERENCE_RPM) / MOTOR_COMMUTATION_REFERENCE_SPEED_KMH)
#define MOTOR_COMMUTATION_TARGET_DUTY_PERCENT    (MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT + ((MOTOR_COMMUTATION_TARGET_RPM / MOTOR_COMMUTATION_REFERENCE_RPM) * (MOTOR_COMMUTATION_REFERENCE_DUTY_PERCENT - MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT)))
#define MOTOR_COMMUTATION_RPM_TRANSITIONS_PER_REV 90.0f
#define MOTOR_COMMUTATION_RPM_CONTROL_INTERVAL_MS 50U
#define MOTOR_COMMUTATION_RPM_SLEW_STEP_PERCENT   2.0f
#define MOTOR_COMMUTATION_RPM_CONTROL_KP          0.30f
#define MOTOR_COMMUTATION_RPM_TIMEOUT_MS          200U
#define MOTOR_COMMUTATION_COMMAND_SLEW_INTERVAL_MS 50U
#define MOTOR_COMMUTATION_COMMAND_SLEW_STEP        2

#define MOTOR_COMMUTATION_SECTOR_OFFSET          5U
#define MOTOR_COMMUTATION_MOTOR1_FORWARD_SECTOR_OFFSET 5U
#define MOTOR_COMMUTATION_MOTOR1_REVERSE_SECTOR_OFFSET 2U
#define MOTOR_COMMUTATION_MOTOR2_FORWARD_SECTOR_OFFSET 2U
#define MOTOR_COMMUTATION_MOTOR2_REVERSE_SECTOR_OFFSET MOTOR_COMMUTATION_SECTOR_OFFSET
#define MOTOR_COMMUTATION_INVERT_POLARITY        0U
#define MOTOR_COMMUTATION_MODE_HALL              0U
#define MOTOR_COMMUTATION_MODE_MOTOR1_SWEEP      1U
#define MOTOR_COMMUTATION_MODE                   MOTOR_COMMUTATION_MODE_HALL
#define MOTOR_COMMUTATION_SWEEP_INTERVAL_MS      2000U
#define MOTOR_COMMUTATION_SWEEP_START_STEP       1U
#define MOTOR_COMMUTATION_SWEEP_FIXED_DUTY       5.0f
#define MOTOR_COMMUTATION_ENABLE_RUNTIME_PRINTS  0U
#define MOTOR_COMMUTATION_ENABLE_MOTOR1          1U
#define MOTOR_COMMUTATION_ENABLE_MOTOR2          1U
#define MOTOR_COMMUTATION_BUTTON_PIN             GPIO_PIN_12
#define MOTOR_COMMUTATION_BUTTON_PORT            GPIOB
#define MOTOR_COMMUTATION_BUTTON_PRESSED_STATE   GPIO_PIN_RESET
#define MOTOR_COMMUTATION_BUTTON_DEBOUNCE_MS     30U
#define MOTOR_COMMUTATION_BUTTON_STARTUP_IGNORE_MS 500U
#define MOTOR_COMMUTATION_ENABLE_PB12_BUTTON     0U
typedef enum {
    MOTOR_COMMUTATION_MOTOR_1 = 0,
    MOTOR_COMMUTATION_MOTOR_2 = 1,
    MOTOR_COMMUTATION_MOTOR_COUNT
} MotorCommutation_MotorIndex;

typedef enum {
    MOTOR_COMMUTATION_DIRECTION_FORWARD = 0,
    MOTOR_COMMUTATION_DIRECTION_REVERSE = 1
} MotorCommutation_Direction;

#define MOTOR_COMMUTATION_MOTOR1_DIRECTION MOTOR_COMMUTATION_DIRECTION_FORWARD
#define MOTOR_COMMUTATION_MOTOR2_DIRECTION MOTOR_COMMUTATION_DIRECTION_REVERSE

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
    float measured_rpm;
    uint8_t last_hall_code;
    uint8_t has_state;
    uint32_t last_rpm_control_tick_ms;
    uint32_t last_hall_tick_ms;
    uint32_t last_transition_cycles;
    uint32_t last_transition_sequence;
    uint8_t has_transition_time;
    uint8_t sweep_step;
    uint32_t last_sweep_tick_ms;
    uint8_t sweep_variant_index;
    uint8_t sweep_step_count_in_variant;
    int16_t requested_percent;
    int16_t command_percent;
    float target_rpm;
    uint8_t command_enabled;
    uint8_t command_forward;
    uint32_t last_command_slew_tick_ms;
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

static const uint8_t kMotorCommutationMotorEnabled[MOTOR_COMMUTATION_MOTOR_COUNT] = {
    MOTOR_COMMUTATION_ENABLE_MOTOR1,
    MOTOR_COMMUTATION_ENABLE_MOTOR2
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
static volatile uint8_t s_motorCommutationToggleRequested = 0U;
static GPIO_PinState s_motorButtonStableState = GPIO_PIN_SET;
static GPIO_PinState s_motorButtonLastRawState = GPIO_PIN_SET;
static uint32_t s_motorButtonLastChangeTickMs = 0U;
static uint32_t s_motorButtonIgnoreUntilTickMs = 0U;

static void MotorCommutation_ApplyPatternWithVariant(MotorCommutation_MotorIndex motor_index,
                                                     const MotorCommutation_PhasePattern *pattern,
                                                     const MotorCommutation_SweepVariant *variant,
                                                     const char *source,
                                                     uint8_t source_index);
static void MotorCommutation_ProcessMotor(MotorCommutation_MotorIndex motor_index, uint32_t now);
static uint8_t MotorCommutation_TryGetMotorIndexFromHallIndex(HallStateFilter_MotorIndex hall_index,
                                                              MotorCommutation_MotorIndex *motor_index);
static float MotorCommutation_GetStartDutyPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward);
static float MotorCommutation_GetDutyBiasPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward);
static float MotorCommutation_GetRpmDutySlewStepPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward);
static uint8_t MotorCommutation_GetSectorOffset(MotorCommutation_MotorIndex motor_index, uint8_t command_forward);

static int16_t MotorCommutation_ClampCommand(int16_t command)
{
    if (command > 100) {
        return 100;
    }
    if (command < -100) {
        return -100;
    }

    return command;
}

static uint8_t MotorCommutation_HasOppositeSign(int16_t a, int16_t b)
{
    return (((a > 0) && (b < 0)) || ((a < 0) && (b > 0))) ? 1U : 0U;
}

static int16_t MotorCommutation_StepToward(int16_t current, int16_t target)
{
    if (current < target) {
        current = (int16_t)(current + MOTOR_COMMUTATION_COMMAND_SLEW_STEP);
        if (current > target) {
            current = target;
        }
        return current;
    }

    if (current > target) {
        current = (int16_t)(current - MOTOR_COMMUTATION_COMMAND_SLEW_STEP);
        if (current < target) {
            current = target;
        }
    }

    return current;
}

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

static uint8_t MotorCommutation_TryGetMotorIndexFromHallIndex(HallStateFilter_MotorIndex hall_index,
                                                              MotorCommutation_MotorIndex *motor_index)
{
    uint32_t i;

    if (motor_index == NULL) {
        return 0U;
    }

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        if (kMotorCommutationMotorConfig[i].hall_index == hall_index) {
            *motor_index = (MotorCommutation_MotorIndex)i;
            return 1U;
        }
    }

    return 0U;
}

static void MotorCommutation_ResetRuntimeState(MotorCommutation_MotorIndex motor_index)
{
    s_motorRuntime[motor_index].duty_percent = MotorCommutation_GetStartDutyPercent(motor_index, 1U);
    s_motorRuntime[motor_index].measured_rpm = 0.0f;
    s_motorRuntime[motor_index].has_state = 0U;
    s_motorRuntime[motor_index].last_hall_code = 0U;
    s_motorRuntime[motor_index].last_rpm_control_tick_ms = HAL_GetTick();
    s_motorRuntime[motor_index].last_hall_tick_ms = 0U;
    s_motorRuntime[motor_index].last_transition_cycles = 0U;
    s_motorRuntime[motor_index].last_transition_sequence = 0U;
    s_motorRuntime[motor_index].has_transition_time = 0U;
    s_motorRuntime[motor_index].sweep_step = MOTOR_COMMUTATION_SWEEP_START_STEP;
    s_motorRuntime[motor_index].last_sweep_tick_ms = HAL_GetTick();
    s_motorRuntime[motor_index].sweep_variant_index = 0U;
    s_motorRuntime[motor_index].sweep_step_count_in_variant = 0U;
    s_motorRuntime[motor_index].requested_percent = 0;
    s_motorRuntime[motor_index].command_percent = 0;
    s_motorRuntime[motor_index].target_rpm = 0.0f;
    s_motorRuntime[motor_index].command_enabled = 0U;
    s_motorRuntime[motor_index].command_forward = 1U;
    s_motorRuntime[motor_index].last_command_slew_tick_ms = HAL_GetTick();

    if (kMotorCommutationMotorEnabled[motor_index] == 0U) {
        s_motorRuntime[motor_index].requested_percent = 0;
        s_motorRuntime[motor_index].command_percent = 0;
        s_motorRuntime[motor_index].target_rpm = 0.0f;
        s_motorRuntime[motor_index].command_enabled = 0U;
        s_motorRuntime[motor_index].duty_percent = 0.0f;
    }
}

static void MotorCommutation_ApplyAppliedCommand(MotorCommutation_MotorIndex motor_index, int16_t command)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];
    int16_t magnitude;
    uint8_t command_forward;

    command = MotorCommutation_ClampCommand(command);
    runtime->command_percent = command;

    if (command == 0) {
        runtime->command_enabled = 0U;
        runtime->target_rpm = 0.0f;
        runtime->command_forward = 1U;
        runtime->duty_percent = 0.0f;
        runtime->measured_rpm = 0.0f;
        runtime->has_transition_time = 0U;
        runtime->last_transition_cycles = 0U;
        runtime->last_transition_sequence = 0U;
        runtime->last_hall_tick_ms = 0U;
        return;
    }

    command_forward = (command > 0) ? 1U : 0U;
    if (runtime->command_enabled == 0U) {
        runtime->duty_percent = MotorCommutation_GetStartDutyPercent(motor_index, command_forward);
        runtime->measured_rpm = 0.0f;
        runtime->has_transition_time = 0U;
        runtime->last_transition_cycles = 0U;
        runtime->last_transition_sequence = 0U;
        runtime->last_hall_tick_ms = 0U;
        runtime->last_rpm_control_tick_ms = HAL_GetTick();
    }

    runtime->command_enabled = 1U;
    runtime->command_forward = command_forward;
    magnitude = (command > 0) ? command : (int16_t)(-command);
    runtime->target_rpm = ((float)magnitude * MOTOR_COMMUTATION_TARGET_RPM) / 100.0f;
}

static void MotorCommutation_ProcessCommandSlew(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];
    int16_t target = runtime->requested_percent;
    int16_t next = runtime->command_percent;

    if ((now - runtime->last_command_slew_tick_ms) < MOTOR_COMMUTATION_COMMAND_SLEW_INTERVAL_MS) {
        return;
    }

    runtime->last_command_slew_tick_ms = now;

    if (MotorCommutation_HasOppositeSign(next, target) != 0U) {
        target = 0;
    }

    next = MotorCommutation_StepToward(next, target);
    if (next != runtime->command_percent) {
        MotorCommutation_ApplyAppliedCommand(motor_index, next);
    }
}

static uint8_t MotorCommutation_ProcessRpmControl(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];
    float desired_duty;
    float feedforward_duty;
    float error_rpm;
    float duty_slew_step;

    if ((now - runtime->last_rpm_control_tick_ms) < MOTOR_COMMUTATION_RPM_CONTROL_INTERVAL_MS) {
        return 0U;
    }

    runtime->last_rpm_control_tick_ms = now;

    if ((runtime->last_hall_tick_ms == 0U) ||
        ((now - runtime->last_hall_tick_ms) > MOTOR_COMMUTATION_RPM_TIMEOUT_MS)) {
        runtime->measured_rpm = 0.0f;
    }

    if (runtime->command_enabled == 0U) {
        if (runtime->duty_percent != 0.0f) {
            runtime->duty_percent = 0.0f;
            return 1U;
        }
        return 0U;
    }

    feedforward_duty = MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT +
                       ((runtime->target_rpm / MOTOR_COMMUTATION_REFERENCE_RPM) *
                        (MOTOR_COMMUTATION_REFERENCE_DUTY_PERCENT - MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT));

    error_rpm = runtime->target_rpm - runtime->measured_rpm;
    desired_duty = feedforward_duty + (MOTOR_COMMUTATION_RPM_CONTROL_KP * error_rpm);
    duty_slew_step = MotorCommutation_GetRpmDutySlewStepPercent(motor_index, runtime->command_forward);

    desired_duty += MotorCommutation_GetDutyBiasPercent(motor_index, runtime->command_forward);

    if (desired_duty < MotorCommutation_GetStartDutyPercent(motor_index, runtime->command_forward)) {
        desired_duty = MotorCommutation_GetStartDutyPercent(motor_index, runtime->command_forward);
    }
    if (desired_duty > MOTOR_COMMUTATION_MAX_DUTY_PERCENT) {
        desired_duty = MOTOR_COMMUTATION_MAX_DUTY_PERCENT;
    }

    if (runtime->duty_percent < desired_duty) {
        runtime->duty_percent += duty_slew_step;
        if (runtime->duty_percent > desired_duty) {
            runtime->duty_percent = desired_duty;
        }
        return 1U;
    }

    if (runtime->duty_percent > desired_duty) {
        runtime->duty_percent -= duty_slew_step;
        if (runtime->duty_percent < desired_duty) {
            runtime->duty_percent = desired_duty;
        }
        return 1U;
    }

    return 0U;
}

static float MotorCommutation_GetStartDutyPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward)
{
    if (motor_index == MOTOR_COMMUTATION_MOTOR_2) {
        return MOTOR_COMMUTATION_MOTOR2_START_DUTY_PERCENT;
    }

    if ((motor_index == MOTOR_COMMUTATION_MOTOR_1) && (command_forward == 0U)) {
        return MOTOR_COMMUTATION_MOTOR1_REVERSE_START_DUTY_PERCENT;
    }

    return MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT;
}

static float MotorCommutation_GetDutyBiasPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward)
{
    if (motor_index == MOTOR_COMMUTATION_MOTOR_2) {
        return MOTOR_COMMUTATION_MOTOR2_DUTY_BIAS_PERCENT;
    }

    if ((motor_index == MOTOR_COMMUTATION_MOTOR_1) && (command_forward == 0U)) {
        return 0.0f;
    }

    return 0.0f;
}

static float MotorCommutation_GetRpmDutySlewStepPercent(MotorCommutation_MotorIndex motor_index, uint8_t command_forward)
{
    if ((motor_index == MOTOR_COMMUTATION_MOTOR_1) && (command_forward == 0U)) {
        return MOTOR_COMMUTATION_MOTOR1_REVERSE_RPM_SLEW_STEP_PERCENT;
    }

    return MOTOR_COMMUTATION_RPM_SLEW_STEP_PERCENT;
}

static uint8_t MotorCommutation_GetSectorOffset(MotorCommutation_MotorIndex motor_index, uint8_t command_forward)
{
    if (motor_index == MOTOR_COMMUTATION_MOTOR_1) {
        if (command_forward == 0U) {
            return MOTOR_COMMUTATION_MOTOR1_REVERSE_SECTOR_OFFSET;
        }

        return MOTOR_COMMUTATION_MOTOR1_FORWARD_SECTOR_OFFSET;
    }

    if (command_forward == 0U) {
        return MOTOR_COMMUTATION_MOTOR2_REVERSE_SECTOR_OFFSET;
    }

    return MOTOR_COMMUTATION_MOTOR2_FORWARD_SECTOR_OFFSET;
}

static uint8_t MotorCommutation_ProcessSweepRamp(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];

    if (runtime->duty_percent >= MOTOR_COMMUTATION_TARGET_DUTY_PERCENT) {
        return 0U;
    }

    if ((now - runtime->last_rpm_control_tick_ms) < MOTOR_COMMUTATION_RPM_CONTROL_INTERVAL_MS) {
        return 0U;
    }

    runtime->last_rpm_control_tick_ms = now;
    runtime->duty_percent += MOTOR_COMMUTATION_RPM_SLEW_STEP_PERCENT;
    if (runtime->duty_percent > MOTOR_COMMUTATION_TARGET_DUTY_PERCENT) {
        runtime->duty_percent = MOTOR_COMMUTATION_TARGET_DUTY_PERCENT;
    }

    return 1U;
}

static uint8_t MotorCommutation_GetCommandSector(MotorCommutation_MotorIndex motor_index, uint8_t hall_sector)
{
    uint8_t offset = MotorCommutation_GetSectorOffset(motor_index, s_motorRuntime[motor_index].command_forward);

    if ((hall_sector < 1U) || (hall_sector > 6U)) {
        return 0U;
    }

    return (uint8_t)(((hall_sector - 1U + offset) % 6U) + 1U);
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
        duty_changed = MotorCommutation_ProcessSweepRamp(MOTOR_COMMUTATION_MOTOR_1, now);
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
    runtime->last_rpm_control_tick_ms = now;
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

    MyPrint_Printf("Motors enabled: M1+M2 hall ON, joystick max=%u kmh duty max=%u\r\n",
                   (unsigned)MOTOR_COMMUTATION_TARGET_SPEED_KMH,
                   (unsigned)MOTOR_COMMUTATION_MAX_DUTY_PERCENT);
}

#if MOTOR_COMMUTATION_ENABLE_PB12_BUTTON
static void MotorCommutation_ProcessButton(uint32_t now)
{
    const GPIO_PinState raw_state = HAL_GPIO_ReadPin(MOTOR_COMMUTATION_BUTTON_PORT, MOTOR_COMMUTATION_BUTTON_PIN);

    if (now < s_motorButtonIgnoreUntilTickMs) {
        s_motorButtonLastRawState = raw_state;
        s_motorButtonStableState = raw_state;
        s_motorButtonLastChangeTickMs = now;
        return;
    }

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
#endif

static void MotorCommutation_ProcessMotor(MotorCommutation_MotorIndex motor_index, uint32_t now)
{
    HallStateFilter_State state;
    const MotorCommutation_MotorConfig *motor = &kMotorCommutationMotorConfig[motor_index];
    MotorCommutation_Runtime *runtime = &s_motorRuntime[motor_index];
    uint8_t command_sector;
    uint8_t duty_changed = 0U;

    if (kMotorCommutationMotorEnabled[motor_index] == 0U) {
        MotorCommutation_DisableMotorOutputs(motor_index);
        runtime->requested_percent = 0;
        runtime->command_percent = 0;
        runtime->command_enabled = 0U;
        runtime->target_rpm = 0.0f;
        runtime->duty_percent = 0.0f;
        runtime->measured_rpm = 0.0f;
        runtime->has_transition_time = 0U;
        runtime->last_transition_sequence = 0U;
        return;
    }

    MotorCommutation_ProcessCommandSlew(motor_index, now);

    if (runtime->command_enabled == 0U) {
        MotorCommutation_DisableMotorOutputs(motor_index);
        runtime->measured_rpm = 0.0f;
        runtime->has_transition_time = 0U;
        runtime->last_transition_sequence = 0U;
        return;
    }

    if (HallStateFilter_GetLatestState(motor->hall_index, &state) == 0U) {
        return;
    }

    duty_changed = MotorCommutation_ProcessRpmControl(motor_index, now);

    if ((runtime->has_state != 0U) &&
        (duty_changed != 0U)) {
        command_sector = MotorCommutation_GetCommandSector(motor_index, state.sector);
        MotorCommutation_ApplySector(motor_index, command_sector);
    }
    (void)duty_changed;
}

void MotorCommutation_OnHallStateAccepted(HallStateFilter_MotorIndex hall_index,
                                          const HallStateFilter_State *state)
{
    MotorCommutation_MotorIndex motor_index;
    MotorCommutation_Runtime *runtime;
    uint8_t command_sector;

    if ((state == NULL) ||
        (MOTOR_COMMUTATION_MODE != MOTOR_COMMUTATION_MODE_HALL) ||
        (s_motorCommutationEnabled == 0U)) {
        return;
    }

    if (MotorCommutation_TryGetMotorIndexFromHallIndex(hall_index, &motor_index) == 0U) {
        return;
    }

    if (kMotorCommutationMotorEnabled[motor_index] == 0U) {
        return;
    }

    runtime = &s_motorRuntime[motor_index];
    if (runtime->command_enabled == 0U) {
        return;
    }

    runtime->last_hall_code = state->hall_code;
    runtime->has_state = 1U;
    runtime->last_hall_tick_ms = state->transition_tick_ms;

    if (runtime->has_transition_time != 0U) {
        const uint32_t delta_cycles = state->transition_cycles - runtime->last_transition_cycles;
        if (delta_cycles != 0U) {
            const float transition_hz = ((float)HAL_RCC_GetHCLKFreq()) / (float)delta_cycles;
            runtime->measured_rpm = (transition_hz * 60.0f) / MOTOR_COMMUTATION_RPM_TRANSITIONS_PER_REV;
        }
    }

    runtime->last_transition_cycles = state->transition_cycles;
    runtime->last_transition_sequence = state->transition_sequence;
    runtime->has_transition_time = 1U;

    command_sector = MotorCommutation_GetCommandSector(motor_index, state->sector);
    MotorCommutation_ApplySector(motor_index, command_sector);
}

void MotorCommutation_Init(void)
{
    uint32_t i;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0U;

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
    s_motorButtonIgnoreUntilTickMs = s_motorButtonLastChangeTickMs + MOTOR_COMMUTATION_BUTTON_STARTUP_IGNORE_MS;

    MyPrint_Printf("Motor commutation ready: mode=%u cfg=%s off=%u/%u/%u/%u inv=%u dir=%u/%u speed max=%u kmh duty max=%u\r\n",
                   (unsigned)MOTOR_COMMUTATION_MODE,
                   kMotorCommutationHallVariant.name,
                   (unsigned)MOTOR_COMMUTATION_MOTOR1_FORWARD_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_MOTOR1_REVERSE_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_MOTOR2_FORWARD_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_MOTOR2_REVERSE_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_INVERT_POLARITY,
                   (unsigned)MOTOR_COMMUTATION_MOTOR1_DIRECTION,
                   (unsigned)MOTOR_COMMUTATION_MOTOR2_DIRECTION,
                   (unsigned)MOTOR_COMMUTATION_TARGET_SPEED_KMH,
                   (unsigned)MOTOR_COMMUTATION_MAX_DUTY_PERCENT);
    MyPrint_Print("Motor commutation pins: M1=PC7/PC1/PC9 M2=PC2/PC3/PC8\r\n");
}

void MotorCommutation_Process(void)
{
    const uint32_t now = HAL_GetTick();

    if (s_motorCommutationToggleRequested != 0U) {
        s_motorCommutationToggleRequested = 0U;
        MotorCommutation_SetEnabled((uint8_t)(s_motorCommutationEnabled == 0U));
    }

#if MOTOR_COMMUTATION_ENABLE_PB12_BUTTON
    MotorCommutation_ProcessButton(now);
#else
    (void)now;
#endif

    if (s_motorCommutationEnabled == 0U) {
        return;
    }

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_MOTOR1_SWEEP) {
        MotorCommutation_ProcessMotor1Sweep(now);
        return;
    }

    MotorCommutation_ProcessMotor(MOTOR_COMMUTATION_MOTOR_1, now);
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

void MotorCommutation_SetWheelCommands(int16_t motor1_percent, int16_t motor2_percent)
{
    int16_t commands[MOTOR_COMMUTATION_MOTOR_COUNT] = {motor1_percent, motor2_percent};
    uint32_t i;

    for (i = 0U; i < MOTOR_COMMUTATION_MOTOR_COUNT; ++i) {
        if (kMotorCommutationMotorEnabled[i] == 0U) {
            s_motorRuntime[i].requested_percent = 0;
        } else {
            s_motorRuntime[i].requested_percent = MotorCommutation_ClampCommand(commands[i]);
        }
    }
}

void MotorCommutation_ToggleEnabled(void)
{
    s_motorCommutationToggleRequested = 1U;
}

float MotorCommutation_GetMotor1SpeedKmh(void)
{
    return (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].measured_rpm *
            MOTOR_COMMUTATION_REFERENCE_SPEED_KMH) /
           MOTOR_COMMUTATION_REFERENCE_RPM;
}

float MotorCommutation_GetMotor2SpeedKmh(void)
{
    return (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_2].measured_rpm *
            MOTOR_COMMUTATION_REFERENCE_SPEED_KMH) /
           MOTOR_COMMUTATION_REFERENCE_RPM;
}

float MotorCommutation_GetVehicleSpeedKmh(void)
{
    float motor1_speed = MotorCommutation_GetMotor1SpeedKmh();
    float motor2_speed = MotorCommutation_GetMotor2SpeedKmh();

    if (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].command_enabled != 0U) {
        if (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_1].command_forward == 0U) {
            motor1_speed = -motor1_speed;
        }
    } else {
        motor1_speed = 0.0f;
    }

    if (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_2].command_enabled != 0U) {
        if (s_motorRuntime[MOTOR_COMMUTATION_MOTOR_2].command_forward == 0U) {
            motor2_speed = -motor2_speed;
        }
    } else {
        motor2_speed = 0.0f;
    }

    return (motor1_speed + motor2_speed) * 0.5f;
}

uint8_t MotorCommutation_IsEnabled(void)
{
    return s_motorCommutationEnabled;
}

uint8_t MotorCommutation_UsesHallInputs(void)
{
    return (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_HALL) ? 1U : 0U;
}
