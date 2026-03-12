/**
 * @file ehsm_mcal_uart_stub.c
 * @brief MCAL UART Stub for HOST platform (PC debugging).
 * 
 * This stub implementation maps UART transmit to printf, allowing
 * developers to trace HSM data flow using GDB on a PC.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_mcal_uart.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

#define LOG_BUFFER_SIZE     (512U)
#define UART_STUB_MAGIC     (0x55415254U)

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

static uint32_t g_uartInitialized = 0U;

/* ============================================================================
 * PUBLIC IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_mcal_uartInit(void)
{
    if (g_uartInitialized == UART_STUB_MAGIC)
    {
        return EHSM_STATUS_OK;
    }
    
    g_uartInitialized = UART_STUB_MAGIC;
    
    printf("[UART-STUB] Initialized (HOST platform)\n");
    printf("[UART-STUB] All UART output redirected to stdout\n");
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartDeinit(void)
{
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    printf("[UART-STUB] Deinitialized\n");
    g_uartInitialized = 0U;
    
    return EHSM_STATUS_OK;
}

bool ehsm_mcal_uartIsReady(void)
{
    return (g_uartInitialized == UART_STUB_MAGIC);
}

ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data)
{
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    putchar((char)data);
    fflush(stdout);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length)
{
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (data == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    size_t written = fwrite(data, 1, length, stdout);
    fflush(stdout);
    
    return (written == length) ? EHSM_STATUS_OK : EHSM_STATUS_ERR_HARDWARE;
}

ehsm_status_t ehsm_mcal_uartTxString(const char* str)
{
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (str == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    fputs(str, stdout);
    fflush(stdout);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartLog(const char* format, ...)
{
    ehsm_status_t status;
    va_list args;
    
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (format == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    time_t now = time(NULL);
    struct tm* tmInfo = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tmInfo);
    
    printf("[%s] ", timestamp);
    
    va_start(args, format);
    status = ehsm_mcal_uartLogV(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
    
    return status;
}

ehsm_status_t ehsm_mcal_uartLogV(const char* format, va_list args)
{
    char buffer[LOG_BUFFER_SIZE];
    int result;
    
    if (format == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    result = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (result < 0)
    {
        return EHSM_STATUS_ERR_INTERNAL;
    }
    
    if ((size_t)result >= sizeof(buffer))
    {
        buffer[sizeof(buffer) - 1] = '\0';
    }
    
    fputs(buffer, stdout);
    fflush(stdout);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartLogHex(const char* prefix, const uint8_t* data, size_t length)
{
    if (g_uartInitialized != UART_STUB_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if ((prefix == NULL) || (data == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    printf("%s (%zu bytes): ", prefix, length);
    
    size_t dumpLen = (length > 32U) ? 32U : length;
    for (size_t i = 0U; i < dumpLen; i++)
    {
        printf("%02X ", data[i]);
        
        if (((i + 1U) % 8U) == 0U)
        {
            printf(" ");
        }
    }
    
    if (length > 32U)
    {
        printf("... (+%zu more)", length - 32U);
    }
    
    printf("\n");
    fflush(stdout);
    
    return EHSM_STATUS_OK;
}
