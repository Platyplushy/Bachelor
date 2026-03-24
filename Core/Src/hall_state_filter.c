#include "hall_state_filter.h"

#include "myprint.h"

#define HALL_FILTER_SAMPLE_PERIOD_MS   1U
#define HALL_FILTER_STABLE_COUNT       2U
#define HALL_FILTER_INVALID_CODE       0xFFU
#define HALL_FILTER_ENABLE_RUNTIME_PRINTS 1U

typedef struct {
    uint8_t hall_code;
    uint8_t sector;
} HallCodeSectorMap;

static const HallCodeSectorMap kHallSequence[] = {
    {0x1U, 1U}, /* UVW = 001 */
    {0x5U, 2U}, /* UVW = 101 */
    {0x4U, 3U}, /* UVW = 100 */
    {0x6U, 4U}, /* UVW = 110 */
    {0x2U, 5U}, /* UVW = 010 */
    {0x3U, 6U}  /* UVW = 011 */
};

static HallStateFilter_State s_latestState = {0U};
static uint8_t s_hasLatestState = 0U;
static uint8_t s_candidateHallCode = HALL_FILTER_INVALID_CODE;
static uint8_t s_candidateStableCount = 0U;
static uint32_t s_lastSampleTick = 0U;

static uint8_t HallStateFilter_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static uint8_t HallStateFilter_ReadHallCode(void)
{
    uint8_t hall_u = HallStateFilter_ReadPin(GPIOC, GPIO_PIN_7);
    uint8_t hall_v = HallStateFilter_ReadPin(GPIOC, GPIO_PIN_1);
    uint8_t hall_w = HallStateFilter_ReadPin(GPIOC, GPIO_PIN_9);

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
    uint8_t sequence_size = (uint8_t)(sizeof(kHallSequence) / sizeof(kHallSequence[0]));

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

static void HallStateFilter_AcceptState(uint8_t hall_code)
{
    HallStateFilter_State state;

    if (HallStateFilter_FillState(hall_code, &state) == 0U) {
        return;
    }

    s_latestState = state;
    s_hasLatestState = 1U;

#if HALL_FILTER_ENABLE_RUNTIME_PRINTS
    MyPrint_Printf("HALL FILTER: sector=%u UVW=%u%u%u code=0x%X\r\n",
                   state.sector,
                   state.hall_u,
                   state.hall_v,
                   state.hall_w,
                   state.hall_code);
#endif
}

void HallStateFilter_Init(void)
{
    s_hasLatestState = 0U;
    s_candidateHallCode = HALL_FILTER_INVALID_CODE;
    s_candidateStableCount = 0U;
    s_lastSampleTick = HAL_GetTick();
    MyPrint_Print("Hall state filter ready: mapping A=W, B=V, C=U\r\n");
}

void HallStateFilter_Process(void)
{
    uint32_t now;
    uint8_t hall_code;

    now = HAL_GetTick();
    if ((now - s_lastSampleTick) < HALL_FILTER_SAMPLE_PERIOD_MS) {
        return;
    }
    s_lastSampleTick = now;

    hall_code = HallStateFilter_ReadHallCode();

    if (HallStateFilter_FindSequenceIndex(hall_code) == HALL_FILTER_INVALID_CODE) {
        s_candidateHallCode = HALL_FILTER_INVALID_CODE;
        s_candidateStableCount = 0U;
        return;
    }

    if (hall_code != s_candidateHallCode) {
        s_candidateHallCode = hall_code;
        s_candidateStableCount = 1U;
        return;
    }

    if (s_candidateStableCount < 0xFFU) {
        ++s_candidateStableCount;
    }

    if (s_candidateStableCount < HALL_FILTER_STABLE_COUNT) {
        return;
    }

    if ((s_hasLatestState != 0U) && (hall_code == s_latestState.hall_code)) {
        return;
    }

    if ((s_hasLatestState != 0U) && (HallStateFilter_IsAdjacent(s_latestState.hall_code, hall_code) == 0U)) {
        return;
    }

    HallStateFilter_AcceptState(hall_code);
}

uint8_t HallStateFilter_GetLatestState(HallStateFilter_State *state)
{
    if ((state == NULL) || (s_hasLatestState == 0U)) {
        return 0U;
    }

    *state = s_latestState;
    return 1U;
}
