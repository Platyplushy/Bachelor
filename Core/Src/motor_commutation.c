#include "motor_commutation.h"

#include "hall_state_filter.h"
#include "myprint.h"
#include "pwm_control.h"

#define MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT      5.0f

#define MOTOR_COMMUTATION_PHASE_MAP_UVW_VWU         0U
#define MOTOR_COMMUTATION_PHASE_MAP_WUV_UVW         1U
#define MOTOR_COMMUTATION_PHASE_MAP_WVU             2U
#define MOTOR_COMMUTATION_PHASE_MAP_UWV             3U
#define MOTOR_COMMUTATION_PHASE_MAP_VUW             4U
#define MOTOR_COMMUTATION_PHASE_MAP_VWU             5U

#define MOTOR_COMMUTATION_PHASE_MAP                 MOTOR_COMMUTATION_PHASE_MAP_UVW_VWU
#define MOTOR_COMMUTATION_SECTOR_OFFSET            0U
#define MOTOR_COMMUTATION_INVERT_POLARITY          0U
#define MOTOR_COMMUTATION_MODE_HALL                0U
#define MOTOR_COMMUTATION_MODE_MANUAL_PROBE        1U
#define MOTOR_COMMUTATION_MODE_OPEN_LOOP           2U
#define MOTOR_COMMUTATION_MODE_DISABLED            3U
#define MOTOR_COMMUTATION_MODE                     MOTOR_COMMUTATION_MODE_HALL
#define MOTOR_COMMUTATION_MANUAL_STEP              3U
#define MOTOR_COMMUTATION_MANUAL_RAMP_STEP_PERCENT 2.5f
#define MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT  25.0f
#define MOTOR_COMMUTATION_MANUAL_RAMP_PERIOD_MS    400U
#define MOTOR_COMMUTATION_OPEN_LOOP_STEP_PERIOD_MS 150U
#define MOTOR_COMMUTATION_ENABLE_RUNTIME_PRINTS    1U
#define MOTOR_COMMUTATION_BUTTON_PIN               GPIO_PIN_12
#define MOTOR_COMMUTATION_BUTTON_PORT              GPIOB
#define MOTOR_COMMUTATION_BUTTON_PRESSED_STATE     GPIO_PIN_RESET
#define MOTOR_COMMUTATION_BUTTON_DEBOUNCE_MS       30U

typedef struct {
    uint32_t u_channel;
    uint32_t v_channel;
    uint32_t w_channel;
    const char *name;
} MotorCommutation_PhaseMap;

typedef struct {
    PWM_PhaseState phase_u;
    PWM_PhaseState phase_v;
    PWM_PhaseState phase_w;
} MotorCommutation_PhasePattern;

static const MotorCommutation_PhasePattern kMotorCommutationTable[6] = {
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT},
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW},
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW},
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT},
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH},
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH}
};

static const MotorCommutation_PhasePattern kMotorCommutationManualProbeTable[6] = {
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT}, /* U+ V- */
    {PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW},   /* U+ W- */
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_LOW},   /* V+ W- */
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH,  PWM_PHASE_STATE_FLOAT}, /* V+ U- */
    {PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_HIGH},  /* W+ U- */
    {PWM_PHASE_STATE_FLOAT, PWM_PHASE_STATE_LOW,   PWM_PHASE_STATE_HIGH}   /* W+ V- */
};

static const MotorCommutation_PhaseMap kMotorCommutationPhaseMaps[] = {
    {TIM_CHANNEL_3, TIM_CHANNEL_2, TIM_CHANNEL_1, "UVW->CH321"},
    {TIM_CHANNEL_1, TIM_CHANNEL_3, TIM_CHANNEL_2, "UVW->CH132"},
    {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, "UVW->CH123"},
    {TIM_CHANNEL_3, TIM_CHANNEL_1, TIM_CHANNEL_2, "UVW->CH312"},
    {TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_1, "UVW->CH231"},
    {TIM_CHANNEL_2, TIM_CHANNEL_1, TIM_CHANNEL_3, "UVW->CH213"}
};

static float s_motorCommutationDutyPercent = MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT;
static uint8_t s_motorCommutationLastHallCode = 0U;
static uint8_t s_motorCommutationHasState = 0U;
static uint32_t s_motorCommutationLastRampTickMs = 0U;
static uint8_t s_motorCommutationOpenLoopStep = 1U;
static uint8_t s_motorCommutationEnabled = 0U;
static GPIO_PinState s_motorButtonStableState = GPIO_PIN_SET;
static GPIO_PinState s_motorButtonLastRawState = GPIO_PIN_SET;
static uint32_t s_motorButtonLastChangeTickMs = 0U;

static void MotorCommutation_ApplyManualProbe(void);
static void MotorCommutation_ApplyOpenLoopStep(uint8_t step);

static const MotorCommutation_PhaseMap *MotorCommutation_GetPhaseMap(void)
{
    if (MOTOR_COMMUTATION_PHASE_MAP >= (sizeof(kMotorCommutationPhaseMaps) / sizeof(kMotorCommutationPhaseMaps[0]))) {
        Error_Handler();
    }

    return &kMotorCommutationPhaseMaps[MOTOR_COMMUTATION_PHASE_MAP];
}

