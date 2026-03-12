/**
 * @file ehsm_keystore.c
 * @brief Implementation of the Keystore Manager.
 * 
 * Manages secure key storage with access control and permission enforcement.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_keystore.h"
#include "ehsm/ehsm_mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE TYPE DEFINITIONS
 * ============================================================================
 */

typedef struct KeystoreSlot
{
    ehsm_keySlot_t info;
    ehsm_key_t     key;
    uint32_t       accessCount;
} keystore_slot_t;

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

static keystore_slot_t g_keySlots[EHSM_MAX_KEY_SLOTS];
static bool g_keystoreInitialized = false;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================
 */

static void secureClear(uint8_t slotId)
{
    keystore_slot_t* slot = &g_keySlots[slotId];
    
    volatile uint8_t* keyPtr = slot->key.data;
    for (size_t i = 0U; i < sizeof(slot->key.data); i++)
    {
        keyPtr[i] = 0U;
    }
    
    slot->info.isOccupied = false;
    slot->info.algorithm = EHSM_ALGORITHM_NONE;
    slot->info.permissions = EHSM_KEY_PERM_NONE;
    slot->info.keySize = 0U;
    slot->accessCount = 0U;
}

static bool isValidSlotId(uint8_t slotId)
{
    return (slotId < EHSM_MAX_KEY_SLOTS);
}

/* ============================================================================
 * PUBLIC IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_keystore_init(void)
{
    if (g_keystoreInitialized)
    {
        return EHSM_STATUS_OK;
    }
    
    (void)memset(g_keySlots, 0, sizeof(g_keySlots));
    
    for (uint8_t i = 0U; i < EHSM_MAX_KEY_SLOTS; i++)
    {
        g_keySlots[i].info.id = i;
        g_keySlots[i].info.isOccupied = false;
        g_keySlots[i].info.algorithm = EHSM_ALGORITHM_NONE;
        g_keySlots[i].info.permissions = EHSM_KEY_PERM_NONE;
        g_keySlots[i].info.keySize = 0U;
        g_keySlots[i].accessCount = 0U;
    }
    
    g_keystoreInitialized = true;
    ehsm_mcal_uartLog("[KEYSTORE] Initialized: %d slots", EHSM_MAX_KEY_SLOTS);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_deinit(void)
{
    if (!g_keystoreInitialized)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    (void)ehsm_keystore_clearAll();
    g_keystoreInitialized = false;
    ehsm_mcal_uartLog("[KEYSTORE] Deinitialized");
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_importKey(
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* keyData,
    size_t keySize,
    uint8_t permissions
)
{
    keystore_slot_t* slot;
    
    if (!isValidSlotId(keySlotId))
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    if ((keyData == NULL) || (keySize == 0U) || (keySize > EHSM_MAX_KEY_SIZE))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    slot = &g_keySlots[keySlotId];
    
    if (slot->info.isOccupied)
    {
        ehsm_mcal_uartLog("[KEYSTORE] Import failed: slot %d occupied", keySlotId);
        return EHSM_STATUS_ERR_KEY_SLOT_FULL;
    }
    
    (void)memcpy(slot->key.data, keyData, keySize);
    slot->key.size = (uint32_t)keySize;
    slot->key.algorithm = algorithm;
    
    slot->info.isOccupied = true;
    slot->info.algorithm = algorithm;
    slot->info.permissions = permissions;
    slot->info.keySize = (uint32_t)keySize;
    slot->accessCount = 0U;
    
    ehsm_mcal_uartLog("[KEYSTORE] Key imported: slot=%d, algo=%d, size=%zu",
                      keySlotId, algorithm, keySize);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_deleteKey(uint8_t keySlotId)
{
    keystore_slot_t* slot;
    
    if (!isValidSlotId(keySlotId))
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_keySlots[keySlotId];
    
    if (!slot->info.isOccupied)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    secureClear(keySlotId);
    ehsm_mcal_uartLog("[KEYSTORE] Key deleted: slot=%d", keySlotId);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_getKey(uint8_t keySlotId, ehsm_key_t* key)
{
    keystore_slot_t* slot;
    
    if (key == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (!isValidSlotId(keySlotId))
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_keySlots[keySlotId];
    
    if (!slot->info.isOccupied)
    {
        ehsm_mcal_uartLog("[KEYSTORE] GetKey failed: slot %d empty", keySlotId);
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    (void)memcpy(key, &slot->key, sizeof(ehsm_key_t));
    slot->accessCount++;
    
    ehsm_mcal_uartLog("[KEYSTORE] Key accessed: slot=%d, count=%lu",
                      keySlotId, (unsigned long)slot->accessCount);
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_getSlotInfo(uint8_t keySlotId, ehsm_keySlot_t* keySlot)
{
    keystore_slot_t* slot;
    
    if (keySlot == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (!isValidSlotId(keySlotId))
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_keySlots[keySlotId];
    (void)memcpy(keySlot, &slot->info, sizeof(ehsm_keySlot_t));
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_keystore_clearAll(void)
{
    for (uint8_t i = 0U; i < EHSM_MAX_KEY_SLOTS; i++)
    {
        secureClear(i);
    }
    
    ehsm_mcal_uartLog("[KEYSTORE] All keys cleared");
    return EHSM_STATUS_OK;
}

bool ehsm_keystore_isSlotOccupied(uint8_t keySlotId)
{
    if (!isValidSlotId(keySlotId))
    {
        return false;
    }
    
    return g_keySlots[keySlotId].info.isOccupied;
}
