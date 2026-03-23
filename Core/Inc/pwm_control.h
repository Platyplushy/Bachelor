/*
 * pwm_control.h
 *
 *  Created on: Mar 2, 2026
 */

#ifndef INC_PWM_CONTROL_H_
#define INC_PWM_CONTROL_H_

#include <stdint.h>

#include "tim.h"

typedef enum {
    PWM_PHASE_STATE_FLOAT = 0,
    PWM_PHASE_STATE_HIGH,
    PWM_PHASE_STATE_LOW
} PWM_PhaseState;

/**
 * @brief Initialiserer PWM-parametre (dead-time, 0% duty).
 */
void PWM_Init(void);

/**
 * @brief Konfigurerer TIM1 for 3-fase komplementaer PWM.
 */
void PWM_TIM1_Configure3PhaseComplementary(void);

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
 * @brief Setter separate duty cycles i prosent (0-100) for TIM1 fase A/B/C.
 */
void PWM_SetMotor1PhaseDuties(float duty_a, float duty_b, float duty_c);

/**
 * @brief Setter duty cycle i prosent (0-100) for alle tre faser (TIM8).
 */
void PWM_SetMotor2Duty(float duty);
void PWM_TIM1_SetPhaseState(uint32_t channel, PWM_PhaseState state, float duty);
void PWM_TIM1_SetPhaseStates(PWM_PhaseState state_u,
                             PWM_PhaseState state_v,
                             PWM_PhaseState state_w,
                             float duty);

/**
 * @brief Kjører en maskinvaretest av gatedrivere (20kHz, 50% duty, 120 graders faseforskyvning).
 *        OBS: Dette er kun for maskinvareverifikasjon!
 */
void PWM_HardwareTest_3Phase(void);
void PWM_HardwareTest_SetOutputFrequency(uint32_t frequency_hz);
void PWM_HardwareTest_SoftStart(uint32_t start_frequency_hz,
                                uint32_t target_frequency_hz,
                                uint32_t step_delay_ms);

/**
 * @brief Håndterer TIM1 update interrupt for 3-fase high-side maskinvaretest.
 */
void PWM_TIM1_UpdateIRQHandler(void);

#endif /* INC_PWM_CONTROL_H_ */
