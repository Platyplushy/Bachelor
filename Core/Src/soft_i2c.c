#include "soft_i2c.h"

#define SOFT_I2C_SCL_PORT GPIOB
#define SOFT_I2C_SCL_PIN  GPIO_PIN_7
#define SOFT_I2C_SDA_PORT GPIOB
#define SOFT_I2C_SDA_PIN  GPIO_PIN_11

static void SoftI2C_Delay(void)
{
    volatile uint32_t i;

    for (i = 0U; i < 80U; ++i) {
        __NOP();
    }
}

static void SoftI2C_SclHigh(void)
{
    HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_SET);
}

static void SoftI2C_SclLow(void)
{
    HAL_GPIO_WritePin(SOFT_I2C_SCL_PORT, SOFT_I2C_SCL_PIN, GPIO_PIN_RESET);
}

static void SoftI2C_SdaHigh(void)
{
    HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_SET);
}

static void SoftI2C_SdaLow(void)
{
    HAL_GPIO_WritePin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN, GPIO_PIN_RESET);
}

static uint8_t SoftI2C_ReadSda(void)
{
    return (HAL_GPIO_ReadPin(SOFT_I2C_SDA_PORT, SOFT_I2C_SDA_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

static void SoftI2C_Start(void)
{
    SoftI2C_SdaHigh();
    SoftI2C_SclHigh();
    SoftI2C_Delay();
    SoftI2C_SdaLow();
    SoftI2C_Delay();
    SoftI2C_SclLow();
}

static void SoftI2C_Stop(void)
{
    SoftI2C_SdaLow();
    SoftI2C_Delay();
    SoftI2C_SclHigh();
    SoftI2C_Delay();
    SoftI2C_SdaHigh();
    SoftI2C_Delay();
}

static uint8_t SoftI2C_WriteByte(uint8_t value)
{
    uint8_t bit_index;

    for (bit_index = 0U; bit_index < 8U; ++bit_index) {
        if ((value & 0x80U) != 0U) {
            SoftI2C_SdaHigh();
        } else {
            SoftI2C_SdaLow();
        }

        SoftI2C_Delay();
        SoftI2C_SclHigh();
        SoftI2C_Delay();
        SoftI2C_SclLow();
        value <<= 1U;
    }

    SoftI2C_SdaHigh();
    SoftI2C_Delay();
    SoftI2C_SclHigh();
    SoftI2C_Delay();

    bit_index = SoftI2C_ReadSda();

    SoftI2C_SclLow();
    SoftI2C_Delay();

    return (bit_index == 0U) ? 1U : 0U;
}

void SoftI2C_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_init.Pin = SOFT_I2C_SCL_PIN | SOFT_I2C_SDA_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    SoftI2C_SdaHigh();
    SoftI2C_SclHigh();
}

uint8_t SoftI2C_Write(uint8_t address_7bit, const uint8_t *data, uint8_t length)
{
    uint8_t i;

    if ((data == NULL) || (length == 0U)) {
        return 0U;
    }

    SoftI2C_Start();

    if (SoftI2C_WriteByte((uint8_t)(address_7bit << 1U)) == 0U) {
        SoftI2C_Stop();
        return 0U;
    }

    for (i = 0U; i < length; ++i) {
        if (SoftI2C_WriteByte(data[i]) == 0U) {
            SoftI2C_Stop();
            return 0U;
        }
    }

    SoftI2C_Stop();
    return 1U;
}
