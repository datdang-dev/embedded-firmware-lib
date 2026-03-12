/**
 * @file ehsm_keystore.h
 * @brief Keystore Manager: Secure key storage and access control.
 * 
 * Manages key slots, enforces access permissions, and provides
 * secure key retrieval for cryptographic operations.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_KEYSTORE_H
#define EHSM_KEYSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

/**
 * @brief Initialize the keystore.
 * 
 * Clears all key slots and prepares for operation.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_init(void);

/**
 * @brief Deinitialize the keystore.
 * 
 * Securely erases all keys.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_deinit(void);

/* ============================================================================
 * KEY OPERATIONS
 * ============================================================================
 */

/**
 * @brief Import a key into a key slot.
 * 
 * @param[in] keySlotId Target slot ID (0 to EHSM_MAX_KEY_SLOTS-1).
 * @param[in] algorithm Algorithm associated with the key.
 * @param[in] keyData Pointer to key data.
 * @param[in] keySize Size of key data in bytes.
 * @param[in] permissions Access permission flags.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_importKey(
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* keyData,
    size_t keySize,
    uint8_t permissions
);

/**
 * @brief Delete a key from a slot.
 * 
 * Securely erases the key data.
 * 
 * @param[in] keySlotId Slot ID to clear.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_deleteKey(uint8_t keySlotId);

/**
 * @brief Get a key from a slot (for crypto operations).
 * 
 * @param[in] keySlotId Slot ID to read from.
 * @param[out] key Pointer to structure to fill with key data.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_getKey(uint8_t keySlotId, ehsm_key_t* key);

/**
 * @brief Get information about a key slot.
 * 
 * @param[in] keySlotId Slot ID to query.
 * @param[out] keySlot Pointer to structure to fill with info.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_getSlotInfo(uint8_t keySlotId, ehsm_keySlot_t* keySlot);

/**
 * @brief Clear all keys from the keystore.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_keystore_clearAll(void);

/**
 * @brief Check if a key slot is occupied.
 * 
 * @param[in] keySlotId Slot ID to check.
 * @return true if occupied, false if empty.
 */
bool ehsm_keystore_isSlotOccupied(uint8_t keySlotId);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_KEYSTORE_H */
