#ifndef INC_MYPRINT_H_
#define INC_MYPRINT_H_

#include "main.h"

void MyPrint_Init(void);
void MyPrint_Print(const char *text);
void MyPrint_Printf(const char *format, ...);
HAL_StatusTypeDef MyPrint_ReadByte(uint8_t *byte);
void MyPrint_ProcessTerminal(void);

#endif /* INC_MYPRINT_H_ */
