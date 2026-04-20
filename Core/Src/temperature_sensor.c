#include "temperature_sensor.h"

#include <math.h>

#include "adc.h"

#define TEMPERATURE_SENSOR_ADC_MAX_VALUE         4095.0f
#define TEMPERATURE_SENSOR_FIXED_RESISTOR_OHMS   51000.0f
#define TEMPERATURE_SENSOR_R25_OHMS              50000.0f
#define TEMPERATURE_SENSOR_BETA                  3950.0f
#define TEMPERATURE_SENSOR_T0_KELVIN             298.15f
#define TEMPERATURE_SENSOR_UPDATE_INTERVAL_MS    500U
#define TEMPERATURE_SENSOR_FILTER_ALPHA          0.25f
#define TEMPERATURE_SENSOR_INVALID_C             (-273.15f)

typedef struct {
    uint32_t adc_channel;
    uint16_t raw_adc;
    float temperature_c;
    uint8_t valid;
} TemperatureSensor_Runtime;

static TemperatureSensor_Runtime s_temperatureRuntime[2];
static uint32_t s_temperatureLastUpdateTickMs;

static uint16_t TemperatureSensor_ReadAdcChannel(uint32_t channel);
static float TemperatureSensor_AdcToTemperatureC(uint16_t adc_raw);

void TemperatureSensor_Init(void)
{
    uint32_t i;

    for (i = 0U; i < 2U; ++i) {
        s_temperatureRuntime[i].raw_adc = 0U;
        s_temperatureRuntime[i].temperature_c = 0.0f;
        s_temperatureRuntime[i].valid = 0U;
    }

    s_temperatureRuntime[0].adc_channel = ADC_CHANNEL_3;
    s_temperatureRuntime[1].adc_channel = ADC_CHANNEL_4;
    s_temperatureLastUpdateTickMs = HAL_GetTick();
}

void TemperatureSensor_Process(void)
{
    uint32_t i;
    const uint32_t now = HAL_GetTick();

    if ((now - s_temperatureLastUpdateTickMs) < TEMPERATURE_SENSOR_UPDATE_INTERVAL_MS) {
        return;
    }

    s_temperatureLastUpdateTickMs = now;

    for (i = 0U; i < 2U; ++i) {
        const uint16_t adc_raw = TemperatureSensor_ReadAdcChannel(s_temperatureRuntime[i].adc_channel);
        const float temperature_c = TemperatureSensor_AdcToTemperatureC(adc_raw);

        s_temperatureRuntime[i].raw_adc = adc_raw;

        if (temperature_c <= TEMPERATURE_SENSOR_INVALID_C) {
            continue;
        }

        if (s_temperatureRuntime[i].valid == 0U) {
            s_temperatureRuntime[i].temperature_c = temperature_c;
            s_temperatureRuntime[i].valid = 1U;
            continue;
        }

        s_temperatureRuntime[i].temperature_c =
            ((1.0f - TEMPERATURE_SENSOR_FILTER_ALPHA) * s_temperatureRuntime[i].temperature_c) +
            (TEMPERATURE_SENSOR_FILTER_ALPHA * temperature_c);
    }
}

uint16_t TemperatureSensor_GetMotor1RawAdc(void)
{
    return s_temperatureRuntime[0].raw_adc;
}

uint16_t TemperatureSensor_GetMotor2RawAdc(void)
{
    return s_temperatureRuntime[1].raw_adc;
}

float TemperatureSensor_GetMotor1TemperatureC(void)
{
    if (s_temperatureRuntime[0].valid == 0U) {
        return TEMPERATURE_SENSOR_INVALID_C;
    }

    return s_temperatureRuntime[0].temperature_c;
}

float TemperatureSensor_GetMotor2TemperatureC(void)
{
    if (s_temperatureRuntime[1].valid == 0U) {
        return TEMPERATURE_SENSOR_INVALID_C;
    }

    return s_temperatureRuntime[1].temperature_c;
}

static uint16_t TemperatureSensor_ReadAdcChannel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        return 0U;
    }

    if (HAL_ADC_Start(&hadc2) != HAL_OK) {
        return 0U;
    }

    if (HAL_ADC_PollForConversion(&hadc2, 10U) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc2);
        return 0U;
    }

    {
        const uint16_t adc_raw = (uint16_t)HAL_ADC_GetValue(&hadc2);
        (void)HAL_ADC_Stop(&hadc2);
        return adc_raw;
    }
}

static float TemperatureSensor_AdcToTemperatureC(uint16_t adc_raw)
{
    const float adc_value = (float)adc_raw;
    float r_ntc;
    float inv_t;

    if ((adc_value <= 0.0f) || (adc_value >= TEMPERATURE_SENSOR_ADC_MAX_VALUE)) {
        return TEMPERATURE_SENSOR_INVALID_C;
    }

    r_ntc = TEMPERATURE_SENSOR_FIXED_RESISTOR_OHMS *
            (adc_value / (TEMPERATURE_SENSOR_ADC_MAX_VALUE - adc_value));

    if (r_ntc <= 0.0f) {
        return TEMPERATURE_SENSOR_INVALID_C;
    }

    inv_t = (1.0f / TEMPERATURE_SENSOR_T0_KELVIN) +
            (logf(r_ntc / TEMPERATURE_SENSOR_R25_OHMS) / TEMPERATURE_SENSOR_BETA);

    if (inv_t <= 0.0f) {
        return TEMPERATURE_SENSOR_INVALID_C;
    }

    return (1.0f / inv_t) - 273.15f;
}
