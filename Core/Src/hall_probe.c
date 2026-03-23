#include "hall_probe.h"

#include "myprint.h"

static uint8_t s_lastHall1A = 0xFFU;
static uint8_t s_lastHall1B = 0xFFU;
static uint8_t s_lastHall1C = 0xFFU;
static uint32_t s_lastPollTick = 0U;

static uint8_t HallProbe_Read(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

void HallProbe_Init(void)
{
    s_lastHall1A = 0xFFU;
    s_lastHall1B = 0xFFU;
    s_lastHall1C = 0xFFU;
    s_lastPollTick = HAL_GetTick();
    MyPrint_Print("Hall probe ready: polling H1_A/H1_B/H1_C\r\n");
}

void HallProbe_Process(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t hall1a;
    uint8_t hall1b;
    uint8_t hall1c;

    if ((now - s_lastPollTick) < 50U) {
        return;
    }

    s_lastPollTick = now;

    hall1a = HallProbe_Read(GPIOC, GPIO_PIN_9);
    hall1b = HallProbe_Read(GPIOC, GPIO_PIN_1);
    hall1c = HallProbe_Read(GPIOC, GPIO_PIN_7);

    if ((hall1a != s_lastHall1A) ||
        (hall1b != s_lastHall1B) ||
        (hall1c != s_lastHall1C)) {
        s_lastHall1A = hall1a;
        s_lastHall1B = hall1b;
        s_lastHall1C = hall1c;

        MyPrint_Printf("POLL H1: A=%u B=%u C=%u\r\n", hall1a, hall1b, hall1c);
    }
}
