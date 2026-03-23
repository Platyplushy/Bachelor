#include "myprint.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32g4xx_nucleo.h"

#define MYPRINT_BUFFER_SIZE 128U
#define MYPRINT_TX_TIMEOUT_MS 100U

void MyPrint_Init(void)
{
    (void)BSP_COM_SelectLogPort(COM1);
    MyPrint_Print("\r\nmyprint ready\r\n");
    MyPrint_Print("COM1/LPUART1: TX=PA2 RX=PA3 115200 8N1\r\n");
    MyPrint_Print("Type characters in the terminal to test RX echo.\r\n");
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

    (void)HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)text, (uint16_t)length, MYPRINT_TX_TIMEOUT_MS);
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

    (void)HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)buffer, (uint16_t)length, MYPRINT_TX_TIMEOUT_MS);
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

    if (MyPrint_ReadByte(&received_byte) != HAL_OK) {
        return;
    }

    if (received_byte == '\r') {
        MyPrint_Print("\r\n");
        return;
    }

    (void)HAL_UART_Transmit(&hcom_uart[COM1], &received_byte, 1U, MYPRINT_TX_TIMEOUT_MS);
}
