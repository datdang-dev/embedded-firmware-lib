/**
 * @file hsm_crypto_service.c
 * @brief Implementation of cryptographic primitives.
 * 
 * This file contains mocked implementations of crypto operations
 * for demonstration purposes. In production, these would interface
 * with hardware crypto accelerators.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_crypto_service.h"
#include "hsm/hsm_keystore.h"
#include "hsm/mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** AES block size in bytes */
#define AES_BLOCK_SIZE          (16U)

/** SHA-256 digest size in bytes */
#define SHA256_DIGEST_SIZE      (32U)

/** XOR-based mock encryption (for demonstration only!) */
#define MOCK_ENCRYPT_KEY        (0x5AU)

/* ============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * ============================================================================
 */

/**
 * @brief Mock AES-128 encryption (XOR-based for demo).
 */
static void Mock_Aes_Encrypt(const uint8_t* input, uint8_t* output, size_t len);

/**
 * @brief Mock AES-128 decryption (XOR-based for demo).
 */
static void Mock_Aes_Decrypt(const uint8_t* input, uint8_t* output, size_t len);

/**
 * @brief Mock SHA-256 hash (simple checksum for demo).
 */
static void Mock_Sha256(const uint8_t* input, size_t len, uint8_t* output);

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

Hsm_Status_t Hsm_CryptoService_Init(void)
{
    /* In production: Initialize hardware crypto accelerator */
    return HSM_OK;
}

Hsm_Status_t Hsm_CryptoService_Deinit(void)
{
    /* In production: Deinitialize hardware crypto accelerator */
    return HSM_OK;
}

/* ============================================================================
 * CRYPTO OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_CryptoService_Encrypt(
    Hsm_Algorithm_t algorithm,
    uint8_t key_slot_id,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
)
{
    Hsm_Status_t status;
    Hsm_Key_t key;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    if ((input->data == NULL) || (output->data == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check buffer sizes */
    if (output->size < input->length)
    {
        return HSM_ERR_INSUFFICIENT_BUFFER;
    }
    
    /* Retrieve key from keystore */
    status = Hsm_Keystore_GetKey(key_slot_id, &key);
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Log encryption start via UART */
    Mcal_Uart_Log("[CRYPTO] Encrypt started: algo=%d, key_slot=%d, len=%zu",
                  algorithm, key_slot_id, input->length);
    
    /* Perform encryption based on algorithm */
    switch (algorithm)
    {
        case HSM_ALGO_AES_128:
        case HSM_ALGO_AES_256:
            /* Mock AES encryption (XOR-based for demo) */
            Mock_Aes_Encrypt(input->data, output->data, input->length);
            output->length = input->length;
            status = HSM_OK;
            break;
            
        case HSM_ALGO_RSA_2048:
        case HSM_ALGO_RSA_4096:
        case HSM_ALGO_ECC_P256:
            /* Asymmetric encryption not implemented in this PoC */
            status = HSM_ERR_UNSUPPORTED;
            break;
            
        default:
            status = HSM_ERR_INVALID_PARAM;
            break;
    }
    
    /* Log encryption result via UART */
    if (status == HSM_OK)
    {
        Mcal_Uart_Log("[CRYPTO] Encrypt completed: output_len=%zu", output->length);
        
        /* Trace first few bytes of ciphertext for debugging */
        Mcal_Uart_Log("[CRYPTO] Ciphertext preview: %02X %02X %02X %02X",
                      output->data[0],
                      output->data[1],
                      output->data[2],
                      output->data[3]);
    }
    else
    {
        Mcal_Uart_Log("[CRYPTO] Encrypt failed: status=%d", status);
    }
    
    return status;
}

