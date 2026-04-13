#include "myprint.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32g4xx_nucleo.h"

#define MYPRINT_BUFFER_SIZE 128U
#define MYPRINT_TX_TIMEOUT_MS 100U

static void MyPrint_WriteNormalized(const char *text, size_t length)
{
    size_t index;
    uint8_t carriage_return = '\r';

    for (index = 0U; index < length; ++index) {
        if (text[index] == '\n' && (index == 0U || text[index - 1U] != '\r')) {
            (void)HAL_UART_Transmit(&hcom_uart[COM1], &carriage_return, 1U, MYPRINT_TX_TIMEOUT_MS);
        }

        (void)HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)&text[index], 1U, MYPRINT_TX_TIMEOUT_MS);
    }
}

void MyPrint_Init(void)
{
    (void)BSP_COM_SelectLogPort(COM1);
    MyPrint_Print("myprint ready\r\n");
    MyPrint_Print("COM1/LPUART1: TX=PA2 RX=PA3 115200 8N1\r\n");
    MyPrint_Print("Terminal log active\r\n");
}

void MyPrint_Print(const char *text)
{
    size_t length;

    if (text == NULL) {
        return;
    }

    length = strlen(text);
    if (length == 0U) {
        return;
    }

    MyPrint_WriteNormalized(text, length);
}

void MyPrint_Printf(const char *format, ...)
{
    va_list args;
    char buffer[MYPRINT_BUFFER_SIZE];
    int length;

    if (format == NULL) {
        return;
    }

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0) {
        return;
    }

    if ((size_t)length >= sizeof(buffer)) {
        length = (int)sizeof(buffer) - 1;
    }

    MyPrint_WriteNormalized(buffer, (size_t)length);
}

HAL_StatusTypeDef MyPrint_ReadByte(uint8_t *byte)
{
    if (byte == NULL) {
        return HAL_ERROR;
    }

    return HAL_UART_Receive(&hcom_uart[COM1], byte, 1U, 0U);
}

void MyPrint_ProcessTerminal(void)
{
    uint8_t received_byte;

    while (MyPrint_ReadByte(&received_byte) == HAL_OK) {
        /* Discard terminal RX traffic to keep logs clean. */
    }
}
