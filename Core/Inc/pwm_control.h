/*
 * pwm_control.h
 *
 *  Created on: Mar 2, 2026
 */

#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include "tim.h"

/**
 * @brief Initialiserer PWM-parametre (dead-time, 0% duty).
 */
void PWM_Init(void);

/**
 * @brief Starter PWM på de konfigurerte kanalene.
 */
void PWM_Start(void);

/**
 * @brief Starter PWM for motor 1 (TIM1) på CH1-3 og CH1N-3N.
 */
void PWM_StartMotor1(void);

/**
 * @brief Starter PWM for motor 2 (TIM8) på CH1-3 og CH1N-3N.
 */
void PWM_StartMotor2(void);

/**
 * @brief Setter duty cycle i prosent (0-100) for en spesifikk kanal.
 * @param duty Prosentverdi (0.0 til 100.0)
 */
void PWM_Set_DutyCycle(uint32_t channel, float duty);

/**
 * @brief Setter duty cycle i prosent (0-100) for alle tre faser (TIM1).
 */
void PWM_SetMotor1Duty(float duty);

/**
 * @brief Setter duty cycle i prosent (0-100) for alle tre faser (TIM8).
 */
void PWM_SetMotor2Duty(float duty);

/**
 * @brief Kjører en maskinvaretest av gatedrivere (20kHz, 50% duty, 120 graders faseforskyvning).
 *        OBS: Dette er kun for maskinvareverifikasjon!
 */
void PWM_HardwareTest_3Phase(void);

#endif /* INC_PWM_CONTROL_H_ */