Hsm_Status_t Hsm_CryptoService_Decrypt(
    Hsm_Algorithm_t algorithm,
    uint8_t key_slot_id,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
)
{
    Hsm_Status_t status;
    Hsm_Key_t key;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    if ((input->data == NULL) || (output->data == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check buffer sizes */
    if (output->size < input->length)
    {
        return HSM_ERR_INSUFFICIENT_BUFFER;
    }
    
    /* Retrieve key from keystore */
    status = Hsm_Keystore_GetKey(key_slot_id, &key);
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Log decryption start via UART */
    Mcal_Uart_Log("[CRYPTO] Decrypt started: algo=%d, key_slot=%d, len=%zu",
                  algorithm, key_slot_id, input->length);
    
    /* Perform decryption based on algorithm */
    switch (algorithm)
    {
        case HSM_ALGO_AES_128:
        case HSM_ALGO_AES_256:
            /* Mock AES decryption (XOR-based for demo) */
            Mock_Aes_Decrypt(input->data, output->data, input->length);
            output->length = input->length;
            status = HSM_OK;
            break;
            
        case HSM_ALGO_RSA_2048:
        case HSM_ALGO_RSA_4096:
        case HSM_ALGO_ECC_P256:
            /* Asymmetric decryption not implemented in this PoC */
            status = HSM_ERR_UNSUPPORTED;
            break;
            
        default:
            status = HSM_ERR_INVALID_PARAM;
            break;
    }
    
    /* Log decryption result via UART */
    if (status == HSM_OK)
    {
        Mcal_Uart_Log("[CRYPTO] Decrypt completed: output_len=%zu", output->length);
        
        /* Trace first few bytes of plaintext for debugging */
        Mcal_Uart_Log("[CRYPTO] Plaintext preview: %02X %02X %02X %02X",
                      output->data[0],
                      output->data[1],
                      output->data[2],
                      output->data[3]);
    }
    else
    {
        Mcal_Uart_Log("[CRYPTO] Decrypt failed: status=%d", status);
    }
    
    return status;
}

Hsm_Status_t Hsm_CryptoService_Hash(
    Hsm_Algorithm_t algorithm,
    const Hsm_Buffer_t* input,
    Hsm_Buffer_t* output
)
{
    /* Validate parameters */
    if ((input == NULL) || (output == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Log hash operation via UART */
    Mcal_Uart_Log("[CRYPTO] Hash started: algo=%d, len=%zu",
                  algorithm, input->length);
    
    switch (algorithm)
    {
        case HSM_ALGO_SHA_256:
            /* Check output buffer size */
            if (output->size < SHA256_DIGEST_SIZE)
            {
                return HSM_ERR_INSUFFICIENT_BUFFER;
            }
            
            /* Mock SHA-256 */
            Mock_Sha256(input->data, input->length, output->data);
            output->length = SHA256_DIGEST_SIZE;
            break;
            
        default:
            return HSM_ERR_UNSUPPORTED;
    }
    
    Mcal_Uart_Log("[CRYPTO] Hash completed");
    
    return HSM_OK;
}

Hsm_Status_t Hsm_CryptoService_GenerateKey(
    Hsm_Algorithm_t algorithm,
    Hsm_Buffer_t* output
)
{
    /* Validate parameters */
    if (output == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Determine key size based on algorithm */
    size_t key_size = 0U;
    switch (algorithm)
    {
        case HSM_ALGO_AES_128:
            key_size = 16U;
            break;
        case HSM_ALGO_AES_256:
            key_size = 32U;
            break;
        default:
            return HSM_ERR_UNSUPPORTED;
    }
    
    /* Check output buffer size */
    if (output->size < key_size)
    {
        return HSM_ERR_INSUFFICIENT_BUFFER;
    }
    
    /* Generate mock random key (in production, use hardware RNG) */
    for (size_t i = 0U; i < key_size; i++)
    {
        output->data[i] = (uint8_t)(0x10U + i);  /* Deterministic for testing */
    }
    output->length = key_size;
    
    Mcal_Uart_Log("[CRYPTO] Key generated: algo=%d, size=%zu", algorithm, key_size);
    
    return HSM_OK;
}

/* ============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */

static void Mock_Aes_Encrypt(const uint8_t* input, uint8_t* output, size_t len)
{
    /* 
     * WARNING: This is a MOCK implementation for demonstration only!
     * In production, use a real AES implementation with hardware acceleration.
     * 
     * This XOR-based "encryption" is NOT secure and should never be used
     * in production code.
     */
    for (size_t i = 0U; i < len; i++)
    {
        output[i] = input[i] ^ MOCK_ENCRYPT_KEY;
    }
}

static void Mock_Aes_Decrypt(const uint8_t* input, uint8_t* output, size_t len)
{
    /* 
     * WARNING: This is a MOCK implementation for demonstration only!
     * XOR is symmetric, so decryption is the same as encryption.
     */
    for (size_t i = 0U; i < len; i++)
    {
        output[i] = input[i] ^ MOCK_ENCRYPT_KEY;
    }
}

static void Mock_Sha256(const uint8_t* input, size_t len, uint8_t* output)
{
    /* 
     * WARNING: This is a MOCK implementation for demonstration only!
     * In production, use a real SHA-256 implementation.
     * 
     * This generates a simple checksum-based "hash".
     */
    uint32_t sum = 0U;
    
    /* Simple checksum */
    for (size_t i = 0U; i < len; i++)
    {
        sum += input[i];
        sum = (sum << 1) | (sum >> 31);  /* Rotate left */
    }
    
    /* Fill output with derived bytes */
    for (size_t i = 0U; i < SHA256_DIGEST_SIZE; i++)
    {
        output[i] = (uint8_t)((sum >> (i % 32)) ^ (uint8_t)i);
    }
}
