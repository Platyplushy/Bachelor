#ifndef INC_JOYSTICK_H_
#define INC_JOYSTICK_H_

#include <stdint.h>

typedef struct {
    uint8_t enabled;
    uint8_t forward;
    float duty_percent;
} Joystick_MotorCommand;

void Joystick_Init(void);
void Joystick_Process(void);
uint16_t Joystick_GetRawX(void);
uint16_t Joystick_GetRawY(void);
int16_t Joystick_GetSpeedCommandX(void);
int16_t Joystick_GetDriveCommand(void);
int16_t Joystick_GetTurnCommand(void);
int16_t Joystick_GetLeftCommand(void);
int16_t Joystick_GetRightCommand(void);
uint8_t Joystick_IsButtonActive(void);
Joystick_MotorCommand Joystick_GetLeftMotorCommand(void);
Joystick_MotorCommand Joystick_GetRightMotorCommand(void);

#endif /* INC_JOYSTICK_H_ */
