#ifndef HARDFLT_DIAG_H
#define HARDFLT_DIAG_H

#include <stdint.h>

extern volatile uint32_t g_hardfault_hfsr;
extern volatile uint32_t g_hardfault_cfsr;
extern volatile uint32_t g_hardfault_mmfar;
extern volatile uint32_t g_hardfault_bfar;

void HardFaultDiag_Handle(void);

#endif /* HARDFLT_DIAG_H */
