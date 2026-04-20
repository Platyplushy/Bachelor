#ifndef INC_TEMPERATURE_SENSOR_H_
#define INC_TEMPERATURE_SENSOR_H_

#include "main.h"

void TemperatureSensor_Init(void);
void TemperatureSensor_Process(void);
uint16_t TemperatureSensor_GetMotor1RawAdc(void);
uint16_t TemperatureSensor_GetMotor2RawAdc(void);
float TemperatureSensor_GetMotor1TemperatureC(void);
float TemperatureSensor_GetMotor2TemperatureC(void);

#endif /* INC_TEMPERATURE_SENSOR_H_ */
