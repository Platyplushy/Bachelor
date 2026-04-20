/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hall_debug.h"
#include "hall_probe.h"
#include "hall_state_filter.h"
#include "joystick.h"
#include "motor_commutation.h"
#include "myapp.h"
#include "myprint.h"
#include "pwm_control.h"
#include "rgb_lcd1602.h"
#include "temperature_sensor.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MOTOR_CONTROL_ENABLE_RAW_HALL_DEBUG 0U
#define LCD_SPEED_UPDATE_INTERVAL_MS 500U
#define TEMPERATURE_PRINT_INTERVAL_MS 500U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void FormatVehicleStatusLine(char *line, int speed_x10, uint8_t motors_enabled);
static void FormatTemperatureLine(char *line, float motor1_temp_c, float motor2_temp_c);
static void FormatTemperatureField(char *field, float temperature_c);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  App_StartTasks();

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  uint32_t last_status_tick = osKernelGetTickCount();
  uint32_t last_lcd_speed_tick = osKernelGetTickCount();
  uint32_t last_temperature_print_tick = osKernelGetTickCount();
  char lcd_line_1[17];
  char lcd_line_2[17];

  /* Infinite loop */
  for(;;)
  {
    MyPrint_ProcessTerminal();
    Joystick_Process();
    TemperatureSensor_Process();

#if MOTOR_CONTROL_ENABLE_RAW_HALL_DEBUG
    HallDebug_Process();
    HallProbe_Process();
#endif

    if ((osKernelGetTickCount() - last_status_tick) >= 5000U)
    {
      last_status_tick = osKernelGetTickCount();
      MyPrint_Print("UART alive: RX=PA3 TX=PA2\r\n");
    }

    if ((osKernelGetTickCount() - last_temperature_print_tick) >= TEMPERATURE_PRINT_INTERVAL_MS)
    {
      const uint16_t motor1_raw_adc = TemperatureSensor_GetMotor1RawAdc();
      const uint16_t motor2_raw_adc = TemperatureSensor_GetMotor2RawAdc();
      const float motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();
      const float motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();
      const int motor1_valid = (motor1_temp_c > -200.0f) ? 1 : 0;
      const int motor2_valid = (motor2_temp_c > -200.0f) ? 1 : 0;
      const int motor1_temp_x10 = motor1_valid ? (int)((motor1_temp_c * 10.0f) + 0.5f) : 0;
      const int motor2_temp_x10 = motor2_valid ? (int)((motor2_temp_c * 10.0f) + 0.5f) : 0;

      last_temperature_print_tick = osKernelGetTickCount();
      if ((motor1_valid != 0) && (motor2_valid != 0)) {
        MyPrint_Printf("TEMP M1: adc=%u temp=%u.%u C | M2: adc=%u temp=%u.%u C\r\n",
                       (unsigned)motor1_raw_adc,
                       (unsigned)(motor1_temp_x10 / 10),
                       (unsigned)(motor1_temp_x10 % 10),
                       (unsigned)motor2_raw_adc,
                       (unsigned)(motor2_temp_x10 / 10),
                       (unsigned)(motor2_temp_x10 % 10));
      } else {
        MyPrint_Printf("TEMP M1: adc=%u temp=invalid | M2: adc=%u temp=invalid\r\n",
                       (unsigned)motor1_raw_adc,
                       (unsigned)motor2_raw_adc);
      }
    }

    if ((osKernelGetTickCount() - last_lcd_speed_tick) >= LCD_SPEED_UPDATE_INTERVAL_MS)
    {
      int vehicle_speed_x10;
      uint8_t motors_enabled;
      float motor1_temp_c;
      float motor2_temp_c;

      last_lcd_speed_tick = osKernelGetTickCount();
      vehicle_speed_x10 = (int)(MotorCommutation_GetVehicleSpeedKmh() * 10.0f);
      if (vehicle_speed_x10 < 0) {
        vehicle_speed_x10 = -vehicle_speed_x10;
      }
      if (vehicle_speed_x10 > 999) {
        vehicle_speed_x10 = 999;
      }
      motors_enabled = MotorCommutation_IsEnabled();
      motor1_temp_c = TemperatureSensor_GetMotor1TemperatureC();
      motor2_temp_c = TemperatureSensor_GetMotor2TemperatureC();

      FormatVehicleStatusLine(lcd_line_1, vehicle_speed_x10, motors_enabled);
      FormatTemperatureLine(lcd_line_2, motor1_temp_c, motor2_temp_c);

      RgbLcd1602_SetCursor(0U, 0U);
      RgbLcd1602_Print(lcd_line_1);

      RgbLcd1602_SetCursor(0U, 1U);
      RgbLcd1602_Print(lcd_line_2);
    }

    osThreadYield();
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void FormatVehicleStatusLine(char *line, int speed_x10, uint8_t motors_enabled)
{
  const int whole = speed_x10 / 10;
  const int decimal = speed_x10 % 10;
  const char *status = (motors_enabled != 0U) ? "ON " : "OFF";

  line[0] = (char)('0' + ((whole / 10) % 10));
  line[1] = (char)('0' + (whole % 10));
  line[2] = '.';
  line[3] = (char)('0' + decimal);
  line[4] = 'k';
  line[5] = 'm';
  line[6] = '/';
  line[7] = 't';
  line[8] = ' ';
  line[9] = status[0];
  line[10] = status[1];
  line[11] = status[2];
  line[12] = ' ';
  line[13] = ' ';
  line[14] = ' ';
  line[15] = ' ';
  line[16] = '\0';
}

static void FormatTemperatureLine(char *line, float motor1_temp_c, float motor2_temp_c)
{
  line[0] = 'T';
  line[1] = '1';
  line[2] = ' ';
  FormatTemperatureField(&line[3], motor1_temp_c);
  line[7] = ' ';
  line[8] = 'T';
  line[9] = '2';
  line[10] = ' ';
  FormatTemperatureField(&line[11], motor2_temp_c);
  line[15] = ' ';
  line[16] = '\0';
}

static void FormatTemperatureField(char *field, float temperature_c)
{
  int temperature_x10;

  if ((temperature_c < 0.0f) || (temperature_c > 99.9f)) {
    field[0] = '-';
    field[1] = '-';
    field[2] = '.';
    field[3] = '-';
    return;
  }

  temperature_x10 = (int)((temperature_c * 10.0f) + 0.5f);
  if (temperature_x10 > 999) {
    temperature_x10 = 999;
  }

  field[0] = (char)('0' + ((temperature_x10 / 100) % 10));
  field[1] = (char)('0' + ((temperature_x10 / 10) % 10));
  field[2] = '.';
  field[3] = (char)('0' + (temperature_x10 % 10));
}

/* USER CODE END Application */

