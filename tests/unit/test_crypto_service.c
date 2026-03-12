/**
 * @file test_crypto_service.c
 * @brief Unit tests for the Crypto Service layer.
 * 
 * Tests encryption, decryption, and hash operations.
 * Uses simple assertions for HOST platform testing.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "hsm/hsm_crypto_service.h"
#include "hsm/hsm_keystore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * TEST CONFIGURATION
 * ============================================================================
 */

#define TEST_KEY_SLOT       (0U)
#define TEST_BUFFER_SIZE    (256U)

/* Test key (AES-256) */
static const uint8_t TEST_KEY[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

/* ============================================================================
 * TEST MACROS
 * ============================================================================
 */

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("[FAIL] %s:%d: %s\n", __FILE__, __LINE__, message); \
            failures++; \
        } else { \
            printf("[PASS] %s\n", message); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

/* ============================================================================
 * TEST FUNCTIONS
 * ============================================================================
 */

static int test_crypto_init(void)
{
    printf("\n=== Test: Crypto Service Initialization ===\n");
    
    Hsm_Status_t status = Hsm_CryptoService_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Crypto service init should succeed");
    
    return 0;
}

static int test_crypto_encrypt_decrypt(void)
{
    printf("\n=== Test: Encrypt/Decrypt Round-Trip ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    
    /* Setup: Import test key */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Keystore init should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT,
        HSM_ALGO_AES_256,
        TEST_KEY,
        sizeof(TEST_KEY),
        HSM_KEY_PERM_ENCRYPT | HSM_KEY_PERM_DECRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key import should succeed");
    
    /* Prepare test data */
    uint8_t plaintext[] = "Hello, World! This is a test message for encryption.";
    uint8_t ciphertext[TEST_BUFFER_SIZE];
    uint8_t decrypted[TEST_BUFFER_SIZE];
    
    Hsm_Buffer_t input = {
        .data = plaintext,
        .size = sizeof(plaintext),
        .length = sizeof(plaintext) - 1  /* Exclude null terminator */
    };
    
    Hsm_Buffer_t output = {
        .data = ciphertext,
        .size = sizeof(ciphertext),
        .length = 0
    };
    
    Hsm_Buffer_t decrypt_output = {
        .data = decrypted,
        .size = sizeof(decrypted),
        .length = 0
    };
    
    /* Test encryption */
    printf("  Plaintext: \"%s\"\n", plaintext);
    status = Hsm_CryptoService_Encrypt(
        HSM_ALGO_AES_256,
        TEST_KEY_SLOT,
        &input,
        &output
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Encryption should succeed");
    TEST_ASSERT(output.length > 0, "Ciphertext length should be > 0");
    TEST_ASSERT(output.length == input.length, "Ciphertext length should match plaintext");
    
    /* Test decryption */
    Hsm_Buffer_t decrypt_input = {
        .data = ciphertext,
        .size = output.length,
        .length = output.length
    };
    
    status = Hsm_CryptoService_Decrypt(
        HSM_ALGO_AES_256,
        TEST_KEY_SLOT,
        &decrypt_input,
        &decrypt_output
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Decryption should succeed");
    TEST_ASSERT(decrypt_output.length == input.length, "Decrypted length should match original");
    
    /* Verify round-trip */
    decrypted[decrypt_output.length] = '\0';  /* Null-terminate */
    TEST_ASSERT(
        memcmp(plaintext, decrypted, input.length) == 0,
        "Decrypted data should match original plaintext"
    );
    
    printf("  Decrypted: \"%s\"\n", decrypted);
    
    /* Cleanup */
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT);
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_crypto_hash(void)
{
    printf("\n=== Test: Hash Operation ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    
    uint8_t input_data[] = "Test data for hashing";
    uint8_t hash_output[64];  /* SHA-256 produces 32 bytes */
    
    Hsm_Buffer_t input = {
        .data = input_data,
        .size = sizeof(input_data),
        .length = sizeof(input_data) - 1
    };
    
    Hsm_Buffer_t output = {
        .data = hash_output,
        .size = sizeof(hash_output),
        .length = 0
    };
    
    status = Hsm_CryptoService_Hash(
        HSM_ALGO_SHA_256,
        &input,
        &output
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Hash operation should succeed");
    TEST_ASSERT(output.length == 32, "SHA-256 should produce 32-byte digest");
    
    printf("  Input: \"%s\"\n", input_data);
    printf("  Hash: ");
    for (size_t i = 0; i < output.length; i++)
    {
        printf("%02X", hash_output[i]);
    }
    printf("\n");
    
    return failures;
}

static int test_crypto_invalid_params(void)
{
    printf("\n=== Test: Invalid Parameters ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    
    Hsm_Buffer_t input = { .data = NULL, .size = 0, .length = 0 };
    Hsm_Buffer_t output = { .data = NULL, .size = 0, .length = 0 };
    
    /* Test with NULL buffers */
    status = Hsm_CryptoService_Encrypt(HSM_ALGO_AES_256, 0, NULL, &output);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "NULL input should fail");
    
    status = Hsm_CryptoService_Encrypt(HSM_ALGO_AES_256, 0, &input, NULL);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "NULL output should fail");
    
    /* Test with invalid key slot */
    uint8_t data[16];
    input.data = data;
    input.size = sizeof(data);
    input.length = sizeof(data);
    output.data = data;
    output.size = sizeof(data);
    
    status = Hsm_CryptoService_Encrypt(HSM_ALGO_AES_256, 255, &input, &output);
    TEST_ASSERT(status != HSM_OK, "Invalid key slot should fail");
    
    return failures;
}

/* ============================================================================
 * MAIN
 * ============================================================================
 */

int main(void)
{
    int total_failures = 0;
    
    printf("==============================================\n");
    printf("  Crypto Service Unit Tests\n");
    printf("==============================================\n");
    
    total_failures += test_crypto_init();
    total_failures += test_crypto_encrypt_decrypt();
    total_failures += test_crypto_hash();
    total_failures += test_crypto_invalid_params();
    
    printf("\n==============================================\n");
    if (total_failures == 0)
    {
        printf("  All tests PASSED!\n");
        printf("==============================================\n");
        return 0;
    }
    else
    {
        printf("  %d test(s) FAILED!\n", total_failures);
        printf("==============================================\n");
        return 1;
    }
}
