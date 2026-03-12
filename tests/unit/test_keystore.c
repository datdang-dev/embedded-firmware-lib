/**
 * @file test_keystore.c
 * @brief Unit tests for the Keystore Manager.
 * 
 * Tests key import, deletion, slot management, and access control.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "hsm/hsm_keystore.h"
#include "hsm/hsm_types.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * TEST CONFIGURATION
 * ============================================================================
 */

#define TEST_KEY_SLOT_0     (0U)
#define TEST_KEY_SLOT_1     (1U)
#define INVALID_SLOT        (255U)

/* Test keys */
static const uint8_t TEST_KEY_AES128[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

static const uint8_t TEST_KEY_AES256[32] = {
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

static int test_keystore_init(void)
{
    printf("\n=== Test: Keystore Initialization ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    
    /* Test init */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Keystore init should succeed");
    
    /* Test double init (should be idempotent) */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Double init should succeed (idempotent)");
    
    /* Test deinit */
    status = Hsm_Keystore_Deinit();
    TEST_ASSERT_EQ(HSM_OK, status, "Keystore deinit should succeed");
    
    /* Test deinit without init */
    status = Hsm_Keystore_Deinit();
    TEST_ASSERT_EQ(HSM_ERR_NOT_INITIALIZED, status, "Deinit without init should fail");
    
    return failures;
}

static int test_key_import(void)
{
    printf("\n=== Test: Key Import ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    
    /* Initialize */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    /* Import AES-128 key */
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_128,
        TEST_KEY_AES128,
        sizeof(TEST_KEY_AES128),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "AES-128 key import should succeed");
    
    /* Import AES-256 key to different slot */
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_1,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT | HSM_KEY_PERM_DECRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "AES-256 key import should succeed");
    
    /* Try to import to same slot (should fail - slot occupied) */
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_128,
        TEST_KEY_AES128,
        sizeof(TEST_KEY_AES128),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_ERR_KEY_SLOT_FULL, status, "Import to occupied slot should fail");
    
    /* Test invalid slot ID */
    status = Hsm_Keystore_ImportKey(
        INVALID_SLOT,
        HSM_ALGO_AES_128,
        TEST_KEY_AES128,
        sizeof(TEST_KEY_AES128),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_ERR_INVALID_KEY_ID, status, "Invalid slot ID should fail");
    
    /* Test NULL key data */
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_128,
        NULL,
        0,
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "NULL key data should fail");
    
    /* Cleanup */
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_1);
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_key_slot_info(void)
{
    printf("\n=== Test: Key Slot Information ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    Hsm_KeySlot_t slot_info;
    
    /* Initialize and import key */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT | HSM_KEY_PERM_DECRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key import should succeed");
    
    /* Get slot info */
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_0, &slot_info);
    TEST_ASSERT_EQ(HSM_OK, status, "GetSlotInfo should succeed");
    
    /* Verify slot info */
    TEST_ASSERT_EQ(TEST_KEY_SLOT_0, slot_info.id, "Slot ID should match");
    TEST_ASSERT(slot_info.is_occupied == true, "Slot should be occupied");
    TEST_ASSERT_EQ(HSM_ALGO_AES_256, slot_info.algorithm, "Algorithm should match");
    TEST_ASSERT_EQ((uint8_t)(HSM_KEY_PERM_ENCRYPT | HSM_KEY_PERM_DECRYPT), 
                   slot_info.permissions, "Permissions should match");
    TEST_ASSERT_EQ(sizeof(TEST_KEY_AES256), slot_info.key_size, "Key size should match");
    
    /* Get info for empty slot */
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_1, &slot_info);
    TEST_ASSERT_EQ(HSM_OK, status, "GetSlotInfo for empty slot should succeed");
    TEST_ASSERT(slot_info.is_occupied == false, "Slot should be empty");
    
    /* Test NULL output parameter */
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_0, NULL);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "NULL output should fail");
    
    /* Test invalid slot ID */
    status = Hsm_Keystore_GetSlotInfo(INVALID_SLOT, &slot_info);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_KEY_ID, status, "Invalid slot ID should fail");
    
    /* Cleanup */
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_key_get(void)
{
    printf("\n=== Test: Key Retrieval ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    Hsm_Key_t retrieved_key;
    
    /* Initialize and import key */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key import should succeed");
    
    /* Retrieve key */
    status = Hsm_Keystore_GetKey(TEST_KEY_SLOT_0, &retrieved_key);
    TEST_ASSERT_EQ(HSM_OK, status, "GetKey should succeed");
    
    /* Verify key data */
    TEST_ASSERT_EQ(retrieved_key.size, sizeof(TEST_KEY_AES256), "Key size should match");
    TEST_ASSERT_EQ(retrieved_key.algorithm, HSM_ALGO_AES_256, "Algorithm should match");
    TEST_ASSERT(
        memcmp(retrieved_key.data, TEST_KEY_AES256, sizeof(TEST_KEY_AES256)) == 0,
        "Key data should match"
    );
    
    /* Test NULL output */
    status = Hsm_Keystore_GetKey(TEST_KEY_SLOT_0, NULL);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "NULL output should fail");
    
    /* Test empty slot */
    status = Hsm_Keystore_GetKey(TEST_KEY_SLOT_1, &retrieved_key);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_KEY_ID, status, "Empty slot should fail");
    
    /* Cleanup */
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_key_delete(void)
{
    printf("\n=== Test: Key Deletion ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    Hsm_KeySlot_t slot_info;
    
    /* Initialize and import key */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key import should succeed");
    
    /* Delete key */
    status = Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    TEST_ASSERT_EQ(HSM_OK, status, "Key deletion should succeed");
    
    /* Verify slot is empty */
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_0, &slot_info);
    TEST_ASSERT_EQ(HSM_OK, status, "GetSlotInfo should succeed");
    TEST_ASSERT(slot_info.is_occupied == false, "Slot should be empty after deletion");
    
    /* Delete already empty slot */
    status = Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_PARAM, status, "Delete empty slot should fail");
    
    /* Delete invalid slot */
    status = Hsm_Keystore_DeleteKey(INVALID_SLOT);
    TEST_ASSERT_EQ(HSM_ERR_INVALID_KEY_ID, status, "Invalid slot should fail");
    
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_clear_all_keys(void)
{
    printf("\n=== Test: Clear All Keys ===\n");
    
    int failures = 0;
    Hsm_Status_t status;
    Hsm_KeySlot_t slot_info;
    
    /* Initialize and import multiple keys */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_128,
        TEST_KEY_AES128,
        sizeof(TEST_KEY_AES128),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key 0 import should succeed");
    
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_1,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT | HSM_KEY_PERM_DECRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key 1 import should succeed");
    
    /* Clear all keys */
    status = Hsm_Keystore_ClearAll();
    TEST_ASSERT_EQ(HSM_OK, status, "ClearAll should succeed");
    
    /* Verify all slots are empty */
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_0, &slot_info);
    TEST_ASSERT(slot_info.is_occupied == false, "Slot 0 should be empty");
    
    status = Hsm_Keystore_GetSlotInfo(TEST_KEY_SLOT_1, &slot_info);
    TEST_ASSERT(slot_info.is_occupied == false, "Slot 1 should be empty");
    
    Hsm_Keystore_Deinit();
    
    return failures;
}

