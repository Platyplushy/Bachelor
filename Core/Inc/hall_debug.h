#ifndef INC_HALL_DEBUG_H_
#define INC_HALL_DEBUG_H_

#include "main.h"

void HallDebug_Init(void);
void HallDebug_Process(void);
void HallDebug_OnExti(uint16_t gpio_pin);

#endif /* INC_HALL_DEBUG_H_ */
