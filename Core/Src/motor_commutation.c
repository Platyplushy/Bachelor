#include "motor_commutation.h"

#include "hall_state_filter.h"
#include "myprint.h"
#include "pwm_control.h"

#define MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT 10.0f

#define MOTOR_COMMUTATION_PHASE_MAP_UVW_VWU   0U
#define MOTOR_COMMUTATION_PHASE_MAP_WUV_UVW   1U
#define MOTOR_COMMUTATION_PHASE_MAP_WVU       2U
#define MOTOR_COMMUTATION_PHASE_MAP_UWV       3U
#define MOTOR_COMMUTATION_PHASE_MAP_VUW       4U
#define MOTOR_COMMUTATION_PHASE_MAP_VWU       5U

#define MOTOR_COMMUTATION_PHASE_MAP           MOTOR_COMMUTATION_PHASE_MAP_UVW_VWU
#define MOTOR_COMMUTATION_SECTOR_OFFSET        1U

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

static const MotorCommutation_PhaseMap *MotorCommutation_GetPhaseMap(void)
{
    if (MOTOR_COMMUTATION_PHASE_MAP >= (sizeof(kMotorCommutationPhaseMaps) / sizeof(kMotorCommutationPhaseMaps[0]))) {
        Error_Handler();
    }

    return &kMotorCommutationPhaseMaps[MOTOR_COMMUTATION_PHASE_MAP];
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

static void MotorCommutation_ApplySector(uint8_t hall_sector, uint8_t command_sector)
{
    const MotorCommutation_PhaseMap *phase_map = MotorCommutation_GetPhaseMap();
    const MotorCommutation_PhasePattern *pattern;

    if ((command_sector < 1U) || (command_sector > 6U)) {
        PWM_TIM1_SetPhaseState(phase_map->u_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
        PWM_TIM1_SetPhaseState(phase_map->v_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
        PWM_TIM1_SetPhaseState(phase_map->w_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
        return;
    }

    pattern = &kMotorCommutationTable[command_sector - 1U];

    PWM_TIM1_SetPhaseState(phase_map->u_channel, pattern->phase_u, s_motorCommutationDutyPercent);
    PWM_TIM1_SetPhaseState(phase_map->v_channel, pattern->phase_v, s_motorCommutationDutyPercent);
    PWM_TIM1_SetPhaseState(phase_map->w_channel, pattern->phase_w, s_motorCommutationDutyPercent);

    MyPrint_Printf("COMM cfg=%s off=%u hall=%u cmd=%u U->CH%u=%s V->CH%u=%s W->CH%u=%s duty=%.1f\r\n",
                   phase_map->name,
                   (unsigned)MOTOR_COMMUTATION_SECTOR_OFFSET,
                   hall_sector,
                   command_sector,
                   (unsigned)((phase_map->u_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(pattern->phase_u),
                   (unsigned)((phase_map->v_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(pattern->phase_v),
                   (unsigned)((phase_map->w_channel >> 2) + 1U),
                   MotorCommutation_PhaseStateName(pattern->phase_w),
                   (double)s_motorCommutationDutyPercent);
}

void MotorCommutation_Init(void)
{
    const MotorCommutation_PhaseMap *phase_map = MotorCommutation_GetPhaseMap();

    PWM_TIM1_Configure3PhaseComplementary();
    PWM_TIM1_SetPhaseState(phase_map->u_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
    PWM_TIM1_SetPhaseState(phase_map->v_channel, PWM_PHASE_STATE_FLOAT, 0.0f);
    PWM_TIM1_SetPhaseState(phase_map->w_channel, PWM_PHASE_STATE_FLOAT, 0.0f);

    s_motorCommutationDutyPercent = MOTOR_COMMUTATION_DEFAULT_DUTY_PERCENT;
    s_motorCommutationHasState = 0U;
    s_motorCommutationLastHallCode = 0U;

    MyPrint_Printf("Motor commutation ready: cfg=%s off=%u duty=%.1f\r\n",
                   phase_map->name,
                   (unsigned)MOTOR_COMMUTATION_SECTOR_OFFSET,
                   (double)s_motorCommutationDutyPercent);
}

void MotorCommutation_Process(void)
{
    HallStateFilter_State state;
    uint8_t command_sector;

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
