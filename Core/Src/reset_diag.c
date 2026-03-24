#include "reset_diag.h"

#include "main.h"
#include "myprint.h"

void ResetDiag_PrintAndClear(void)
{
    uint32_t csr = RCC->CSR;

    MyPrint_Printf("Reset cause: OBL=%u BOR=%u PIN=%u SFTRST=%u IWDG=%u WWDG=%u LPWR=%u\r\n",
                   (unsigned)((csr & RCC_CSR_OBLRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_BORRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_PINRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_SFTRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_IWDGRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_WWDGRSTF) != 0U),
                   (unsigned)((csr & RCC_CSR_LPWRRSTF) != 0U));

    __HAL_RCC_CLEAR_RESET_FLAGS();
}
