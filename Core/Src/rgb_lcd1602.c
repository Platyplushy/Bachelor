#include "rgb_lcd1602.h"

#include "soft_i2c.h"

#define RGB_LCD1602_LCD_ADDRESS 0x3EU
#define RGB_LCD1602_RGB_ADDRESS 0x2DU

#define RGB_LCD1602_CMD_PREFIX  0x80U
#define RGB_LCD1602_DATA_PREFIX 0x40U

#define RGB_LCD1602_LCD_CLEARDISPLAY 0x01U
#define RGB_LCD1602_LCD_RETURNHOME   0x02U
#define RGB_LCD1602_LCD_ENTRYMODESET 0x04U
#define RGB_LCD1602_LCD_DISPLAYCONTROL 0x08U
#define RGB_LCD1602_LCD_FUNCTIONSET  0x20U
#define RGB_LCD1602_LCD_SETDDRAMADDR 0x80U

#define RGB_LCD1602_LCD_ENTRYLEFT 0x02U
#define RGB_LCD1602_LCD_ENTRYSHIFTDECREMENT 0x00U
#define RGB_LCD1602_LCD_DISPLAYON 0x04U
#define RGB_LCD1602_LCD_CURSOROFF 0x00U
#define RGB_LCD1602_LCD_BLINKOFF  0x00U
#define RGB_LCD1602_LCD_4BITMODE  0x00U
#define RGB_LCD1602_LCD_2LINE     0x08U
#define RGB_LCD1602_LCD_5x8DOTS   0x00U

#define RGB_LCD1602_REG_RED   0x01U
#define RGB_LCD1602_REG_GREEN 0x02U
#define RGB_LCD1602_REG_BLUE  0x03U

static uint8_t RgbLcd1602_WriteLcd(uint8_t prefix, uint8_t value)
{
    const uint8_t buffer[2] = {prefix, value};

    return SoftI2C_Write(RGB_LCD1602_LCD_ADDRESS, buffer, 2U);
}

static uint8_t RgbLcd1602_WriteRgbReg(uint8_t reg, uint8_t value)
{
    const uint8_t buffer[2] = {reg, value};

    return SoftI2C_Write(RGB_LCD1602_RGB_ADDRESS, buffer, 2U);
}

void RgbLcd1602_Init(void)
{
    SoftI2C_Init();

    HAL_Delay(50U);

    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX,
                              RGB_LCD1602_LCD_FUNCTIONSET | RGB_LCD1602_LCD_4BITMODE |
                              RGB_LCD1602_LCD_2LINE | RGB_LCD1602_LCD_5x8DOTS);
    HAL_Delay(5U);
    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX,
                              RGB_LCD1602_LCD_FUNCTIONSET | RGB_LCD1602_LCD_4BITMODE |
                              RGB_LCD1602_LCD_2LINE | RGB_LCD1602_LCD_5x8DOTS);
    HAL_Delay(5U);
    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX,
                              RGB_LCD1602_LCD_FUNCTIONSET | RGB_LCD1602_LCD_4BITMODE |
                              RGB_LCD1602_LCD_2LINE | RGB_LCD1602_LCD_5x8DOTS);

    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX,
                              RGB_LCD1602_LCD_DISPLAYCONTROL |
                              RGB_LCD1602_LCD_DISPLAYON |
                              RGB_LCD1602_LCD_CURSOROFF |
                              RGB_LCD1602_LCD_BLINKOFF);

    RgbLcd1602_Clear();

    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX,
                              RGB_LCD1602_LCD_ENTRYMODESET |
                              RGB_LCD1602_LCD_ENTRYLEFT |
                              RGB_LCD1602_LCD_ENTRYSHIFTDECREMENT);

    RgbLcd1602_SetRgb(255U, 255U, 255U);
}

void RgbLcd1602_Clear(void)
{
    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX, RGB_LCD1602_LCD_CLEARDISPLAY);
    HAL_Delay(2U);
}

void RgbLcd1602_SetCursor(uint8_t col, uint8_t row)
{
    uint8_t address = (row == 0U) ? (uint8_t)(0x80U | col) : (uint8_t)(0xC0U | col);

    (void)RgbLcd1602_WriteLcd(RGB_LCD1602_CMD_PREFIX, address);
}

void RgbLcd1602_Print(const char *text)
{
    if (text == NULL) {
        return;
    }

    while (*text != '\0') {
        (void)RgbLcd1602_WriteLcd(RGB_LCD1602_DATA_PREFIX, (uint8_t)*text);
        ++text;
    }
}

void RgbLcd1602_SetRgb(uint8_t red, uint8_t green, uint8_t blue)
{
    (void)RgbLcd1602_WriteRgbReg(RGB_LCD1602_REG_RED, red);
    (void)RgbLcd1602_WriteRgbReg(RGB_LCD1602_REG_GREEN, green);
    (void)RgbLcd1602_WriteRgbReg(RGB_LCD1602_REG_BLUE, blue);
}
