/**
 * @file hsm_keystore.h
 * @brief Keystore Manager: Secure key storage and access control.
 * 
 * Manages key slots, enforces access permissions, and provides
 * secure key retrieval for cryptographic operations.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_KEYSTORE_H
#define HSM_KEYSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

/**
 * @brief Initialize the keystore.
 * 
 * Clears all key slots and prepares for operation.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_Init(void);

/**
 * @brief Deinitialize the keystore.
 * 
 * Securely erases all keys.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_Deinit(void);

/* ============================================================================
 * KEY OPERATIONS
 * ============================================================================
 */

/**
 * @brief Import a key into a key slot.
 * 
 * @param key_slot_id Target slot ID (0 to HSM_MAX_KEY_SLOTS-1).
 * @param algorithm Algorithm associated with the key.
 * @param key_data Pointer to key data.
 * @param key_size Size of key data in bytes.
 * @param permissions Access permission flags.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_ImportKey(
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* key_data,
    size_t key_size,
    uint8_t permissions
);

/**
 * @brief Delete a key from a slot.
 * 
 * Securely erases the key data.
 * 
 * @param key_slot_id Slot ID to clear.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_DeleteKey(uint8_t key_slot_id);

/**
 * @brief Get a key from a slot (for crypto operations).
 * 
 * @param key_slot_id Slot ID to read from.
 * @param[out] key Pointer to structure to fill with key data.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_GetKey(uint8_t key_slot_id, Hsm_Key_t* key);

/**
 * @brief Get information about a key slot.
 * 
 * @param key_slot_id Slot ID to query.
 * @param[out] key_slot Pointer to structure to fill with info.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_GetSlotInfo(uint8_t key_slot_id, Hsm_KeySlot_t* key_slot);

/**
 * @brief Clear all keys from the keystore.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Keystore_ClearAll(void);

/**
 * @brief Check if a key slot is occupied.
 * 
 * @param key_slot_id Slot ID to check.
 * @return true if occupied, false if empty.
 */
bool Hsm_Keystore_IsSlotOccupied(uint8_t key_slot_id);

#ifdef __cplusplus
}
#endif

#endif /* HSM_KEYSTORE_H */
