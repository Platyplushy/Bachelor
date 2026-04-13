#ifndef INC_SOFT_I2C_H_
#define INC_SOFT_I2C_H_

#include "main.h"

void SoftI2C_Init(void);
uint8_t SoftI2C_Write(uint8_t address_7bit, const uint8_t *data, uint8_t length);

#endif /* INC_SOFT_I2C_H_ */
