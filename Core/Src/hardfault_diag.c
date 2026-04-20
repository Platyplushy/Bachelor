#include "hardfault_diag.h"

#include "main.h"

volatile uint32_t g_hardfault_hfsr = 0U;
volatile uint32_t g_hardfault_cfsr = 0U;
volatile uint32_t g_hardfault_mmfar = 0U;
volatile uint32_t g_hardfault_bfar = 0U;

void HardFaultDiag_Handle(void)
{
    g_hardfault_hfsr = SCB->HFSR;
    g_hardfault_cfsr = SCB->CFSR;
    g_hardfault_mmfar = SCB->MMFAR;
    g_hardfault_bfar = SCB->BFAR;
    NVIC_SystemReset();
}
