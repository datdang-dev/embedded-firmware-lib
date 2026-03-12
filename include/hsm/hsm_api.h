/**
 * @file hsm_api.h
 * @brief Public API layer for the Hardware Security Module.
 * 
 * This header provides the highest-level interface for user applications
 * to interact with the HSM. Functions include encryption, decryption,
 * key management, and session control.
 * 
 * @note All API functions are thread-safe and reentrant.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_API_H
#define HSM_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"

/* ============================================================================
 * INITIALIZATION & SHUTDOWN
 * ============================================================================
 */

/**
 * @brief Initialize the HSM subsystem.
 * 
 * This function must be called before any other HSM API function.
 * It initializes all layers from MCAL up to the API layer.
 * 
 * @return HSM_OK on success, error code otherwise.
 * 
 * @pre None
 * @post HSM is ready to accept commands
 * 
 * @example
 * ```c
 * Hsm_Status_t status = Hsm_Init();
 * if (status != HSM_OK) {
 *     // Handle initialization error
 * }
 * ```
 */
Hsm_Status_t Hsm_Init(void);

/**
 * @brief Deinitialize the HSM subsystem.
 * 
 * Cleans up all resources, clears sensitive data from memory,
 * and shuts down the HSM gracefully.
 * 
 * @return HSM_OK on success, error code otherwise.
 * 
 * @pre Hsm_Init() has been called
 * @post HSM is shut down and must be reinitialized to use
 */
Hsm_Status_t Hsm_Deinit(void);

/**
 * @brief Check if the HSM is initialized and ready.
 * 
 * @return true if HSM is ready, false otherwise.
 */
bool Hsm_IsReady(void);

/* ============================================================================
 * SESSION MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Create a new HSM session.
 * 
 * Sessions are required for performing cryptographic operations.
 * Each session maintains its own context and job queue.
 * 
 * @param[out] session_id Pointer to store the new session ID.
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Session created successfully
 * @retval HSM_ERR_SESSION_INVALID No sessions available
 * @retval HSM_ERR_NOT_INITIALIZED HSM not initialized
 */
Hsm_Status_t Hsm_CreateSession(Hsm_SessionId_t* session_id);

/**
 * @brief Close an existing HSM session.
 * 
 * Releases all resources associated with the session and
 * invalidates the session ID.
 * 
 * @param session_id Session ID to close.
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Session closed successfully
 * @retval HSM_ERR_SESSION_INVALID Session ID is invalid
 */
Hsm_Status_t Hsm_CloseSession(Hsm_SessionId_t session_id);

/**
 * @brief Validate a session ID.
 * 
 * @param session_id Session ID to validate.
 * @return true if session is valid, false otherwise.
 */
bool Hsm_IsSessionValid(Hsm_SessionId_t session_id);

/* ============================================================================
 * CRYPTOGRAPHIC OPERATIONS
 * ============================================================================
 */

/**
 * @brief Encrypt data using the specified key and algorithm.
 * 
 * This is the primary encryption API function. It dispatches the
 * encryption job through the middleware to the crypto service.
 * 
 * @param session_id Valid session ID.
 * @param key_slot_id ID of the key slot containing the encryption key.
 * @param algorithm Encryption algorithm to use.
 * @param input Pointer to input (plaintext) data.
 * @param input_len Length of input data in bytes.
 * @param output Pointer to output (ciphertext) buffer.
 * @param output_len Size of output buffer (in/out: actual ciphertext length).
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Encryption successful
 * @retval HSM_ERR_INVALID_PARAM Invalid parameters
 * @retval HSM_ERR_INVALID_KEY_ID Key slot not valid for encryption
 * @retval HSM_ERR_INSUFFICIENT_BUFFER Output buffer too small
 * @retval HSM_ERR_CRYPTO_FAILED Encryption operation failed
 * @retval HSM_ERR_SESSION_INVALID Session is not valid
 * 
 * @pre Session must be valid and key slot must have ENCRYPT permission
 * @post Output buffer contains ciphertext, output_len updated
 * 
 * @example
 * ```c
 * uint8_t plaintext[] = "Secret message";
 * uint8_t ciphertext[256];
 * size_t ciphertext_len = sizeof(ciphertext);
 * 
 * Hsm_Status_t status = Hsm_Encrypt(
 *     session_id,
 *     key_slot_id,
 *     HSM_ALGO_AES_256,
 *     plaintext, sizeof(plaintext),
 *     ciphertext, &ciphertext_len
 * );
 * 
 * if (status == HSM_OK) {
 *     // Use ciphertext
 * }
 * ```
 */
