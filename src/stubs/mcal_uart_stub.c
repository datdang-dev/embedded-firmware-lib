/**
 * @file mcal_uart_stub.c
 * @brief MCAL UART Stub for HOST platform (PC debugging).
 * 
 * This stub implementation maps UART transmit to printf, allowing
 * developers to trace HSM data flow using GDB on a PC.
 * 
 * When TARGET_PLATFORM=HOST, this file is compiled instead of
 * the real UART driver.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/mcal_uart.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** Maximum log message buffer size */
#define LOG_BUFFER_SIZE     (512U)

/** Stub magic number for validation */
#define UART_STUB_MAGIC     (0x55415254U)  /* "UART" */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** UART initialization state */
static uint32_t g_uart_initialized = 0U;

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

Hsm_Status_t Mcal_Uart_Init(void)
{
    if (g_uart_initialized == UART_STUB_MAGIC)
    {
        return HSM_OK;  /* Already initialized */
    }
    
    /* On HOST, we just mark as initialized - stdout is always available */
    g_uart_initialized = UART_STUB_MAGIC;
    
    /* Print startup message */
    printf("[UART-STUB] Initialized (HOST platform)\n");
    printf("[UART-STUB] All UART output will be redirected to stdout\n");
    
    return HSM_OK;
}

Hsm_Status_t Mcal_Uart_Deinit(void)
{
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    printf("[UART-STUB] Deinitialized\n");
    
    g_uart_initialized = 0U;
    
    return HSM_OK;
}

bool Mcal_Uart_IsReady(void)
{
    return (g_uart_initialized == UART_STUB_MAGIC);
}

/* ============================================================================
 * TRANSMIT OPERATIONS (Stubbed to stdout)
 * ============================================================================
 */

Hsm_Status_t Mcal_Uart_TxByte(uint8_t data)
{
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Map UART byte transmit to putchar */
    putchar((char)data);
    fflush(stdout);
    
    return HSM_OK;
}

Hsm_Status_t Mcal_Uart_TxBuffer(const uint8_t* data, size_t length)
{
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    if (data == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Map UART buffer transmit to fwrite */
    size_t written = fwrite(data, 1, length, stdout);
    fflush(stdout);
    
    return (written == length) ? HSM_OK : HSM_ERR_HARDWARE;
}

Hsm_Status_t Mcal_Uart_TxString(const char* str)
{
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    if (str == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Map UART string transmit to fputs */
    fputs(str, stdout);
    fflush(stdout);
    
    return HSM_OK;
}

/* ============================================================================
 * LOGGING FUNCTIONS
 * ============================================================================
 */

Hsm_Status_t Mcal_Uart_Log(const char* format, ...)
{
    Hsm_Status_t status;
    va_list args;
    
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    if (format == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Get timestamp */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    /* Print timestamp prefix */
    printf("[%s] ", timestamp);
    
    /* Process variable arguments */
    va_start(args, format);
    status = Mcal_Uart_LogV(format, args);
    va_end(args);
    
    /* Print newline */
    printf("\n");
    fflush(stdout);
    
    return status;
}

Hsm_Status_t Mcal_Uart_LogV(const char* format, va_list args)
{
    char buffer[LOG_BUFFER_SIZE];
    int result;
    
    if (format == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Format the message */
    result = vsnprintf(buffer, sizeof(buffer), format, args);
    
    if (result < 0)
    {
        return HSM_ERR_INTERNAL;
    }
    
    if ((size_t)result >= sizeof(buffer))
    {
        /* Message was truncated */
        buffer[sizeof(buffer) - 1] = '\0';
    }
    
    /* Output to stdout */
    fputs(buffer, stdout);
    fflush(stdout);
    
    return HSM_OK;
}

Hsm_Status_t Mcal_Uart_LogHex(const char* prefix, const uint8_t* data, size_t length)
{
    if (g_uart_initialized != UART_STUB_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    if ((prefix == NULL) || (data == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Print prefix */
    printf("%s (%zu bytes): ", prefix, length);
    
    /* Print hex dump (up to 32 bytes for readability) */
    size_t dump_len = (length > 32U) ? 32U : length;
    for (size_t i = 0U; i < dump_len; i++)
    {
        printf("%02X ", data[i]);
        
        /* Add space every 8 bytes for readability */
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
    
    return HSM_OK;
}
