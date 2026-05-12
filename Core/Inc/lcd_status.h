#ifndef INC_LCD_STATUS_H_
#define INC_LCD_STATUS_H_

#include "main.h"

void LcdStatus_Update(int speed_x10,
                      uint8_t motors_enabled,
                      float motor1_temp_c,
                      float motor2_temp_c,
                      uint8_t temperature_faults);

#endif /* INC_LCD_STATUS_H_ */