Hsm_Status_t Hsm_Encrypt(
    Hsm_SessionId_t session_id,
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t* output_len
);

/**
 * @brief Decrypt data using the specified key and algorithm.
 * 
 * This is the primary decryption API function. It dispatches the
 * decryption job through the middleware to the crypto service.
 * 
 * @param session_id Valid session ID.
 * @param key_slot_id ID of the key slot containing the decryption key.
 * @param algorithm Decryption algorithm to use.
 * @param input Pointer to input (ciphertext) data.
 * @param input_len Length of input data in bytes.
 * @param output Pointer to output (plaintext) buffer.
 * @param output_len Size of output buffer (in/out: actual plaintext length).
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Decryption successful
 * @retval HSM_ERR_INVALID_PARAM Invalid parameters
 * @retval HSM_ERR_INVALID_KEY_ID Key slot not valid for decryption
 * @retval HSM_ERR_INSUFFICIENT_BUFFER Output buffer too small
 * @retval HSM_ERR_CRYPTO_FAILED Decryption operation failed
 * @retval HSM_ERR_SESSION_INVALID Session is not valid
 * 
 * @pre Session must be valid and key slot must have DECRYPT permission
 * @post Output buffer contains plaintext, output_len updated
 */
Hsm_Status_t Hsm_Decrypt(
    Hsm_SessionId_t session_id,
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t* output_len
);

/* ============================================================================
 * KEY MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Import a key into a key slot.
 * 
 * Securely imports a key into the specified slot with the given
 * permissions. The key data is copied to secure storage.
 * 
 * @param key_slot_id Target key slot ID (0 to HSM_MAX_KEY_SLOTS-1).
 * @param algorithm Algorithm associated with the key.
 * @param key_data Pointer to key data.
 * @param key_size Size of key data in bytes.
 * @param permissions Access permission flags.
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Key imported successfully
 * @retval HSM_ERR_INVALID_PARAM Invalid parameters
 * @retval HSM_ERR_KEY_SLOT_FULL Slot already occupied
 * @retval HSM_ERR_INVALID_KEY_ID Invalid slot ID
 */
Hsm_Status_t Hsm_ImportKey(
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* key_data,
    size_t key_size,
    uint8_t permissions
);

/**
 * @brief Delete a key from a key slot.
 * 
 * Securely erases the key from the specified slot.
 * 
 * @param key_slot_id Key slot ID to clear.
 * @return HSM_OK on success, error code otherwise.
 * 
 * @retval HSM_OK Key deleted successfully
 * @retval HSM_ERR_INVALID_KEY_ID Invalid slot ID
 * @retval HSM_ERR_INVALID_PARAM Slot is empty
 */
Hsm_Status_t Hsm_DeleteKey(uint8_t key_slot_id);

/**
 * @brief Get information about a key slot.
 * 
 * @param key_slot_id Key slot ID to query.
 * @param[out] key_slot Pointer to structure to fill with info.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_GetKeySlotInfo(uint8_t key_slot_id, Hsm_KeySlot_t* key_slot);

/**
 * @brief Clear all keys from the keystore.
 * 
 * Securely erases all keys. Use with caution.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_ClearAllKeys(void);

/* ============================================================================
 * DIAGNOSTICS
 * ============================================================================
 */

/**
 * @brief Get HSM firmware version.
 * 
 * @return Version string (static, do not free).
 */
const char* Hsm_GetVersion(void);

/**
 * @brief Get the last error message (for debugging).
 * 
 * @param status Status code to get message for.
 * @return Human-readable error message.
 */
const char* Hsm_GetErrorString(Hsm_Status_t status);

#ifdef __cplusplus
}
#endif

#endif /* HSM_API_H */
