#ifndef INC_MOTOR_COMMUTATION_H_
#define INC_MOTOR_COMMUTATION_H_

#include "main.h"

void MotorCommutation_Init(void);
void MotorCommutation_Process(void);
void MotorCommutation_SetDuty(float duty_percent);
uint8_t MotorCommutation_UsesHallInputs(void);

#endif /* INC_MOTOR_COMMUTATION_H_ */
