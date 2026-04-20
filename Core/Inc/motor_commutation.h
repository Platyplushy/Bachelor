#ifndef INC_MOTOR_COMMUTATION_H_
#define INC_MOTOR_COMMUTATION_H_

#include "hall_state_filter.h"
#include "main.h"

void MotorCommutation_Init(void);
void MotorCommutation_Process(void);
void MotorCommutation_SetDuty(float duty_percent);
void MotorCommutation_SetWheelCommands(int16_t motor1_percent, int16_t motor2_percent);
void MotorCommutation_ToggleEnabled(void);
void MotorCommutation_OnHallStateAccepted(HallStateFilter_MotorIndex hall_index,
                                          const HallStateFilter_State *state);
float MotorCommutation_GetMotor1SpeedKmh(void);
float MotorCommutation_GetMotor2SpeedKmh(void);
float MotorCommutation_GetVehicleSpeedKmh(void);
uint8_t MotorCommutation_IsEnabled(void);
uint8_t MotorCommutation_UsesHallInputs(void);

#endif /* INC_MOTOR_COMMUTATION_H_ */
