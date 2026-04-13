#include "joystick.h"

#include "adc.h"
#include "myprint.h"

#define JOYSTICK_X_CENTER          1810U
#define JOYSTICK_X_DEADBAND        60U
#define JOYSTICK_X_MIN_RAW         840U
#define JOYSTICK_X_MAX_RAW         2870U
#define JOYSTICK_Y_CENTER          1880U
#define JOYSTICK_Y_DEADBAND        80U
#define JOYSTICK_Y_MIN_RAW         900U
#define JOYSTICK_Y_MAX_RAW         2920U
#define JOYSTICK_BUTTON_THRESHOLD  3800U
#define JOYSTICK_OUTPUT_DEADBAND   4
#define JOYSTICK_DUTY_MIN_PERCENT  5.0f
#define JOYSTICK_DUTY_MAX_PERCENT  40.0f
#define JOYSTICK_PRINT_INTERVAL_MS 200U
#define JOYSTICK_MIN_CHANGE        4U
#define JOYSTICK_ENABLE_PRINTS     0U

static uint16_t s_joystickRawX = 0U;
static uint16_t s_joystickRawY = 0U;
static uint16_t s_joystickLastPrintedX = 0U;
static uint16_t s_joystickLastPrintedY = 0U;
static int16_t s_joystickSpeedCommandX = 0;
static int16_t s_joystickDriveCommand = 0;
static int16_t s_joystickTurnCommand = 0;
static int16_t s_joystickLatchedTurnCommand = 0;
static int16_t s_joystickLeftCommand = 0;
static int16_t s_joystickRightCommand = 0;
static Joystick_MotorCommand s_leftMotorCommand = {0U, 1U, 0.0f};
static Joystick_MotorCommand s_rightMotorCommand = {0U, 1U, 0.0f};
static uint8_t s_joystickButtonActive = 0U;
static uint8_t s_joystickHasPrint = 0U;
static uint32_t s_joystickLastPrintTick = 0U;

static int16_t Joystick_Clamp100(int32_t value)
{
    if (value > 100) {
        return 100;
    }
    if (value < -100) {
        return -100;
    }

    return (int16_t)value;
}

static int16_t Joystick_ComputeAxisCommand(uint16_t raw_value,
                                           uint16_t center,
                                           uint16_t deadband,
                                           uint16_t min_raw,
                                           uint16_t max_raw)
{
    int32_t delta;
    int32_t range;
    int32_t command;

    if ((raw_value >= (center - deadband)) &&
        (raw_value <= (center + deadband))) {
        return 0;
    }

    if (raw_value > center) {
        delta = (int32_t)raw_value - (int32_t)(center + deadband);
        range = (int32_t)max_raw - (int32_t)(center + deadband);
        if (range <= 0) {
            return 0;
        }

        command = (delta * 100) / range;
        return Joystick_Clamp100(command);
    }

    delta = (int32_t)(center - deadband) - (int32_t)raw_value;
    range = (int32_t)(center - deadband) - (int32_t)min_raw;
    if (range <= 0) {
        return 0;
    }

    command = (delta * 100) / range;
    return (int16_t)(-Joystick_Clamp100(command));
}

static void Joystick_UpdateMixedCommands(void)
{
    int32_t left;
    int32_t right;

    left = (int32_t)s_joystickDriveCommand + (int32_t)s_joystickTurnCommand;
    right = (int32_t)s_joystickDriveCommand - (int32_t)s_joystickTurnCommand;

    s_joystickLeftCommand = Joystick_Clamp100(left);
    s_joystickRightCommand = Joystick_Clamp100(right);

    if ((s_joystickLeftCommand >= -JOYSTICK_OUTPUT_DEADBAND) &&
        (s_joystickLeftCommand <= JOYSTICK_OUTPUT_DEADBAND)) {
        s_joystickLeftCommand = 0;
    }

    if ((s_joystickRightCommand >= -JOYSTICK_OUTPUT_DEADBAND) &&
        (s_joystickRightCommand <= JOYSTICK_OUTPUT_DEADBAND)) {
        s_joystickRightCommand = 0;
    }
}

