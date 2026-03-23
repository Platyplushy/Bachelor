#ifndef INC_HALL_STATE_FILTER_H_
#define INC_HALL_STATE_FILTER_H_

#include "main.h"

typedef struct {
    uint8_t hall_u;
    uint8_t hall_v;
    uint8_t hall_w;
    uint8_t hall_code;
    uint8_t sector;
} HallStateFilter_State;

void HallStateFilter_Init(void);
void HallStateFilter_Process(void);
uint8_t HallStateFilter_GetLatestState(HallStateFilter_State *state);

#endif /* INC_HALL_STATE_FILTER_H_ */
