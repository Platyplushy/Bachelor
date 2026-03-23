#include "hall_debug.h"

#include "myprint.h"

#define HALL_DEBUG_ENABLE_MOTOR2 0U

#define HALL_EVENT_H1_A (1U << 0)
#define HALL_EVENT_H1_B (1U << 1)
#define HALL_EVENT_H1_C (1U << 2)
#define HALL_EVENT_H2_A (1U << 3)
#define HALL_EVENT_H2_B (1U << 4)
#define HALL_EVENT_H2_C (1U << 5)

static volatile uint32_t s_hallEventFlags = 0U;
static volatile uint8_t s_hall1SnapshotA = 0U;
static volatile uint8_t s_hall1SnapshotB = 0U;
static volatile uint8_t s_hall1SnapshotC = 0U;
static volatile uint8_t s_hall2SnapshotA = 0U;
static volatile uint8_t s_hall2SnapshotB = 0U;
static volatile uint8_t s_hall2SnapshotC = 0U;

static uint8_t HallDebug_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

static void HallDebug_CaptureSnapshots(void)
{
    s_hall1SnapshotA = HallDebug_ReadPin(GPIOC, GPIO_PIN_9);
    s_hall1SnapshotB = HallDebug_ReadPin(GPIOC, GPIO_PIN_1);
    s_hall1SnapshotC = HallDebug_ReadPin(GPIOC, GPIO_PIN_7);
    s_hall2SnapshotA = HallDebug_ReadPin(GPIOC, GPIO_PIN_2);
    s_hall2SnapshotB = HallDebug_ReadPin(GPIOC, GPIO_PIN_3);
    s_hall2SnapshotC = HallDebug_ReadPin(GPIOC, GPIO_PIN_8);
}

static void HallDebug_PrintHall1State(const char *source)
{
    MyPrint_Printf("%s IRQ: H1_A=%u H1_B=%u H1_C=%u\r\n",
                   source,
                   s_hall1SnapshotA,
                   s_hall1SnapshotB,
                   s_hall1SnapshotC);
}

static void HallDebug_PrintHall2State(const char *source)
{
    MyPrint_Printf("%s IRQ: H2_A=%u H2_B=%u H2_C=%u\r\n",
                   source,
                   HallDebug_ReadPin(GPIOC, GPIO_PIN_2),
                   HallDebug_ReadPin(GPIOC, GPIO_PIN_3),
                   HallDebug_ReadPin(GPIOC, GPIO_PIN_8));
}

void HallDebug_Init(void)
{
    MyPrint_Print("Hall debug ready: H1=PC9/PC1/PC7\r\n");
}

void HallDebug_Process(void)
{
    uint32_t event_flags;

    __disable_irq();
    event_flags = s_hallEventFlags;
    s_hallEventFlags = 0U;
    __enable_irq();

    if (event_flags & HALL_EVENT_H1_A) {
        HallDebug_PrintHall1State("EXTI HALL_1_A");
    }
    if (event_flags & HALL_EVENT_H1_B) {
        HallDebug_PrintHall1State("EXTI HALL_1_B");
    }
    if (event_flags & HALL_EVENT_H1_C) {
        HallDebug_PrintHall1State("EXTI HALL_1_C");
    }
    if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_A) != 0U)) {
        HallDebug_PrintHall2State("EXTI HALL_2_A");
    }
    if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_B) != 0U)) {
        HallDebug_PrintHall2State("EXTI HALL_2_B");
    }
    if ((HALL_DEBUG_ENABLE_MOTOR2 != 0U) && ((event_flags & HALL_EVENT_H2_C) != 0U)) {
        HallDebug_PrintHall2State("EXTI HALL_2_C");
    }
}

void HallDebug_OnExti(uint16_t gpio_pin)
{
    HallDebug_CaptureSnapshots();

    switch (gpio_pin) {
        case GPIO_PIN_9:
            s_hallEventFlags |= HALL_EVENT_H1_A;
            break;
        case GPIO_PIN_1:
            s_hallEventFlags |= HALL_EVENT_H1_B;
            break;
        case GPIO_PIN_7:
            s_hallEventFlags |= HALL_EVENT_H1_C;
            break;
        case GPIO_PIN_2:
            if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {
                s_hallEventFlags |= HALL_EVENT_H2_A;
            }
            break;
        case GPIO_PIN_3:
            if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {
                s_hallEventFlags |= HALL_EVENT_H2_B;
            }
            break;
        case GPIO_PIN_8:
            if (HALL_DEBUG_ENABLE_MOTOR2 != 0U) {
                s_hallEventFlags |= HALL_EVENT_H2_C;
            }
            break;
        default:
            break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HallDebug_OnExti(GPIO_Pin);
}
