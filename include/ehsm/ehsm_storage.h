/**
 * @file ehsm_storage.h
 * @brief Storage Driver: Non-volatile storage interface for HSM.
 * 
 * Provides abstraction for internal storage used to persist
 * secure data, configuration, and (optionally) encrypted keys.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_STORAGE_H
#define EHSM_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"

/* ============================================================================
 * DEFINITIONS
 * ============================================================================
 */

#define EHSM_STORAGE_MAX_ADDRESS    (0xFFFFU)
#define EHSM_STORAGE_SECTOR_SIZE    (4096U)
#define EHSM_STORAGE_CAPACITY       (EHSM_STORAGE_SECTOR_SIZE * 16U)

/* ============================================================================
 * STORAGE OPERATIONS
 * ============================================================================
 */

/**
 * @brief Initialize the storage driver.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_storage_init(void);

/**
 * @brief Deinitialize the storage driver.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_storage_deinit(void);

/**
 * @brief Read data from storage.
 * 
 * @param[in] address Start address to read from.
 * @param[out] data Buffer to store read data.
 * @param[in] length Number of bytes to read.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_storage_read(uint32_t address, uint8_t* data, size_t length);

/**
 * @brief Write data to storage.
 * 
 * @param[in] address Start address to write to.
 * @param[in] data Data to write.
 * @param[in] length Number of bytes to write.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_storage_write(uint32_t address, const uint8_t* data, size_t length);

/**
 * @brief Erase a storage sector.
 * 
 * @param[in] address Sector address to erase (must be sector-aligned).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_storage_eraseSector(uint32_t address);

/**
 * @brief Get storage capacity.
 * 
 * @return Total storage capacity in bytes.
 */
size_t ehsm_storage_getCapacity(void);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_STORAGE_H */
