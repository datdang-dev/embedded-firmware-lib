/**
 * @file hsm_storage_driver.h
 * @brief Storage Driver: Non-volatile storage interface for HSM.
 * 
 * Provides abstraction for internal storage used to persist
 * secure data, configuration, and (optionally) encrypted keys.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_STORAGE_DRIVER_H
#define HSM_STORAGE_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"

/* ============================================================================
 * DEFINITIONS
 * ============================================================================
 */

/** Maximum storage address space */
#define HSM_STORAGE_MAX_ADDRESS     (0xFFFFU)

/** Storage sector size in bytes */
#define HSM_STORAGE_SECTOR_SIZE     (4096U)

/** Total storage capacity in bytes */
#define HSM_STORAGE_CAPACITY        (HSM_STORAGE_SECTOR_SIZE * 16U)

/* ============================================================================
 * STORAGE OPERATIONS
 * ============================================================================
 */

/**
 * @brief Initialize the storage driver.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Storage_Init(void);

/**
 * @brief Deinitialize the storage driver.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Storage_Deinit(void);

/**
 * @brief Read data from storage.
 * 
 * @param address Start address to read from.
 * @param[out] data Buffer to store read data.
 * @param length Number of bytes to read.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Storage_Read(uint32_t address, uint8_t* data, size_t length);

/**
 * @brief Write data to storage.
 * 
 * @param address Start address to write to.
 * @param data Data to write.
 * @param length Number of bytes to write.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Storage_Write(uint32_t address, const uint8_t* data, size_t length);

/**
 * @brief Erase a storage sector.
 * 
 * @param address Sector address to erase (must be sector-aligned).
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Storage_EraseSector(uint32_t address);

/**
 * @brief Get storage capacity.
 * 
 * @return Total storage capacity in bytes.
 */
size_t Hsm_Storage_GetCapacity(void);

#ifdef __cplusplus
}
#endif

#endif /* HSM_STORAGE_DRIVER_H */
