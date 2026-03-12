/**
 * @file ehsm_crypto.c
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
#include "ehsm/ehsm_crypto.h"
#include "ehsm/ehsm_keystore.h"
#include "ehsm/ehsm_mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

#define AES_BLOCK_SIZE          (16U)
#define SHA256_DIGEST_SIZE      (32U)
#define MOCK_ENCRYPT_KEY        (0x5AU)

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================
 */

static void mockAesEncrypt(const uint8_t* input, uint8_t* output, size_t len)
{
    for (size_t i = 0U; i < len; i++)
    {
        output[i] = input[i] ^ MOCK_ENCRYPT_KEY;
    }
}

static void mockAesDecrypt(const uint8_t* input, uint8_t* output, size_t len)
{
    for (size_t i = 0U; i < len; i++)
    {
        output[i] = input[i] ^ MOCK_ENCRYPT_KEY;
    }
}

static void mockSha256(const uint8_t* input, size_t len, uint8_t* output)
{
    uint32_t sum = 0U;
    
    for (size_t i = 0U; i < len; i++)
    {
        sum += input[i];
        sum = (sum << 1) | (sum >> 31);
    }
    
    for (size_t i = 0U; i < SHA256_DIGEST_SIZE; i++)
    {
        output[i] = (uint8_t)((sum >> (i % 32)) ^ (uint8_t)i);
    }
}

/* ============================================================================
 * PUBLIC IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_crypto_init(void)
{
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_crypto_deinit(void)
{
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_crypto_encrypt(
    ehsm_algorithm_t algorithm,
    uint8_t keySlotId,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
)
{
    ehsm_status_t status;
    ehsm_key_t key;
    
    if ((input == NULL) || (output == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if ((input->data == NULL) || (output->data == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (output->size < input->length)
    {
        return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER;
    }
    
    status = ehsm_keystore_getKey(keySlotId, &key);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    ehsm_mcal_uartLog("[CRYPTO] Encrypt started: algo=%d, slot=%d, len=%zu",
                      algorithm, keySlotId, input->length);
    
    switch (algorithm)
    {
        case EHSM_ALGORITHM_AES_128:
        case EHSM_ALGORITHM_AES_256:
            mockAesEncrypt(input->data, output->data, input->length);
            output->length = input->length;
            status = EHSM_STATUS_OK;
            break;
            
        default:
            status = EHSM_STATUS_ERR_UNSUPPORTED;
            break;
    }
    
    if (status == EHSM_STATUS_OK)
    {
        ehsm_mcal_uartLog("[CRYPTO] Encrypt completed: output_len=%zu", output->length);
        ehsm_mcal_uartLog("[CRYPTO] Ciphertext: %02X %02X %02X %02X",
                          output->data[0], output->data[1],
                          output->data[2], output->data[3]);
    }
    
    return status;
}

ehsm_status_t ehsm_crypto_decrypt(
    ehsm_algorithm_t algorithm,
    uint8_t keySlotId,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
)
{
    ehsm_status_t status;
    ehsm_key_t key;
    
    if ((input == NULL) || (output == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if ((input->data == NULL) || (output->data == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (output->size < input->length)
    {
        return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER;
    }
    
    status = ehsm_keystore_getKey(keySlotId, &key);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    ehsm_mcal_uartLog("[CRYPTO] Decrypt started: algo=%d, slot=%d, len=%zu",
                      algorithm, keySlotId, input->length);
    
    switch (algorithm)
    {
        case EHSM_ALGORITHM_AES_128:
        case EHSM_ALGORITHM_AES_256:
            mockAesDecrypt(input->data, output->data, input->length);
            output->length = input->length;
            status = EHSM_STATUS_OK;
            break;
            
        default:
            status = EHSM_STATUS_ERR_UNSUPPORTED;
            break;
    }
    
    if (status == EHSM_STATUS_OK)
    {
        ehsm_mcal_uartLog("[CRYPTO] Decrypt completed: output_len=%zu", output->length);
        ehsm_mcal_uartLog("[CRYPTO] Plaintext: %02X %02X %02X %02X",
                          output->data[0], output->data[1],
                          output->data[2], output->data[3]);
    }
    
    return status;
}

ehsm_status_t ehsm_crypto_hash(
    ehsm_algorithm_t algorithm,
    const ehsm_buffer_t* input,
    ehsm_buffer_t* output
)
{
    if ((input == NULL) || (output == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    ehsm_mcal_uartLog("[CRYPTO] Hash started: algo=%d, len=%zu",
                      algorithm, input->length);
    
    switch (algorithm)
    {
        case EHSM_ALGORITHM_SHA_256:
            if (output->size < SHA256_DIGEST_SIZE)
            {
                return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER;
            }
            
            mockSha256(input->data, input->length, output->data);
            output->length = SHA256_DIGEST_SIZE;
            break;
            
        default:
            return EHSM_STATUS_ERR_UNSUPPORTED;
    }
    
    ehsm_mcal_uartLog("[CRYPTO] Hash completed");
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_crypto_generateKey(
    ehsm_algorithm_t algorithm,
    ehsm_buffer_t* output
)
{
    if (output == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    size_t keySize = 0U;
    switch (algorithm)
    {
        case EHSM_ALGORITHM_AES_128:
            keySize = 16U;
            break;
        case EHSM_ALGORITHM_AES_256:
            keySize = 32U;
            break;
        default:
            return EHSM_STATUS_ERR_UNSUPPORTED;
    }
    
    if (output->size < keySize)
    {
        return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER;
    }
    
    for (size_t i = 0U; i < keySize; i++)
    {
        output->data[i] = (uint8_t)(0x10U + i);
    }
    output->length = keySize;
    
    ehsm_mcal_uartLog("[CRYPTO] Key generated: algo=%d, size=%zu", algorithm, keySize);
    return EHSM_STATUS_OK;
}