static Joystick_MotorCommand Joystick_ComputeMotorCommand(int16_t command)
{
    Joystick_MotorCommand motor_command;
    int16_t magnitude;

    motor_command.enabled = 0U;
    motor_command.forward = 1U;
    motor_command.duty_percent = 0.0f;

    if (command == 0) {
        return motor_command;
    }

    motor_command.enabled = 1U;
    motor_command.forward = (command > 0) ? 1U : 0U;
    magnitude = (command > 0) ? command : (int16_t)(-command);

    motor_command.duty_percent = JOYSTICK_DUTY_MIN_PERCENT +
                                 (((float)(magnitude - 1)) * (JOYSTICK_DUTY_MAX_PERCENT - JOYSTICK_DUTY_MIN_PERCENT) / 99.0f);

    if (motor_command.duty_percent > JOYSTICK_DUTY_MAX_PERCENT) {
        motor_command.duty_percent = JOYSTICK_DUTY_MAX_PERCENT;
    }

    return motor_command;
}

void Joystick_Init(void)
{
    s_joystickRawX = 0U;
    s_joystickRawY = 0U;
    s_joystickLastPrintedX = 0U;
    s_joystickLastPrintedY = 0U;
    s_joystickSpeedCommandX = 0;
    s_joystickDriveCommand = 0;
    s_joystickTurnCommand = 0;
    s_joystickLatchedTurnCommand = 0;
    s_joystickLeftCommand = 0;
    s_joystickRightCommand = 0;
    s_leftMotorCommand.enabled = 0U;
    s_leftMotorCommand.forward = 1U;
    s_leftMotorCommand.duty_percent = 0.0f;
    s_rightMotorCommand.enabled = 0U;
    s_rightMotorCommand.forward = 1U;
    s_rightMotorCommand.duty_percent = 0.0f;
    s_joystickButtonActive = 0U;
    s_joystickHasPrint = 0U;
    s_joystickLastPrintTick = HAL_GetTick();
}

