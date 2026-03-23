/*
 * myapp.h
 *
 *  Created on: Mar 11, 2026
 *      Author: Gemini CLI
 */

#ifndef INC_MYAPP_H_
#define INC_MYAPP_H_

#include "main.h"

/**
 * @brief Initialiserer applikasjonen.
 *        Kalles fra main.c etter at periferiutstyr er konfigurert.
 */
void App_Init(void);
void App_StartTasks(void);

/**
 * @brief Applikasjonens hovedløkke.
 *        Kalles fra main.c sin while(1)-løkke.
 */
void App_Run(void);

#endif /* INC_MYAPP_H_ */
