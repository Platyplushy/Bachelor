# STM32G431 Pin Configuration
This document defines all MCU pin assignments used in the project.
All firmware agents must follow this mapping when generating code.

| Signal  | Timer Function | Pin  | Port  |
| ------- | -------------- | ---- | ----- |
| PWM_HA1 | TIM1_CH1       | PA8  | GPIOA |
| PWM_HB1 | TIM1_CH2       | PA9  | GPIOA |
| PWM_HC1 | TIM1_CH3       | PA10 | GPIOA |
| PWM_LA1 | TIM1_CH1N      | PB13 | GPIOB |
| PWM_LB1 | TIM1_CH2N      | PB14 | GPIOB |
| PWM_LC1 | TIM1_CH3N      | PB15 | GPIOB |
| PWM_HA2 | TIM8_CH1       | PB6  | GPIOB |
| PWM_HB2 | TIM8_CH2       | PB8  | GPIOB |
| PWM_HC2 | TIM8_CH3       | PB9  | GPIOB |
| PWM_LA2 | TIM8_CH1N      | PC10 | GPIOC |
| PWM_LB2 | TIM8_CH2N      | PC11 | GPIOC |
| PWM_LC2 | TIM8_CH3N      | PC12 | GPIOC |

| Signal   | Pin | Port  |
| -------- | --- | ----- |
| HALL_1_U | PC7 | GPIOC |
| HALL_1_V | PC1 | GPIOC |
| HALL_1_W | PC9 | GPIOC |
| HALL_2_U | PC2 | GPIOC |
| HALL_2_V | PC3 | GPIOC |
| HALL_2_W | PC8 | GPIOC |

Hall-signaler skal leses som `UVW` i firmware. Dette matcher `hall_state_filter.c`, som bygger hall-koden som `(U << 2) | (V << 1) | W`.

| Signal     | ADC Function | Pin | Port  |
| ---------- | ------------ | --- | ----- |
| Joystick_0 | ADC1_IN1     | PA0 | GPIOA |
| Joystick_1 | ADC1_IN2     | PA1 | GPIOA |
| TEMP_1     | ADC1_IN11    | PA6 | GPIOA |
| TEMP_2     | ADC1_IN12    | PA7 | GPIOA |

| Signal    | Function  | Pin | Port  |
| --------- | --------- | --- | ----- |
| USART2_TX | USART2_TX | PA2 | GPIOA |
| USART2_RX | USART2_RX | PA3 | GPIOA |

| Signal      | Function           | Pin  | Port  |
| ----------- | ------------------ | ---- | ----- |
| MOTOR_START | Motor start/stop   | PB12 | GPIOB |
| LCD_SCL     | Software I2C SCL   | PB7  | GPIOB |
| LCD_SDA     | Software I2C SDA   | PB11 | GPIOB |
| BLINK_LED   | Blink task output  | PB10 | GPIOB |

| Signal | Pin  | Port  |
| ------ | ---- | ----- |
| SWDIO  | PA13 | GPIOA |
| SWCLK  | PA14 | GPIOA |
| NRST   | PG10 | GPIOG |
| BOOT0  | PB8  | GPIOB |
