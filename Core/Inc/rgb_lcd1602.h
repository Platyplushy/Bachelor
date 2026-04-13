#ifndef INC_RGB_LCD1602_H_
#define INC_RGB_LCD1602_H_

#include <stdint.h>

void RgbLcd1602_Init(void);
void RgbLcd1602_Clear(void);
void RgbLcd1602_SetCursor(uint8_t col, uint8_t row);
void RgbLcd1602_Print(const char *text);
void RgbLcd1602_SetRgb(uint8_t red, uint8_t green, uint8_t blue);

#endif /* INC_RGB_LCD1602_H_ */
