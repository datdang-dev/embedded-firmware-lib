/**
 * @file hsm_storage_driver.c
 * @brief Implementation of the Storage Driver.
 * 
 * Provides mock implementation for non-volatile storage.
 * In production, this would interface with flash memory or secure element.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_storage_driver.h"
#include "hsm/mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** Mock storage buffer size */
#define STORAGE_BUFFER_SIZE     (HSM_STORAGE_CAPACITY)

/** Magic number for storage validation */
#define STORAGE_INIT_MAGIC      (0x53544F52U)  /* "STOR" */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** Mock storage buffer (simulates flash memory) */
static uint8_t g_storage_buffer[STORAGE_BUFFER_SIZE];

/** Storage initialization flag */
static uint32_t g_storage_initialized = 0U;

/* ============================================================================
 * STORAGE OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Storage_Init(void)
{
    /* Check if already initialized */
    if (g_storage_initialized == STORAGE_INIT_MAGIC)
    {
        return HSM_OK;
    }
    
    /* Initialize storage buffer (simulates erased flash = all 0xFF) */
    (void)memset(g_storage_buffer, 0xFF, sizeof(g_storage_buffer));
    
    g_storage_initialized = STORAGE_INIT_MAGIC;
    
    Mcal_Uart_Log("[STORAGE] Initialized: capacity=%zu bytes", STORAGE_BUFFER_SIZE);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Storage_Deinit(void)
{
    /* Check if initialized */
    if (g_storage_initialized != STORAGE_INIT_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Clear storage buffer */
    (void)memset(g_storage_buffer, 0, sizeof(g_storage_buffer));
    
    g_storage_initialized = 0U;
    
    Mcal_Uart_Log("[STORAGE] Deinitialized");
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Storage_Read(uint32_t address, uint8_t* data, size_t length)
{
    /* Validate parameters */
    if (data == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if initialized */
    if (g_storage_initialized != STORAGE_INIT_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate address range */
    if ((address >= HSM_STORAGE_CAPACITY) ||
        ((address + length) > HSM_STORAGE_CAPACITY))
    {
        Mcal_Uart_Log("[STORAGE] Read failed: invalid address %lu", (unsigned long)address);
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Read from storage buffer */
    (void)memcpy(data, &g_storage_buffer[address], length);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Storage_Write(uint32_t address, const uint8_t* data, size_t length)
{
    /* Validate parameters */
    if (data == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if initialized */
    if (g_storage_initialized != STORAGE_INIT_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate address range */
    if ((address >= HSM_STORAGE_CAPACITY) ||
        ((address + length) > HSM_STORAGE_CAPACITY))
    {
        Mcal_Uart_Log("[STORAGE] Write failed: invalid address %lu", (unsigned long)address);
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Write to storage buffer */
    (void)memcpy(&g_storage_buffer[address], data, length);
    
    Mcal_Uart_Log("[STORAGE] Write: addr=%lu, len=%zu", (unsigned long)address, length);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Storage_EraseSector(uint32_t address)
{
    /* Check if initialized */
    if (g_storage_initialized != STORAGE_INIT_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate address (must be sector-aligned) */
    if ((address % HSM_STORAGE_SECTOR_SIZE) != 0U)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    if (address >= HSM_STORAGE_CAPACITY)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Erase sector (set to 0xFF) */
    (void)memset(&g_storage_buffer[address], 0xFF, HSM_STORAGE_SECTOR_SIZE);
    
    Mcal_Uart_Log("[STORAGE] Erase sector: addr=%lu", (unsigned long)address);
    
    return HSM_OK;
}

size_t Hsm_Storage_GetCapacity(void)
{
    return STORAGE_BUFFER_SIZE;
}
