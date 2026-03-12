/**
 * @file hsm_crypto_service.h
 * @brief Crypto Service Layer: Cryptographic primitives interface.
 * 
 * Provides low-level cryptographic operations (encrypt, decrypt, hash, etc.)
 * This layer interfaces with hardware crypto accelerators when available.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_CRYPTO_SERVICE_H
#define HSM_CRYPTO_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"

/* ============================================================================
 * CRYPTO OPERATIONS
 * ============================================================================
 */

/**
 * @brief Initialize the crypto service.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_Init(void);

/**
 * @brief Deinitialize the crypto service.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_Deinit(void);

/**
 * @brief Perform symmetric encryption.
 * 
 * @param algorithm Encryption algorithm.
 * @param key_slot_id Key slot containing the encryption key.
 * @param input Input data buffer (plaintext).
 * @param output Output data buffer (ciphertext).
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_Encrypt(
    Hsm_Algorithm_t algorithm,
    uint8_t key_slot_id,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
);

/**
 * @brief Perform symmetric decryption.
 * 
 * @param algorithm Decryption algorithm.
 * @param key_slot_id Key slot containing the decryption key.
 * @param input Input data buffer (ciphertext).
 * @param output Output data buffer (plaintext).
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_Decrypt(
    Hsm_Algorithm_t algorithm,
    uint8_t key_slot_id,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
);

/**
 * @brief Compute hash of input data.
 * 
 * @param algorithm Hash algorithm (e.g., SHA-256).
 * @param input Input data buffer.
 * @param output Output buffer for hash digest.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_Hash(
    Hsm_Algorithm_t algorithm,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
);

/**
 * @brief Generate a random key.
 * 
 * @param algorithm Algorithm type for the key.
 * @param output Output buffer for generated key.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_CryptoService_GenerateKey(
    Hsm_Algorithm_t algorithm,
    Hsm_Buffer_t* output
);

#ifdef __cplusplus
}
#endif

#endif /* HSM_CRYPTO_SERVICE_H */
