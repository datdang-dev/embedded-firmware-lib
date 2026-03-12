/**
 * @file ehsm_crypto.h
 * @brief Crypto Service Layer: Cryptographic primitives interface.
 * 
 * Provides low-level cryptographic operations (encrypt, decrypt, hash, etc.)
 * This layer interfaces with hardware crypto accelerators when available.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_CRYPTO_H
#define EHSM_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"

/* ============================================================================
 * CRYPTO OPERATIONS
 * ============================================================================
 */

/**
 * @brief Initialize the crypto service.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_init(void);

/**
 * @brief Deinitialize the crypto service.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_deinit(void);

/**
 * @brief Perform symmetric encryption.
 * 
 * @param[in] algorithm Encryption algorithm.
 * @param[in] keySlotId Key slot containing the encryption key.
 * @param[in] input Input data buffer (plaintext).
 * @param[out] output Output data buffer (ciphertext).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_encrypt(
    ehsm_algorithm_t algorithm,
    uint8_t keySlotId,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
);

/**
 * @brief Perform symmetric decryption.
 * 
 * @param[in] algorithm Decryption algorithm.
 * @param[in] keySlotId Key slot containing the decryption key.
 * @param[in] input Input data buffer (ciphertext).
 * @param[out] output Output data buffer (plaintext).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_decrypt(
    ehsm_algorithm_t algorithm,
    uint8_t keySlotId,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
);

/**
 * @brief Compute hash of input data.
 * 
 * @param[in] algorithm Hash algorithm (e.g., SHA-256).
 * @param[in] input Input data buffer.
 * @param[out] output Output buffer for hash digest.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_hash(
    ehsm_algorithm_t algorithm,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
);

/**
 * @brief Generate a random key.
 * 
 * @param[in] algorithm Algorithm type for the key.
 * @param[out] output Output buffer for generated key.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_crypto_generateKey(
    ehsm_algorithm_t algorithm,
    ehsm_buffer_t* output
);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_CRYPTO_H */
