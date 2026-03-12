/**
 * @file mcal_uart.h
 * @brief MCAL UART: Microcontroller UART interface for host communication.
 * 
 * This is the lowest hardware abstraction layer providing UART transmit
 * functionality. On HOST platform, this is stubbed to printf for debugging.
 * On EMBEDDED platform, this interfaces with actual UART hardware.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef MCAL_UART_H
#define MCAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"
#include <stdarg.h>
#include <stddef.h>

/* ============================================================================
 * UART CONFIGURATION
 * ============================================================================
 */

/** Default UART baud rate */
#define MCAL_UART_BAUDRATE      (115200U)

/** Maximum log message length */
#define MCAL_UART_MAX_LOG_LEN   (256U)

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

/**
 * @brief Initialize the UART peripheral.
 * 
 * Configures UART hardware for communication.
 * Must be called before any other UART functions.
 * 
 * @return HSM_OK on success, error code otherwise.
 * 
 * @pre None
 * @post UART is ready for transmission
 */
Hsm_Status_t Mcal_Uart_Init(void);

/**
 * @brief Deinitialize the UART peripheral.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_Deinit(void);

/**
 * @brief Check if UART is initialized.
 * 
 * @return true if UART is ready, false otherwise.
 */
bool Mcal_Uart_IsReady(void);

/* ============================================================================
 * TRANSMIT OPERATIONS
 * ============================================================================
 */

/**
 * @brief Transmit a single byte over UART.
 * 
 * @param data Byte to transmit.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_TxByte(uint8_t data);

/**
 * @brief Transmit a buffer of bytes over UART.
 * 
 * @param data Pointer to data buffer.
 * @param length Number of bytes to transmit.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_TxBuffer(const uint8_t* data, size_t length);

/**
 * @brief Transmit a null-terminated string over UART.
 * 
 * @param str Pointer to null-terminated string.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_TxString(const char* str);

/* ============================================================================
 * LOGGING FUNCTIONS (Debug/Trace)
 * ============================================================================
 */

/**
 * @brief Log a formatted message via UART.
 * 
 * Similar to printf but routes through UART. Used for tracing
 * data flow through the HSM stack during debugging.
 * 
 * @param format Format string (printf-style).
 * @param ... Variable arguments.
 * @return HSM_OK on success, error code otherwise.
 * 
 * @example
 * ```c
 * Mcal_Uart_Log("[HSM] Encrypt: len=%d, result=%d", length, result);
 * ```
 */
Hsm_Status_t Mcal_Uart_Log(const char* format, ...);

/**
 * @brief Log a formatted message via UART (va_list version).
 * 
 * @param format Format string.
 * @param args Variable argument list.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_LogV(const char* format, va_list args);

/**
 * @brief Log a hex dump of data via UART.
 * 
 * @param prefix Label prefix for the dump.
 * @param data Pointer to data to dump.
 * @param length Number of bytes to dump.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Mcal_Uart_LogHex(const char* prefix, const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MCAL_UART_H */