void Joystick_Process(void)
{
    uint32_t now;
    uint16_t delta_x;
    uint16_t delta_y;

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc1);
        return;
    }

    s_joystickRawX = (uint16_t)HAL_ADC_GetValue(&hadc1);
    s_joystickSpeedCommandX = Joystick_ComputeAxisCommand(s_joystickRawX,
                                                          JOYSTICK_X_CENTER,
                                                          JOYSTICK_X_DEADBAND,
                                                          JOYSTICK_X_MIN_RAW,
                                                          JOYSTICK_X_MAX_RAW);

    if (HAL_ADC_ConfigChannel(&hadc1, &(ADC_ChannelConfTypeDef){
            .Channel = ADC_CHANNEL_2,
            .Rank = ADC_REGULAR_RANK_1,
            .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
            .SingleDiff = ADC_SINGLE_ENDED,
            .OffsetNumber = ADC_OFFSET_NONE,
            .Offset = 0
        }) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc1);
        return;
    }

    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc1);
        return;
    }

    s_joystickRawY = (uint16_t)HAL_ADC_GetValue(&hadc1);
    (void)HAL_ADC_Stop(&hadc1);

    (void)HAL_ADC_ConfigChannel(&hadc1, &(ADC_ChannelConfTypeDef){
            .Channel = ADC_CHANNEL_1,
            .Rank = ADC_REGULAR_RANK_1,
            .SamplingTime = ADC_SAMPLETIME_2CYCLES_5,
            .SingleDiff = ADC_SINGLE_ENDED,
            .OffsetNumber = ADC_OFFSET_NONE,
            .Offset = 0
        });

    s_joystickButtonActive = (s_joystickRawX >= JOYSTICK_BUTTON_THRESHOLD) ? 1U : 0U;
    s_joystickDriveCommand = Joystick_ComputeAxisCommand(s_joystickRawY,
                                                         JOYSTICK_Y_CENTER,
                                                         JOYSTICK_Y_DEADBAND,
                                                         JOYSTICK_Y_MIN_RAW,
                                                         JOYSTICK_Y_MAX_RAW);

    if (s_joystickButtonActive == 0U) {
        s_joystickTurnCommand = s_joystickSpeedCommandX;
        s_joystickLatchedTurnCommand = s_joystickTurnCommand;
    } else {
        s_joystickTurnCommand = s_joystickLatchedTurnCommand;
    }

    Joystick_UpdateMixedCommands();
    s_leftMotorCommand = Joystick_ComputeMotorCommand(s_joystickLeftCommand);
    s_rightMotorCommand = Joystick_ComputeMotorCommand(s_joystickRightCommand);

    now = HAL_GetTick();
    if ((now - s_joystickLastPrintTick) < JOYSTICK_PRINT_INTERVAL_MS) {
        return;
    }

    delta_x = (s_joystickRawX > s_joystickLastPrintedX)
            ? (uint16_t)(s_joystickRawX - s_joystickLastPrintedX)
            : (uint16_t)(s_joystickLastPrintedX - s_joystickRawX);

    delta_y = (s_joystickRawY > s_joystickLastPrintedY)
            ? (uint16_t)(s_joystickRawY - s_joystickLastPrintedY)
            : (uint16_t)(s_joystickLastPrintedY - s_joystickRawY);

    if ((s_joystickHasPrint != 0U) &&
        (delta_x < JOYSTICK_MIN_CHANGE) &&
        (delta_y < JOYSTICK_MIN_CHANGE)) {
        return;
    }

#if JOYSTICK_ENABLE_PRINTS
    s_joystickLastPrintTick = now;
    s_joystickLastPrintedX = s_joystickRawX;
    s_joystickLastPrintedY = s_joystickRawY;
    s_joystickHasPrint = 1U;

    MyPrint_Printf("JOY turn=%d drive=%d left=%d %s %d right=%d %s %d btn=%u\r\n",
                   (int)s_joystickTurnCommand,
                   (int)s_joystickDriveCommand,
                   (int)s_joystickLeftCommand,
                   s_leftMotorCommand.enabled ? (s_leftMotorCommand.forward ? "FWD" : "REV") : "OFF",
                   (int)(s_leftMotorCommand.duty_percent + 0.5f),
                   (int)s_joystickRightCommand,
                   s_rightMotorCommand.enabled ? (s_rightMotorCommand.forward ? "FWD" : "REV") : "OFF",
                   (int)(s_rightMotorCommand.duty_percent + 0.5f),
                   (unsigned)s_joystickButtonActive);
#else
    (void)now;
    (void)delta_x;
    (void)delta_y;
#endif
}

uint16_t Joystick_GetRawX(void)
{
    return s_joystickRawX;
}

uint16_t Joystick_GetRawY(void)
{
    return s_joystickRawY;
}

int16_t Joystick_GetSpeedCommandX(void)
{
    return s_joystickSpeedCommandX;
}

int16_t Joystick_GetDriveCommand(void)
{
    return s_joystickDriveCommand;
}

int16_t Joystick_GetTurnCommand(void)
{
    return s_joystickTurnCommand;
}

int16_t Joystick_GetLeftCommand(void)
{
    return s_joystickLeftCommand;
}

int16_t Joystick_GetRightCommand(void)
{
    return s_joystickRightCommand;
}

uint8_t Joystick_IsButtonActive(void)
{
    return s_joystickButtonActive;
}

Joystick_MotorCommand Joystick_GetLeftMotorCommand(void)
{
    return s_leftMotorCommand;
}

Joystick_MotorCommand Joystick_GetRightMotorCommand(void)
{
    return s_rightMotorCommand;
}
