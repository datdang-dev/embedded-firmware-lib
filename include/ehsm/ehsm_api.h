/**
 * @file ehsm_api.h
 * @brief Public API layer for the Embedded Hardware Security Module.
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

#ifndef EHSM_API_H
#define EHSM_API_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"

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
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @pre None
 * @post HSM is ready to accept commands
 * 
 * @example
 * ```c
 * ehsm_status_t status = ehsm_api_init();
 * if (status != EHSM_STATUS_OK) {
 *     // Handle initialization error
 * }
 * ```
 */
ehsm_status_t ehsm_api_init(void);

/**
 * @brief Deinitialize the HSM subsystem.
 * 
 * Cleans up all resources, clears sensitive data from memory,
 * and shuts down the HSM gracefully.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @pre ehsm_api_init() has been called
 * @post HSM is shut down and must be reinitialized to use
 */
ehsm_status_t ehsm_api_deinit(void);

/**
 * @brief Check if the HSM is initialized and ready.
 * 
 * @return true if HSM is ready, false otherwise.
 */
bool ehsm_api_isReady(void);

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
 * @param[out] sessionId Pointer to store the new session ID.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Session created successfully
 * @retval EHSM_STATUS_ERR_SESSION_INVALID No sessions available
 * @retval EHSM_STATUS_ERR_NOT_INITIALIZED HSM not initialized
 */
ehsm_status_t ehsm_api_createSession(ehsm_sessionId_t* sessionId);

/**
 * @brief Close an existing HSM session.
 * 
 * Releases all resources associated with the session and
 * invalidates the session ID.
 * 
 * @param sessionId Session ID to close.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Session closed successfully
 * @retval EHSM_STATUS_ERR_SESSION_INVALID Session ID is invalid
 */
ehsm_status_t ehsm_api_closeSession(ehsm_sessionId_t sessionId);

/**
 * @brief Validate a session ID.
 * 
 * @param sessionId Session ID to validate.
 * @return true if session is valid, false otherwise.
 */
bool ehsm_api_isSessionValid(ehsm_sessionId_t sessionId);

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
 * @param[in] sessionId Valid session ID.
 * @param[in] keySlotId ID of the key slot containing the encryption key.
 * @param[in] algorithm Encryption algorithm to use.
 * @param[in] input Pointer to input (plaintext) data.
 * @param[in] inputLen Length of input data in bytes.
 * @param[out] output Pointer to output (ciphertext) buffer.
 * @param[in,out] outputLen Size of output buffer (in/out: actual ciphertext length).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Encryption successful
 * @retval EHSM_STATUS_ERR_INVALID_PARAM Invalid parameters
 * @retval EHSM_STATUS_ERR_INVALID_KEY_ID Key slot not valid for encryption
 * @retval EHSM_STATUS_ERR_INSUFFICIENT_BUFFER Output buffer too small
 * @retval EHSM_STATUS_ERR_CRYPTO_FAILED Encryption operation failed
 * @retval EHSM_STATUS_ERR_SESSION_INVALID Session is not valid
 * 
 * @pre Session must be valid and key slot must have ENCRYPT permission
 * @post Output buffer contains ciphertext, outputLen updated
 * 
 * @example
 * ```c
 * uint8_t plaintext[] = "Secret message";
 * uint8_t ciphertext[256];
 * size_t ciphertextLen = sizeof(ciphertext);
 * 
 * ehsm_status_t status = ehsm_api_encrypt(
 *     sessionId,
 *     keySlotId,
 *     EHSM_ALGORITHM_AES_256,
 *     plaintext, sizeof(plaintext),
 *     ciphertext, &ciphertextLen
 * );
 * 
 * if (status == EHSM_STATUS_OK) {
 *     // Use ciphertext
 * }
 * ```
 */
ehsm_status_t ehsm_api_encrypt(
    ehsm_sessionId_t sessionId,
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t* outputLen
);

/**
 * @brief Decrypt data using the specified key and algorithm.
 * 
 * This is the primary decryption API function. It dispatches the
 * decryption job through the middleware to the crypto service.
 * 
 * @param[in] sessionId Valid session ID.
 * @param[in] keySlotId ID of the key slot containing the decryption key.
 * @param[in] algorithm Decryption algorithm to use.
 * @param[in] input Pointer to input (ciphertext) data.
 * @param[in] inputLen Length of input data in bytes.
 * @param[out] output Pointer to output (plaintext) buffer.
 * @param[in,out] outputLen Size of output buffer (in/out: actual plaintext length).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Decryption successful
 * @retval EHSM_STATUS_ERR_INVALID_PARAM Invalid parameters
 * @retval EHSM_STATUS_ERR_INVALID_KEY_ID Key slot not valid for decryption
 * @retval EHSM_STATUS_ERR_INSUFFICIENT_BUFFER Output buffer too small
 * @retval EHSM_STATUS_ERR_CRYPTO_FAILED Decryption operation failed
 * @retval EHSM_STATUS_ERR_SESSION_INVALID Session is not valid
 * 
 * @pre Session must be valid and key slot must have DECRYPT permission
 * @post Output buffer contains plaintext, outputLen updated
 */
ehsm_status_t ehsm_api_decrypt(
    ehsm_sessionId_t sessionId,
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t* outputLen
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
 * @param[in] keySlotId Target key slot ID (0 to EHSM_MAX_KEY_SLOTS-1).
 * @param[in] algorithm Algorithm associated with the key.
 * @param[in] keyData Pointer to key data.
 * @param[in] keySize Size of key data in bytes.
 * @param[in] permissions Access permission flags.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Key imported successfully
 * @retval EHSM_STATUS_ERR_INVALID_PARAM Invalid parameters
 * @retval EHSM_STATUS_ERR_KEY_SLOT_FULL Slot already occupied
 * @retval EHSM_STATUS_ERR_INVALID_KEY_ID Invalid slot ID
 */
ehsm_status_t ehsm_api_importKey(
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* keyData,
    size_t keySize,
    uint8_t permissions
);

/**
 * @brief Delete a key from a key slot.
 * 
 * Securely erases the key from the specified slot.
 * 
 * @param[in] keySlotId Key slot ID to clear.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @retval EHSM_STATUS_OK Key deleted successfully
 * @retval EHSM_STATUS_ERR_INVALID_KEY_ID Invalid slot ID
 * @retval EHSM_STATUS_ERR_INVALID_PARAM Slot is empty
 */
ehsm_status_t ehsm_api_deleteKey(uint8_t keySlotId);

/**
 * @brief Get information about a key slot.
 * 
 * @param[in] keySlotId Key slot ID to query.
 * @param[out] keySlot Pointer to structure to fill with info.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_api_getKeySlotInfo(uint8_t keySlotId, ehsm_keySlot_t* keySlot);

/**
 * @brief Clear all keys from the keystore.
 * 
 * Securely erases all keys. Use with caution.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_api_clearAllKeys(void);

/* ============================================================================
 * DIAGNOSTICS
 * ============================================================================
 */

/**
 * @brief Get HSM firmware version.
 * 
 * @return Version string (static, do not free).
 */
const char* ehsm_api_getVersion(void);

/**
 * @brief Get the last error message (for debugging).
 * 
 * @param[in] status Status code to get message for.
 * @return Human-readable error message.
 */
const char* ehsm_api_getErrorString(ehsm_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_API_H */
