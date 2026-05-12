#ifndef INC_MOTOR_TEMPERATURE_SAFETY_H_
#define INC_MOTOR_TEMPERATURE_SAFETY_H_

#include "main.h"

#define MOTOR_TEMPERATURE_FAULT_M1 (1U << 0)
#define MOTOR_TEMPERATURE_FAULT_M2 (1U << 1)

void MotorTemperatureSafety_Process(float motor1_temp_c, float motor2_temp_c);
uint8_t MotorTemperatureSafety_GetFaults(void);

#endif /* INC_MOTOR_TEMPERATURE_SAFETY_H_ */
