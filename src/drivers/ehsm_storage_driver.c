/**
 * @file ehsm_storage_driver.c
 * @brief Implementation of the Storage Driver.
 * 
 * Provides mock implementation for non-volatile storage.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_storage.h"
#include "ehsm/ehsm_mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

#define STORAGE_BUFFER_SIZE     (EHSM_STORAGE_CAPACITY)
#define STORAGE_INIT_MAGIC      (0x53544F52U)

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

static uint8_t g_storageBuffer[STORAGE_BUFFER_SIZE];
static uint32_t g_storageInitialized = 0U;

/* ============================================================================
 * PUBLIC IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_storage_init(void)
{
    if (g_storageInitialized == STORAGE_INIT_MAGIC)
    {
        return EHSM_STATUS_OK;
    }
    
    (void)memset(g_storageBuffer, 0xFF, sizeof(g_storageBuffer));
    g_storageInitialized = STORAGE_INIT_MAGIC;
    
    ehsm_mcal_uartLog("[STORAGE] Initialized: capacity=%zu bytes", STORAGE_BUFFER_SIZE);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_storage_deinit(void)
{
    if (g_storageInitialized != STORAGE_INIT_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    (void)memset(g_storageBuffer, 0, sizeof(g_storageBuffer));
    g_storageInitialized = 0U;
    
    ehsm_mcal_uartLog("[STORAGE] Deinitialized");
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_storage_read(uint32_t address, uint8_t* data, size_t length)
{
    if (data == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (g_storageInitialized != STORAGE_INIT_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if ((address >= EHSM_STORAGE_CAPACITY) ||
        ((address + length) > EHSM_STORAGE_CAPACITY))
    {
        ehsm_mcal_uartLog("[STORAGE] Read failed: invalid address %lu", (unsigned long)address);
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    (void)memcpy(data, &g_storageBuffer[address], length);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_storage_write(uint32_t address, const uint8_t* data, size_t length)
{
    if (data == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (g_storageInitialized != STORAGE_INIT_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if ((address >= EHSM_STORAGE_CAPACITY) ||
        ((address + length) > EHSM_STORAGE_CAPACITY))
    {
        ehsm_mcal_uartLog("[STORAGE] Write failed: invalid address %lu", (unsigned long)address);
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    (void)memcpy(&g_storageBuffer[address], data, length);
    
    ehsm_mcal_uartLog("[STORAGE] Write: addr=%lu, len=%zu", (unsigned long)address, length);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_storage_eraseSector(uint32_t address)
{
    if (g_storageInitialized != STORAGE_INIT_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if ((address % EHSM_STORAGE_SECTOR_SIZE) != 0U)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (address >= EHSM_STORAGE_CAPACITY)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    (void)memset(&g_storageBuffer[address], 0xFF, EHSM_STORAGE_SECTOR_SIZE);
    
    ehsm_mcal_uartLog("[STORAGE] Erase sector: addr=%lu", (unsigned long)address);
    
    return EHSM_STATUS_OK;
}

size_t ehsm_storage_getCapacity(void)
{
    return STORAGE_BUFFER_SIZE;
}