static void MotorCommutation_DisableOutputs(void)
{
    const MotorCommutation_PhaseMap *phase_map = MotorCommutation_GetPhaseMap();

    PWM_TIM1_SetPhaseState(phase_map->u_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
    PWM_TIM1_SetPhaseState(phase_map->v_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
    PWM_TIM1_SetPhaseState(phase_map->w_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
}

static void MotorCommutation_ResetRuntimeState(void)
{
    s_motorCommutationDutyPercent = MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT;
    s_motorCommutationHasState = 0U;
    s_motorCommutationLastHallCode = 0U;
    s_motorCommutationLastRampTickMs = HAL_GetTick();
    s_motorCommutationOpenLoopStep = 1U;
}

static void MotorCommutation_SetEnabled(uint8_t enabled)
{
    if (enabled == s_motorCommutationEnabled) {
        return;
    }

    s_motorCommutationEnabled = enabled;

    if (s_motorCommutationEnabled == 0U) {
        MotorCommutation_DisableOutputs();
        MyPrint_Print("Motor disabled: PB12 toggled OFF\r\n");
        return;
    }

    MotorCommutation_ResetRuntimeState();
    MyPrint_Print("Motor enabled: PB12 toggled ON\r\n");

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_MANUAL_PROBE) {
        MotorCommutation_ApplyManualProbe();
    } else if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_OPEN_LOOP) {
        MotorCommutation_ApplyOpenLoopStep(s_motorCommutationOpenLoopStep);
    }
}

static void MotorCommutation_ProcessButton(uint32_t now)
{
    GPIO_PinState raw_state = HAL_GPIO_ReadPin(MOTOR_COMMUTATION_BUTTON_PORT, MOTOR_COMMUTATION_BUTTON_PIN);

    if (raw_state != s_motorButtonLastRawState) {
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
    if (s_motorButtonStableState == MOTOR_COMMUTATION_BUTTON_PRESSED_STATE) {
        MotorCommutation_SetEnabled((uint8_t)(s_motorCommutationEnabled == 0U));
    }
}

static uint8_t MotorCommutation_GetCommandSector(uint8_t hall_sector)
{
    if ((hall_sector < 1U) || (hall_sector > 6U)) {
        return 0U;
    }

    return (uint8_t)(((hall_sector - 1U + MOTOR_COMMUTATION_SECTOR_OFFSET) % 6U) + 1U);
}

static const char *MotorCommutation_PhaseStateName(PWM_PhaseState state)
{
    switch (state) {
        case PWM_PHASE_STATE_FLOAT:
            return "FLOAT";
        case PWM_PHASE_STATE_HIGH:
            return "HIGH";
        case PWM_PHASE_STATE_LOW:
            return "LOW";
        default:
            return "UNKNOWN";
    }
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

static void MotorCommutation_ApplyPattern(const MotorCommutation_PhasePattern *pattern,
                                          const char *source,
                                          uint8_t source_index)
{
    const MotorCommutation_PhaseMap *phase_map = MotorCommutation_GetPhaseMap();
    PWM_PhaseState phase_u = MotorCommutation_ApplyPolarityVariant(pattern->phase_u);
    PWM_PhaseState phase_v = MotorCommutation_ApplyPolarityVariant(pattern->phase_v);
    PWM_PhaseState phase_w = MotorCommutation_ApplyPolarityVariant(pattern->phase_w);

    PWM_TIM1_SetPhaseState(phase_map->u_channel, phase_u, s_motorCommutationDutyPercent);
    PWM_TIM1_SetPhaseState(phase_map->v_channel, phase_v, s_motorCommutationDutyPercent);
    PWM_TIM1_SetPhaseState(phase_map->w_channel, phase_w, s_motorCommutationDutyPercent);

#if MOTOR_COMMUTATION_ENABLE_RUNTIME_PRINTS
    MyPrint_Printf("%s cfg=%s off=%u inv=%u step=%u U->CH%u=%s V->CH%u=%s W->CH%u=%s duty=%.1f\r\n",
                   source,
                   phase_map->name,
                   (unsigned)MOTOR_COMMUTATION_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_INVERT_POLARITY,
                   source_index,
                   (unsigned)((phase_map->u_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(phase_u),
                   (unsigned)((phase_map->v_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(phase_v),
                   (unsigned)((phase_map->w_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(phase_w),
                   (double)s_motorCommutationDutyPercent);
#else
    (void)source;
    (void)source_index;
#endif
}

static void MotorCommutation_ApplySector(uint8_t hall_sector, uint8_t command_sector)
{
    const MotorCommutation_PhasePattern *pattern;
    (void)hall_sector;

    if ((command_sector < 1U) || (command_sector > 6U)) {
        MotorCommutation_DisableOutputs();
        return;
    }

    pattern = &kMotorCommutationTable[command_sector - 1U];
    MotorCommutation_ApplyPattern(pattern, "COMM", command_sector);
}

static void MotorCommutation_ApplyManualProbe(void)
{
    const uint8_t step_index = (uint8_t)(MOTOR_COMMUTATION_MANUAL_STEP - 1U);

    if ((MOTOR_COMMUTATION_MANUAL_STEP < 1U) || (MOTOR_COMMUTATION_MANUAL_STEP > 6U)) {
        Error_Handler();
    }

    MotorCommutation_ApplyPattern(&kMotorCommutationManualProbeTable[step_index],
                                  "COMM MANUAL",
                                  MOTOR_COMMUTATION_MANUAL_STEP);
}

static void MotorCommutation_ApplyOpenLoopStep(uint8_t step)
{
    const uint8_t step_index = (uint8_t)(step - 1U);

    if ((step < 1U) || (step > 6U)) {
        Error_Handler();
    }

    MotorCommutation_ApplyPattern(&kMotorCommutationManualProbeTable[step_index],
                                  "COMM OPEN",
                                  step);
}

void MotorCommutation_Init(void)
{
    const MotorCommutation_PhaseMap *phase_map = MotorCommutation_GetPhaseMap();

    PWM_TIM1_Configure3PhaseComplementary();
    MotorCommutation_DisableOutputs();

    MotorCommutation_ResetRuntimeState();
    s_motorCommutationEnabled = 0U;
    s_motorButtonStableState = HAL_GPIO_ReadPin(MOTOR_COMMUTATION_BUTTON_PORT, MOTOR_COMMUTATION_BUTTON_PIN);
    s_motorButtonLastRawState = s_motorButtonStableState;
    s_motorButtonLastChangeTickMs = HAL_GetTick();

    MyPrint_Printf("Motor commutation ready: mode=%u cfg=%s off=%u inv=%u duty=%.1f pwm=10kHz PB12=toggle\r\n",
                   (unsigned)MOTOR_COMMUTATION_MODE,
                   phase_map->name,
                   (unsigned)MOTOR_COMMUTATION_SECTOR_OFFSET,
                   (unsigned)MOTOR_COMMUTATION_INVERT_POLARITY,
                   (double)s_motorCommutationDutyPercent);
}

void MotorCommutation_Process(void)
{
    HallStateFilter_State state;
    uint8_t command_sector;
    uint32_t now;

    now = HAL_GetTick();
    MotorCommutation_ProcessButton(now);

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_DISABLED) {
        MotorCommutation_DisableOutputs();
        return;
    }

    if (s_motorCommutationEnabled == 0U) {
        return;
    }

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_MANUAL_PROBE) {
        if ((now - s_motorCommutationLastRampTickMs) >= MOTOR_COMMUTATION_MANUAL_RAMP_PERIOD_MS) {
            s_motorCommutationLastRampTickMs = now;

            if (s_motorCommutationDutyPercent < MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT) {
                s_motorCommutationDutyPercent += MOTOR_COMMUTATION_MANUAL_RAMP_STEP_PERCENT;
                if (s_motorCommutationDutyPercent > MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT) {
                    s_motorCommutationDutyPercent = MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT;
                }

                MotorCommutation_ApplyManualProbe();
            }
        }

        return;
    }

    if (MOTOR_COMMUTATION_MODE == MOTOR_COMMUTATION_MODE_OPEN_LOOP) {
        if ((now - s_motorCommutationLastRampTickMs) >= MOTOR_COMMUTATION_MANUAL_RAMP_PERIOD_MS) {
            s_motorCommutationLastRampTickMs = now;

            if (s_motorCommutationDutyPercent < MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT) {
                s_motorCommutationDutyPercent += MOTOR_COMMUTATION_MANUAL_RAMP_STEP_PERCENT;
                if (s_motorCommutationDutyPercent > MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT) {
                    s_motorCommutationDutyPercent = MOTOR_COMMUTATION_MANUAL_RAMP_MAX_PERCENT;
                }
            }
        }

        static uint32_t s_lastOpenLoopStepTickMs = 0U;
        if ((now - s_lastOpenLoopStepTickMs) >= MOTOR_COMMUTATION_OPEN_LOOP_STEP_PERIOD_MS) {
            s_lastOpenLoopStepTickMs = now;
            s_motorCommutationOpenLoopStep = (uint8_t)((s_motorCommutationOpenLoopStep % 6U) + 1U);
            MotorCommutation_ApplyOpenLoopStep(s_motorCommutationOpenLoopStep);
        }

        return;
    }

    if (HallStateFilter_GetLatestState(&state) == 0U) {
        return;
    }

    if ((s_motorCommutationHasState != 0U) && (state.hall_code == s_motorCommutationLastHallCode)) {
        return;
    }

    s_motorCommutationLastHallCode = state.hall_code;
    s_motorCommutationHasState = 1U;

    command_sector = MotorCommutation_GetCommandSector(state.sector);
    MotorCommutation_ApplySector(state.sector, command_sector);
}

void MotorCommutation_SetDuty(float duty_percent)
{
    if (duty_percent < 0.0f) {
        duty_percent = 0.0f;
    }
    if (duty_percent > 100.0f) {
        duty_percent = 100.0f;
    }

    s_motorCommutationDutyPercent = duty_percent;
}
