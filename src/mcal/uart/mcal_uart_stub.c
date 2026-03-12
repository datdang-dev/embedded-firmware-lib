/**
 * @file mcal_uart_stub.c
 * @brief MCAL UART stub for HOST platform (C code).
 * 
 * This stub implementation maps UART transmit to printf for debugging.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EHSM_STATUS_OK = 0,
    EHSM_STATUS_ERR_INVALID_PARAM = -1,
    EHSM_STATUS_ERR_NOT_INITIALIZED = -2
} ehsm_status_t;

static uint32_t g_uart_initialized = 0U;
#define UART_MAGIC 0x55415254U

ehsm_status_t ehsm_mcal_uartInit(void) {
    if (g_uart_initialized == UART_MAGIC) {
        return EHSM_STATUS_OK;
    }
    
    g_uart_initialized = UART_MAGIC;
    printf("[UART-STUB] Initialized (HOST platform)\n");
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartDeinit(void) {
    if (g_uart_initialized != UART_MAGIC) {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    g_uart_initialized = 0U;
    printf("[UART-STUB] Deinitialized\n");
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data) {
    if (g_uart_initialized != UART_MAGIC) {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    putchar((char)data);
    fflush(stdout);
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length) {
    if (g_uart_initialized != UART_MAGIC) {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (data == NULL) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    size_t written = fwrite(data, 1, length, stdout);
    fflush(stdout);
    
    return (written == length) ? EHSM_STATUS_OK : EHSM_STATUS_ERR_INVALID_PARAM;
}

ehsm_status_t ehsm_mcal_uartTxString(const char* str) {
    if (g_uart_initialized != UART_MAGIC) {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (str == NULL) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    fputs(str, stdout);
    fflush(stdout);
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mcal_uartLog(const char* format, ...) {
    if (g_uart_initialized != UART_MAGIC) {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (format == NULL) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);
    
    printf("[%s] ", timestamp);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
    
    return EHSM_STATUS_OK;
}

#ifdef __cplusplus
}
#endif
