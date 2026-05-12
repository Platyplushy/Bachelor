#include "motor_temperature_safety.h"

#include "motor_commutation.h"
#include "myprint.h"

#define MOTOR_TEMPERATURE_LIMIT_C 80.0f
#define MOTOR_TEMPERATURE_CLEAR_C 70.0f

static uint8_t s_motorTemperatureFaults = 0U;

static uint8_t MotorTemperatureSafety_IsTemperatureValid(float temperature_c)
{
    return (temperature_c > -200.0f) ? 1U : 0U;
}

void MotorTemperatureSafety_Process(float motor1_temp_c, float motor2_temp_c)
{
    uint8_t new_faults = s_motorTemperatureFaults;

    if (MotorTemperatureSafety_IsTemperatureValid(motor1_temp_c) != 0U) {
        if (motor1_temp_c >= MOTOR_TEMPERATURE_LIMIT_C) {
            new_faults |= MOTOR_TEMPERATURE_FAULT_M1;
        } else if (motor1_temp_c <= MOTOR_TEMPERATURE_CLEAR_C) {
            new_faults &= (uint8_t)~MOTOR_TEMPERATURE_FAULT_M1;
        }
    }

    if (MotorTemperatureSafety_IsTemperatureValid(motor2_temp_c) != 0U) {
        if (motor2_temp_c >= MOTOR_TEMPERATURE_LIMIT_C) {
            new_faults |= MOTOR_TEMPERATURE_FAULT_M2;
        } else if (motor2_temp_c <= MOTOR_TEMPERATURE_CLEAR_C) {
            new_faults &= (uint8_t)~MOTOR_TEMPERATURE_FAULT_M2;
        }
    }

    if (new_faults != 0U) {
        if (s_motorTemperatureFaults == 0U) {
            MyPrint_Print("Motor stop: overtemperature fault\r\n");
        }
        MotorCommutation_ForceStop();
    }

    s_motorTemperatureFaults = new_faults;
}

uint8_t MotorTemperatureSafety_GetFaults(void)
{
    return s_motorTemperatureFaults;
}
