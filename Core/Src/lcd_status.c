#include "lcd_status.h"

#include "motor_temperature_safety.h"
#include "rgb_lcd1602.h"

static void LcdStatus_FormatVehicleLine(char *line, int speed_x10, uint8_t motors_enabled);
static void LcdStatus_FormatTemperatureLine(char *line, float motor1_temp_c, float motor2_temp_c);
static void LcdStatus_FormatOvertemperatureLine(char *line, uint8_t faults);
static void LcdStatus_FormatTemperatureField(char *field, float temperature_c);

void LcdStatus_Update(int speed_x10,
                      uint8_t motors_enabled,
                      float motor1_temp_c,
                      float motor2_temp_c,
                      uint8_t temperature_faults)
{
    char lcd_line_1[17];
    char lcd_line_2[17];

    if (speed_x10 < 0) {
        speed_x10 = -speed_x10;
    }
    if (speed_x10 > 999) {
        speed_x10 = 999;
    }

    LcdStatus_FormatVehicleLine(lcd_line_1, speed_x10, motors_enabled);

    if (temperature_faults != 0U) {
        LcdStatus_FormatOvertemperatureLine(lcd_line_2, temperature_faults);
    } else {
        LcdStatus_FormatTemperatureLine(lcd_line_2, motor1_temp_c, motor2_temp_c);
    }

    RgbLcd1602_SetCursor(0U, 0U);
    RgbLcd1602_Print(lcd_line_1);

    RgbLcd1602_SetCursor(0U, 1U);
    RgbLcd1602_Print(lcd_line_2);
}

static void LcdStatus_FormatVehicleLine(char *line, int speed_x10, uint8_t motors_enabled)
{
    const int whole = speed_x10 / 10;
    const int decimal = speed_x10 % 10;
    uint32_t i;

    for (i = 0U; i < 16U; ++i) {
        line[i] = ' ';
    }

    line[0] = 'M';
    line[1] = '2';
    line[2] = ' ';
    line[3] = (char)('0' + ((whole / 10) % 10));
    line[4] = (char)('0' + (whole % 10));
    line[5] = '.';
    line[6] = (char)('0' + decimal);
    line[7] = 'k';
    line[8] = 'm';
    line[9] = '/';
    line[10] = 't';

    if (motors_enabled != 0U) {
        line[14] = 'O';
        line[15] = 'N';
    } else {
        line[13] = 'O';
        line[14] = 'F';
        line[15] = 'F';
    }

    line[16] = '\0';
}

static void LcdStatus_FormatTemperatureLine(char *line, float motor1_temp_c, float motor2_temp_c)
{
    line[0] = 'T';
    line[1] = '1';
    line[2] = ' ';
    LcdStatus_FormatTemperatureField(&line[3], motor1_temp_c);
    line[7] = ' ';
    line[8] = 'T';
    line[9] = '2';
    line[10] = ' ';
    LcdStatus_FormatTemperatureField(&line[11], motor2_temp_c);
    line[15] = ' ';
    line[16] = '\0';
}

static void LcdStatus_FormatOvertemperatureLine(char *line, uint8_t faults)
{
    uint32_t i;

    for (i = 0U; i < 16U; ++i) {
        line[i] = ' ';
    }

    line[0] = 'T';
    line[1] = 'E';
    line[2] = 'M';
    line[3] = 'P';
    line[4] = ' ';
    line[5] = 'F';
    line[6] = 'A';
    line[7] = 'U';
    line[8] = 'L';
    line[9] = 'T';
    line[10] = ' ';

    if ((faults & (MOTOR_TEMPERATURE_FAULT_M1 | MOTOR_TEMPERATURE_FAULT_M2)) ==
        (MOTOR_TEMPERATURE_FAULT_M1 | MOTOR_TEMPERATURE_FAULT_M2)) {
        line[11] = 'M';
        line[12] = '1';
        line[13] = '+';
        line[14] = 'M';
        line[15] = '2';
    } else if ((faults & MOTOR_TEMPERATURE_FAULT_M1) != 0U) {
        line[12] = 'M';
        line[13] = '1';
    } else if ((faults & MOTOR_TEMPERATURE_FAULT_M2) != 0U) {
        line[12] = 'M';
        line[13] = '2';
    }

    line[16] = '\0';
}

static void LcdStatus_FormatTemperatureField(char *field, float temperature_c)
{
    int temperature_x10;

    if ((temperature_c < 0.0f) || (temperature_c > 99.9f)) {
        field[0] = '-';
        field[1] = '-';
        field[2] = '.';
        field[3] = '-';
        return;
    }

    temperature_x10 = (int)((temperature_c * 10.0f) + 0.5f);
    if (temperature_x10 > 999) {
        temperature_x10 = 999;
    }

    field[0] = (char)('0' + ((temperature_x10 / 100) % 10));
    field[1] = (char)('0' + ((temperature_x10 / 10) % 10));
    field[2] = '.';
    field[3] = (char)('0' + (temperature_x10 % 10));
}
