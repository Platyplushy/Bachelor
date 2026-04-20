#include "hall_state_filter.h"

#include "motor_commutation.h"
#include "myprint.h"

#define HALL_FILTER_SAMPLE_PERIOD_MS      0U
#define HALL_FILTER_STABLE_COUNT          1U
#define HALL_FILTER_INVALID_CODE          0xFFU
#define HALL_FILTER_ENABLE_RUNTIME_PRINTS 0U
#define HALL_FILTER_MIN_TRANSITION_MS     0U

typedef struct {
    uint8_t hall_code;
    uint8_t sector;
} HallCodeSectorMap;

typedef struct {
    GPIO_TypeDef *u_port;
    uint16_t u_pin;
    GPIO_TypeDef *v_port;
    uint16_t v_pin;
    GPIO_TypeDef *w_port;
    uint16_t w_pin;
    const char *name;
} HallFilter_MotorPins;

typedef struct {
    HallStateFilter_State latest_state;
    uint8_t has_latest_state;
    uint8_t candidate_hall_code;
    uint8_t candidate_stable_count;
    uint32_t last_sample_tick;
    uint32_t last_accept_tick;
    uint32_t transition_sequence;
} HallFilter_Runtime;

static const HallCodeSectorMap kHallSequence[] = {
    {0x1U, 1U}, /* UVW = 001 */
    {0x5U, 2U}, /* UVW = 101 */
    {0x4U, 3U}, /* UVW = 100 */
    {0x6U, 4U}, /* UVW = 110 */
    {0x2U, 5U}, /* UVW = 010 */
    {0x3U, 6U}  /* UVW = 011 */
};

static const HallFilter_MotorPins kHallFilterMotorPins[HALL_STATE_FILTER_MOTOR_COUNT] = {
    {GPIOC, GPIO_PIN_7, GPIOC, GPIO_PIN_1, GPIOC, GPIO_PIN_9, "M1"},
    {GPIOC, GPIO_PIN_2, GPIOC, GPIO_PIN_3, GPIOC, GPIO_PIN_8, "M2"}
};

static HallFilter_Runtime s_hallFilterRuntime[HALL_STATE_FILTER_MOTOR_COUNT];
static uint8_t HallStateFilter_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static uint8_t HallStateFilter_ReadHallCode(HallStateFilter_MotorIndex motor_index)
{
    const HallFilter_MotorPins *pins = &kHallFilterMotorPins[motor_index];
    const uint8_t hall_u = HallStateFilter_ReadPin(pins->u_port, pins->u_pin);
    const uint8_t hall_v = HallStateFilter_ReadPin(pins->v_port, pins->v_pin);
    const uint8_t hall_w = HallStateFilter_ReadPin(pins->w_port, pins->w_pin);

    return (uint8_t)((hall_u << 2) | (hall_v << 1) | hall_w);
}

static uint8_t HallStateFilter_FindSequenceIndex(uint8_t hall_code)
{
    uint32_t i;

    for (i = 0U; i < (sizeof(kHallSequence) / sizeof(kHallSequence[0])); ++i) {
        if (kHallSequence[i].hall_code == hall_code) {
            return (uint8_t)i;
        }
    }

    return HALL_FILTER_INVALID_CODE;
}

static uint8_t HallStateFilter_IsAdjacent(uint8_t previous_code, uint8_t next_code)
{
    uint8_t previous_index;
    uint8_t next_index;
    const uint8_t sequence_size = (uint8_t)(sizeof(kHallSequence) / sizeof(kHallSequence[0]));

    previous_index = HallStateFilter_FindSequenceIndex(previous_code);
    next_index = HallStateFilter_FindSequenceIndex(next_code);

    if ((previous_index == HALL_FILTER_INVALID_CODE) || (next_index == HALL_FILTER_INVALID_CODE)) {
        return 0U;
    }

    if ((uint8_t)((previous_index + 1U) % sequence_size) == next_index) {
        return 1U;
    }
    if ((uint8_t)((next_index + 1U) % sequence_size) == previous_index) {
        return 1U;
    }

    return 0U;
}

static uint8_t HallStateFilter_FillState(uint8_t hall_code, HallStateFilter_State *state)
{
    uint8_t sequence_index;

    sequence_index = HallStateFilter_FindSequenceIndex(hall_code);
    if (sequence_index == HALL_FILTER_INVALID_CODE) {
        return 0U;
    }

    state->hall_code = hall_code;
    state->hall_u = (uint8_t)((hall_code >> 2) & 0x1U);
    state->hall_v = (uint8_t)((hall_code >> 1) & 0x1U);
    state->hall_w = (uint8_t)(hall_code & 0x1U);
    state->sector = kHallSequence[sequence_index].sector;

    return 1U;
}

static uint32_t HallStateFilter_GetCycleCounter(void)
{
    return DWT->CYCCNT;
}

static void HallStateFilter_AcceptState(HallStateFilter_MotorIndex motor_index,
                                        uint8_t hall_code,
                                        uint32_t transition_tick_ms)
{
    HallStateFilter_State state;
    HallFilter_Runtime *runtime = &s_hallFilterRuntime[motor_index];

    if (HallStateFilter_FillState(hall_code, &state) == 0U) {
        return;
    }

    runtime->transition_sequence++;
    state.transition_cycles = HallStateFilter_GetCycleCounter();
    state.transition_tick_ms = transition_tick_ms;
    state.transition_sequence = runtime->transition_sequence;

    runtime->latest_state = state;
    runtime->has_latest_state = 1U;
    MotorCommutation_OnHallStateAccepted(motor_index, &state);

#if HALL_FILTER_ENABLE_RUNTIME_PRINTS
    if (motor_index == HALL_STATE_FILTER_MOTOR_2) {
        MyPrint_Printf("HALL %s: sector=%u UVW=%u%u%u code=0x%X\r\n",
                       kHallFilterMotorPins[motor_index].name,
                       state.sector,
                       state.hall_u,
                       state.hall_v,
                       state.hall_w,
                       state.hall_code);
    }
#endif
}