static int test_is_slot_occupied(void)
{
    printf("\n=== Test: Is Slot Occupied ===\n");
    
    int failures = 0;
    bool occupied;
    Hsm_Status_t status;
    
    /* Initialize */
    status = Hsm_Keystore_Init();
    TEST_ASSERT_EQ(HSM_OK, status, "Init should succeed");
    
    /* Check empty slot */
    occupied = Hsm_Keystore_IsSlotOccupied(TEST_KEY_SLOT_0);
    TEST_ASSERT(occupied == false, "Empty slot should return false");
    
    /* Import key and check */
    status = Hsm_Keystore_ImportKey(
        TEST_KEY_SLOT_0,
        HSM_ALGO_AES_256,
        TEST_KEY_AES256,
        sizeof(TEST_KEY_AES256),
        HSM_KEY_PERM_ENCRYPT
    );
    TEST_ASSERT_EQ(HSM_OK, status, "Key import should succeed");
    
    occupied = Hsm_Keystore_IsSlotOccupied(TEST_KEY_SLOT_0);
    TEST_ASSERT(occupied == true, "Occupied slot should return true");
    
    /* Check invalid slot */
    occupied = Hsm_Keystore_IsSlotOccupied(INVALID_SLOT);
    TEST_ASSERT(occupied == false, "Invalid slot should return false");
    
    Hsm_Keystore_DeleteKey(TEST_KEY_SLOT_0);
    Hsm_Keystore_Deinit();
    
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
    printf("  Keystore Manager Unit Tests\n");
    printf("==============================================\n");
    
    total_failures += test_keystore_init();
    total_failures += test_key_import();
    total_failures += test_key_slot_info();
    total_failures += test_key_get();
    total_failures += test_key_delete();
    total_failures += test_clear_all_keys();
    total_failures += test_is_slot_occupied();
    
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
