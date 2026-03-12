/**
 * @file ehsm_mcal_uart.h
 * @brief MCAL UART: Microcontroller UART interface for host communication.
 * 
 * This is the lowest hardware abstraction layer providing UART transmit
 * functionality. On HOST platform, this is stubbed to printf for debugging.
 * On EMBEDDED platform, this interfaces with actual UART hardware.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_MCAL_UART_H
#define EHSM_MCAL_UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"
#include <stdarg.h>
#include <stddef.h>

/* ============================================================================
 * UART CONFIGURATION
 * ============================================================================
 */

/** Default UART baud rate */
#define EHSM_UART_BAUDRATE      (115200U)

/** Maximum log message length */
#define EHSM_UART_MAX_LOG_LEN   (256U)

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
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartInit(void);

/**
 * @brief Deinitialize the UART peripheral.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartDeinit(void);

/**
 * @brief Check if UART is initialized.
 * 
 * @return true if UART is ready, false otherwise.
 */
bool ehsm_mcal_uartIsReady(void);

/* ============================================================================
 * TRANSMIT OPERATIONS
 * ============================================================================
 */

/**
 * @brief Transmit a single byte over UART.
 * 
 * @param[in] data Byte to transmit.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data);

/**
 * @brief Transmit a buffer of bytes over UART.
 * 
 * @param[in] data Pointer to data buffer.
 * @param[in] length Number of bytes to transmit.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length);

/**
 * @brief Transmit a null-terminated string over UART.
 * 
 * @param[in] str Pointer to null-terminated string.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartTxString(const char* str);

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
 * @param[in] format Format string (printf-style).
 * @param[in] ... Variable arguments.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartLog(const char* format, ...);

/**
 * @brief Log a formatted message via UART (va_list version).
 * 
 * @param[in] format Format string.
 * @param[in] args Variable argument list.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartLogV(const char* format, va_list args);

/**
 * @brief Log a hex dump of data via UART.
 * 
 * @param[in] prefix Label prefix for the dump.
 * @param[in] data Pointer to data to dump.
 * @param[in] length Number of bytes to dump.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mcal_uartLogHex(const char* prefix, const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_MCAL_UART_H */
