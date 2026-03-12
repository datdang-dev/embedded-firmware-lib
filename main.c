/**
 * @file main.c
 * @brief Main entry point for Embedded HSM firmware on HOST platform.
 * 
 * This file demonstrates the complete HSM encryption flow:
 * 1. Initialize HSM subsystem
 * 2. Create a session
 * 3. Import an encryption key
 * 4. Encrypt data
 * 5. Decrypt data
 * 6. Cleanup and shutdown
 * 
 * Build: cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build
 * Run:   ./build/ehsm_host
 * Debug: gdb ./build/ehsm_host
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_api.h"
#include "ehsm/ehsm_types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

static const uint8_t TEST_KEY[32] = {
    0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U,
    0x08U, 0x09U, 0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x0FU,
    0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U,
    0x18U, 0x19U, 0x1AU, 0x1BU, 0x1CU, 0x1DU, 0x1EU, 0x1FU
};

static const char TEST_PLAINTEXT[] = "Hello, Embedded HSM! This is a secret message.";

#define TEST_KEY_SLOT_ID        (0U)
#define OUTPUT_BUFFER_SIZE      (256U)

/* ============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * ============================================================================
 */

static ehsm_status_t runEncryptDecryptDemo(ehsm_sessionId_t sessionId);
static void printHsmInfo(void);

/* ============================================================================
 * MAIN FUNCTION
 * ============================================================================
 */

int main(void)
{
    ehsm_status_t status;
    ehsm_sessionId_t sessionId;
    
    printf("=================================================\n");
    printf("  Embedded HSM Firmware - HOST Platform Demo\n");
    printf("  Naming Convention: ehsm_<module>_camelCase()\n");
    printf("=================================================\n\n");
    
    /* Initialize HSM */
    printf("[MAIN] Initializing HSM subsystem...\n");
    status = ehsm_api_init();
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] ERROR: ehsm_api_init failed: %s\n", 
               ehsm_api_getErrorString(status));
        return EXIT_FAILURE;
    }
    printf("[MAIN] HSM initialized successfully (version: %s)\n\n", 
           ehsm_api_getVersion());
    
    printHsmInfo();
    
    /* Create session */
    printf("[MAIN] Creating HSM session...\n");
    status = ehsm_api_createSession(&sessionId);
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] ERROR: ehsm_api_createSession failed: %s\n",
               ehsm_api_getErrorString(status));
        ehsm_api_deinit();
        return EXIT_FAILURE;
    }
    printf("[MAIN] Session created: ID=0x%08lX\n\n", (unsigned long)sessionId);
    
    /* Import key */
    printf("[MAIN] Importing AES-256 key into slot %d...\n", TEST_KEY_SLOT_ID);
    status = ehsm_api_importKey(
        TEST_KEY_SLOT_ID,
        EHSM_ALGORITHM_AES_256,
        TEST_KEY,
        sizeof(TEST_KEY),
        EHSM_KEY_PERM_ENCRYPT | EHSM_KEY_PERM_DECRYPT
    );
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] ERROR: ehsm_api_importKey failed: %s\n",
               ehsm_api_getErrorString(status));
        ehsm_api_closeSession(sessionId);
        ehsm_api_deinit();
        return EXIT_FAILURE;
    }
    printf("[MAIN] Key imported successfully\n\n");
    
    /* Run demo */
    printf("[MAIN] Running encryption/decryption demonstration...\n");
    printf("-------------------------------------------------\n");
    status = runEncryptDecryptDemo(sessionId);
    printf("-------------------------------------------------\n\n");
    
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] ERROR: Demo failed: %s\n",
               ehsm_api_getErrorString(status));
        ehsm_api_closeSession(sessionId);
        ehsm_api_deinit();
        return EXIT_FAILURE;
    }
    
    /* Cleanup */
    printf("[MAIN] Cleaning up...\n");
    
    printf("[MAIN] Deleting key from slot %d...\n", TEST_KEY_SLOT_ID);
    status = ehsm_api_deleteKey(TEST_KEY_SLOT_ID);
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] WARNING: ehsm_api_deleteKey failed: %s\n",
               ehsm_api_getErrorString(status));
    }
    
    printf("[MAIN] Closing session...\n");
    status = ehsm_api_closeSession(sessionId);
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] WARNING: ehsm_api_closeSession failed: %s\n",
               ehsm_api_getErrorString(status));
    }
    
    printf("[MAIN] Deinitializing HSM...\n");
    status = ehsm_api_deinit();
    if (status != EHSM_STATUS_OK)
    {
        printf("[MAIN] WARNING: ehsm_api_deinit failed: %s\n",
               ehsm_api_getErrorString(status));
    }
    
    printf("\n=================================================\n");
    printf("  Demo completed successfully!\n");
    printf("=================================================\n");
    
    return EXIT_SUCCESS;
}