static void HallStateFilter_TryAcceptState(HallStateFilter_MotorIndex motor_index, uint8_t hall_code)
{
    HallFilter_Runtime *runtime = &s_hallFilterRuntime[motor_index];
    const uint32_t now = HAL_GetTick();

    if (HallStateFilter_FindSequenceIndex(hall_code) == HALL_FILTER_INVALID_CODE) {
        runtime->candidate_hall_code = HALL_FILTER_INVALID_CODE;
        runtime->candidate_stable_count = 0U;
        return;
    }

    if ((runtime->has_latest_state != 0U) && (hall_code == runtime->latest_state.hall_code)) {
        return;
    }

    if ((runtime->has_latest_state != 0U) &&
        ((now - runtime->last_accept_tick) < HALL_FILTER_MIN_TRANSITION_MS)) {
        return;
    }

    if ((runtime->has_latest_state != 0U) &&
        (HallStateFilter_IsAdjacent(runtime->latest_state.hall_code, hall_code) == 0U)) {
        return;
    }

    runtime->candidate_hall_code = hall_code;
    runtime->candidate_stable_count = HALL_FILTER_STABLE_COUNT;
    HallStateFilter_AcceptState(motor_index, hall_code, now);
    runtime->last_accept_tick = now;
}

static void HallStateFilter_ProcessMotor(HallStateFilter_MotorIndex motor_index, uint32_t now)
{
    HallFilter_Runtime *runtime = &s_hallFilterRuntime[motor_index];
    uint8_t hall_code;

    if ((HALL_FILTER_SAMPLE_PERIOD_MS != 0U) &&
        ((now - runtime->last_sample_tick) < HALL_FILTER_SAMPLE_PERIOD_MS)) {
        return;
    }
    runtime->last_sample_tick = now;

    hall_code = HallStateFilter_ReadHallCode(motor_index);

    if (hall_code != runtime->candidate_hall_code) {
        runtime->candidate_hall_code = hall_code;
        runtime->candidate_stable_count = 1U;
        return;
    }

    if (runtime->candidate_stable_count < 0xFFU) {
        ++runtime->candidate_stable_count;
    }

    if (runtime->candidate_stable_count < HALL_FILTER_STABLE_COUNT) {
        return;
    }

    HallStateFilter_TryAcceptState(motor_index, hall_code);
}

void HallStateFilter_Init(void)
{
    uint32_t i;
    const uint32_t now = HAL_GetTick();

    for (i = 0U; i < HALL_STATE_FILTER_MOTOR_COUNT; ++i) {
        s_hallFilterRuntime[i].has_latest_state = 0U;
        s_hallFilterRuntime[i].candidate_hall_code = HALL_FILTER_INVALID_CODE;
        s_hallFilterRuntime[i].candidate_stable_count = 0U;
        s_hallFilterRuntime[i].last_sample_tick = now;
        s_hallFilterRuntime[i].last_accept_tick = 0U;
        s_hallFilterRuntime[i].transition_sequence = 0U;
    }
    MyPrint_Print("Hall state filter ready: M1=PC7/PC1/PC9 M2=PC2/PC3/PC8\r\n");
}

void HallStateFilter_Process(void)
{
    const uint32_t now = HAL_GetTick();

    HallStateFilter_ProcessMotor(HALL_STATE_FILTER_MOTOR_1, now);
    HallStateFilter_ProcessMotor(HALL_STATE_FILTER_MOTOR_2, now);
}

void HallStateFilter_OnExti(uint16_t gpio_pin)
{
    switch (gpio_pin) {
        case GPIO_PIN_1:
        case GPIO_PIN_7:
        case GPIO_PIN_9:
            HallStateFilter_TryAcceptState(HALL_STATE_FILTER_MOTOR_1,
                                           HallStateFilter_ReadHallCode(HALL_STATE_FILTER_MOTOR_1));
            break;
        case GPIO_PIN_2:
        case GPIO_PIN_3:
        case GPIO_PIN_8:
            HallStateFilter_TryAcceptState(HALL_STATE_FILTER_MOTOR_2,
                                           HallStateFilter_ReadHallCode(HALL_STATE_FILTER_MOTOR_2));
            break;
        default:
            break;
    }
}

uint8_t HallStateFilter_GetLatestState(HallStateFilter_MotorIndex motor_index,
                                       HallStateFilter_State *state)
{
    uint8_t has_state;

    if ((motor_index >= HALL_STATE_FILTER_MOTOR_COUNT) ||
        (state == NULL)) {
        return 0U;
    }

    __disable_irq();
    has_state = s_hallFilterRuntime[motor_index].has_latest_state;
    if (has_state != 0U) {
        *state = s_hallFilterRuntime[motor_index].latest_state;
    }
    __enable_irq();

    return has_state;
}

uint8_t HallStateFilter_GetCurrentState(HallStateFilter_MotorIndex motor_index,
                                        HallStateFilter_State *state)
{
    uint8_t hall_code;

    if ((motor_index >= HALL_STATE_FILTER_MOTOR_COUNT) ||
        (state == NULL)) {
        return 0U;
    }

    hall_code = HallStateFilter_ReadHallCode(motor_index);
    if (HallStateFilter_FillState(hall_code, state) == 0U) {
        return 0U;
    }

    state->transition_cycles = HallStateFilter_GetCycleCounter();
    state->transition_tick_ms = HAL_GetTick();
    state->transition_sequence = 0U;

    return 1U;
}
