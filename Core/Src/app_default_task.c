#include "app_default_task.h"

#include "cmsis_os.h"
#include "hall_debug.h"
#include "hall_probe.h"
#include "joystick.h"
#include "lcd_status.h"
#include "motor_commutation.h"
#include "motor_temperature_safety.h"
#include "myprint.h"
#include "temperature_sensor.h"

#define APP_DEFAULT_TASK_ENABLE_RAW_HALL_DEBUG 0U
#define APP_DEFAULT_TASK_ENABLE_TEMPERATURE_PRINTS 0U
#define APP_DEFAULT_TASK_LCD_UPDATE_INTERVAL_MS 500U
#define APP_DEFAULT_TASK_TEMPERATURE_PRINT_INTERVAL_MS 500U

#if APP_DEFAULT_TASK_ENABLE_TEMPERATURE_PRINTS
static void AppDefaultTask_PrintTemperatureStatus(float motor1_temp_c, float motor2_temp_c)
{
    const uint16_t motor1_raw_adc = TemperatureSensor_GetMotor1RawAdc();
    const uint16_t motor2_raw_adc = TemperatureSensor_GetMotor2RawAdc();
    const int motor1_valid = (motor1_temp_c > -200.0f) ? 1 : 0;
    const int motor2_valid = (motor2_temp_c > -200.0f) ? 1 : 0;
    const int motor1_temp_x10 = motor1_valid ? (int)((motor1_temp_c * 10.0f) + 0.5f) : 0;
    const int motor2_temp_x10 = motor2_valid ? (int)((motor2_temp_c * 10.0f) + 0.5f) : 0;

    if ((motor1_valid != 0) && (motor2_valid != 0)) {
        MyPrint_Printf("TEMP M1: adc=%u temp=%u.%u C | M2: adc=%u temp=%u.%u C\r\n",
                       (unsigned)motor1_raw_adc,
                       (unsigned)(motor1_temp_x10 / 10),
                       (unsigned)(motor1_temp_x10 % 10),
                       (unsigned)motor2_raw_adc,
                       (unsigned)(motor2_temp_x10 / 10),
                       (unsigned)(motor2_temp_x10 % 10));
    } else {
        MyPrint_Printf("TEMP M1: adc=%u temp=invalid | M2: adc=%u temp=invalid\r\n",
                       (unsigned)motor1_raw_adc,
                       (unsigned)motor2_raw_adc);
    }
}
#endif

void AppDefaultTask_Run(void *argument)
{
    uint32_t last_status_tick = osKernelGetTickCount();
    uint32_t last_lcd_speed_tick = osKernelGetTickCount();
#if APP_DEFAULT_TASK_ENABLE_TEMPERATURE_PRINTS
    uint32_t last_temperature_print_tick = osKernelGetTickCount();
#endif

    (void)argument;

    for (;;) {
        MyPrint_ProcessTerminal();
        Joystick_Process();
        TemperatureSensor_Process();

#if APP_DEFAULT_TASK_ENABLE_RAW_HALL_DEBUG
        HallDebug_Process();
        HallProbe_Process();
#endif

        if ((osKernelGetTickCount() - last_status_tick) >= 5000U) {
            last_status_tick = osKernelGetTickCount();
            MyPrint_Print("UART alive: RX=PA3 TX=PA2\r\n");
        }

#if APP_DEFAULT_TASK_ENABLE_TEMPERATURE_PRINTS
        if ((osKernelGetTickCount() - last_temperature_print_tick) >=
            APP_DEFAULT_TASK_TEMPERATURE_PRINT_INTERVAL_MS) {
            const float motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();
            const float motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();

            last_temperature_print_tick = osKernelGetTickCount();
            AppDefaultTask_PrintTemperatureStatus(motor1_temp_c, motor2_temp_c);
        }
#endif

        if ((osKernelGetTickCount() - last_lcd_speed_tick) >=
            APP_DEFAULT_TASK_LCD_UPDATE_INTERVAL_MS) {
            const float motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();
            const float motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();
            const int motor2_speed_x10 = (int)(MotorCommutation_GetMotor2SpeedKmh() * 10.0f);

            last_lcd_speed_tick = osKernelGetTickCount();
            MotorTemperatureSafety_Process(motor1_temp_c, motor2_temp_c);
            LcdStatus_Update(motor2_speed_x10,
                             MotorCommutation_IsEnabled(),
                             motor1_temp_c,
                             motor2_temp_c,
                             MotorTemperatureSafety_GetFaults());
        }

        osThreadYield();
    }
}