/* ============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */

static ehsm_status_t runEncryptDecryptDemo(ehsm_sessionId_t sessionId)
{
    ehsm_status_t status;
    uint8_t ciphertext[OUTPUT_BUFFER_SIZE];
    uint8_t decrypted[OUTPUT_BUFFER_SIZE];
    size_t ciphertextLen = sizeof(ciphertext);
    size_t decryptedLen = sizeof(decrypted);
    
    const uint8_t* plaintext = (const uint8_t*)TEST_PLAINTEXT;
    size_t plaintextLen = strlen(TEST_PLAINTEXT);
    
    printf("\n[DEMO] Original plaintext:\n");
    printf("       \"%s\"\n", TEST_PLAINTEXT);
    printf("       Length: %zu bytes\n", plaintextLen);
    
    /* Encrypt */
    printf("\n[DEMO] Calling ehsm_api_encrypt()...\n");
    printf("       Session: 0x%08lX, Key Slot: %d, Algorithm: AES-256\n",
           (unsigned long)sessionId, TEST_KEY_SLOT_ID);
    
    status = ehsm_api_encrypt(
        sessionId,
        TEST_KEY_SLOT_ID,
        EHSM_ALGORITHM_AES_256,
        plaintext,
        plaintextLen,
        ciphertext,
        &ciphertextLen
    );
    
    if (status != EHSM_STATUS_OK)
    {
        printf("[DEMO] ERROR: ehsm_api_encrypt failed: %s\n",
               ehsm_api_getErrorString(status));
        return status;
    }
    
    printf("[DEMO] Encryption successful!\n");
    printf("       Ciphertext length: %zu bytes\n", ciphertextLen);
    
    printf("       Ciphertext: ");
    for (size_t i = 0U; i < ciphertextLen; i++)
    {
        printf("%02X", ciphertext[i]);
        if ((i + 1U) % 8U == 0U)
        {
            printf(" ");
        }
    }
    printf("\n");
    
    /* Decrypt */
    printf("\n[DEMO] Calling ehsm_api_decrypt()...\n");
    
    status = ehsm_api_decrypt(
        sessionId,
        TEST_KEY_SLOT_ID,
        EHSM_ALGORITHM_AES_256,
        ciphertext,
        ciphertextLen,
        decrypted,
        &decryptedLen
    );
    
    if (status != EHSM_STATUS_OK)
    {
        printf("[DEMO] ERROR: ehsm_api_decrypt failed: %s\n",
               ehsm_api_getErrorString(status));
        return status;
    }
    
    printf("[DEMO] Decryption successful!\n");
    printf("       Decrypted length: %zu bytes\n", decryptedLen);
    
    if (decryptedLen < sizeof(decrypted))
    {
        decrypted[decryptedLen] = '\0';
    }
    else
    {
        decrypted[sizeof(decrypted) - 1] = '\0';
    }
    
    printf("       Decrypted text: \"%s\"\n", (char*)decrypted);
    
    /* Verify */
    printf("\n[DEMO] Verifying decryption...\n");
    
    if (decryptedLen != plaintextLen)
    {
        printf("[DEMO] ERROR: Length mismatch!\n");
        return EHSM_STATUS_ERR_CRYPTO_FAILED;
    }
    
    if (memcmp(plaintext, decrypted, plaintextLen) != 0)
    {
        printf("[DEMO] ERROR: Data mismatch!\n");
        return EHSM_STATUS_ERR_CRYPTO_FAILED;
    }
    
    printf("[DEMO] Verification PASSED - decrypted data matches original!\n");
    
    return EHSM_STATUS_OK;
}

static void printHsmInfo(void)
{
    printf("HSM Configuration:\n");
    printf("  - Max Key Slots:     %d\n", EHSM_MAX_KEY_SLOTS);
    printf("  - Max Job Queue:     %d\n", EHSM_MAX_JOB_QUEUE_DEPTH);
    printf("  - Max Sessions:      %d\n", EHSM_MAX_SESSIONS);
    printf("  - Max Key Size:      %d bytes\n", EHSM_MAX_KEY_SIZE);
    printf("  - Max Block Size:    %d bytes\n", EHSM_MAX_BLOCK_SIZE);
    printf("\n");
}
