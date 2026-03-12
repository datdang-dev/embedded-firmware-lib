/**
 * @file hsm_keystore.c
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
#include "hsm/hsm_keystore.h"
#include "hsm/hsm_storage_driver.h"
#include "hsm/mcal_uart.h"
#include <string.h>

/* ============================================================================
 * PRIVATE TYPE DEFINITIONS
 * ============================================================================
 */

/**
 * @brief Internal key slot structure.
 */
typedef struct Keystore_Slot
{
    Hsm_KeySlot_t     info;           /**< Slot metadata */
    Hsm_Key_t         key;            /**< Key data */
    uint32_t          access_count;   /**< Number of accesses */
} Keystore_Slot_t;

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** Key slot storage array */
static Keystore_Slot_t g_key_slots[HSM_MAX_KEY_SLOTS];

/** Keystore initialization flag */
static bool g_keystore_initialized = false;

/* ============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * ============================================================================
 */

/**
 * @brief Securely zero a key slot.
 * 
 * @param slot_id Slot ID to clear.
 */
static void Keystore_SecureClear(uint8_t slot_id);

/**
 * @brief Validate a key slot ID.
 * 
 * @param slot_id Slot ID to validate.
 * @return true if valid, false otherwise.
 */
static bool Keystore_IsValidSlotId(uint8_t slot_id);

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Keystore_Init(void)
{
    /* Check if already initialized */
    if (g_keystore_initialized)
    {
        return HSM_OK;
    }
    
    /* Clear all key slots */
    (void)memset(g_key_slots, 0, sizeof(g_key_slots));
    
    /* Initialize slot metadata */
    for (uint8_t i = 0U; i < HSM_MAX_KEY_SLOTS; i++)
    {
        g_key_slots[i].info.id = i;
        g_key_slots[i].info.is_occupied = false;
        g_key_slots[i].info.algorithm = HSM_ALGO_NONE;
        g_key_slots[i].info.permissions = HSM_KEY_PERM_NONE;
        g_key_slots[i].info.key_size = 0U;
        g_key_slots[i].access_count = 0U;
    }
    
    g_keystore_initialized = true;
    
    Mcal_Uart_Log("[KEYSTORE] Initialized: %d slots", HSM_MAX_KEY_SLOTS);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Keystore_Deinit(void)
{
    /* Check if initialized */
    if (!g_keystore_initialized)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Securely clear all keys */
    (void)Hsm_Keystore_ClearAll();
    
    g_keystore_initialized = false;
    
    Mcal_Uart_Log("[KEYSTORE] Deinitialized");
    
    return HSM_OK;
}

/* ============================================================================
 * KEY OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Keystore_ImportKey(
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* key_data,
    size_t key_size,
    uint8_t permissions
)
{
    Keystore_Slot_t* slot;
    
    /* Validate slot ID */
    if (!Keystore_IsValidSlotId(key_slot_id))
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    /* Validate key data */
    if ((key_data == NULL) || (key_size == 0U) || (key_size > HSM_MAX_KEY_SIZE))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    slot = &g_key_slots[key_slot_id];
    
    /* Check if slot is already occupied */
    if (slot->info.is_occupied)
    {
        Mcal_Uart_Log("[KEYSTORE] Import failed: slot %d occupied", key_slot_id);
        return HSM_ERR_KEY_SLOT_FULL;
    }
    
    /* Copy key data securely */
    (void)memcpy(slot->key.data, key_data, key_size);
    slot->key.size = (uint32_t)key_size;
    slot->key.algorithm = algorithm;
    
    /* Update slot metadata */
    slot->info.is_occupied = true;
    slot->info.algorithm = algorithm;
    slot->info.permissions = permissions;
    slot->info.key_size = (uint32_t)key_size;
    slot->access_count = 0U;
    
    Mcal_Uart_Log("[KEYSTORE] Key imported: slot=%d, algo=%d, size=%zu, perms=0x%02X",
                  key_slot_id, algorithm, key_size, permissions);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Keystore_DeleteKey(uint8_t key_slot_id)
{
    Keystore_Slot_t* slot;
    
    /* Validate slot ID */
    if (!Keystore_IsValidSlotId(key_slot_id))
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_key_slots[key_slot_id];
    
    /* Check if slot is occupied */
    if (!slot->info.is_occupied)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Securely clear the slot */
    Keystore_SecureClear(key_slot_id);
    
    Mcal_Uart_Log("[KEYSTORE] Key deleted: slot=%d", key_slot_id);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Keystore_GetKey(uint8_t key_slot_id, Hsm_Key_t* key)
{
    Keystore_Slot_t* slot;
    
    /* Validate parameters */
    if (key == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Validate slot ID */
    if (!Keystore_IsValidSlotId(key_slot_id))
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_key_slots[key_slot_id];
    
    /* Check if slot is occupied */
    if (!slot->info.is_occupied)
    {
        Mcal_Uart_Log("[KEYSTORE] GetKey failed: slot %d empty", key_slot_id);
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    /* Copy key data */
    (void)memcpy(key, &slot->key, sizeof(Hsm_Key_t));
    slot->access_count++;
    
    Mcal_Uart_Log("[KEYSTORE] Key accessed: slot=%d, count=%lu",
                  key_slot_id, (unsigned long)slot->access_count);
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Keystore_GetSlotInfo(uint8_t key_slot_id, Hsm_KeySlot_t* key_slot)
{
    Keystore_Slot_t* slot;
    
    /* Validate parameters */
    if (key_slot == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Validate slot ID */
    if (!Keystore_IsValidSlotId(key_slot_id))
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    slot = &g_key_slots[key_slot_id];
    
    /* Copy slot info */
    (void)memcpy(key_slot, &slot->info, sizeof(Hsm_KeySlot_t));
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Keystore_ClearAll(void)
{
    /* Securely clear all slots */
    for (uint8_t i = 0U; i < HSM_MAX_KEY_SLOTS; i++)
    {
        Keystore_SecureClear(i);
    }
    
    Mcal_Uart_Log("[KEYSTORE] All keys cleared");
    
    return HSM_OK;
}

bool Hsm_Keystore_IsSlotOccupied(uint8_t key_slot_id)
{
    /* Validate slot ID */
    if (!Keystore_IsValidSlotId(key_slot_id))
    {
        return false;
    }
    
    return g_key_slots[key_slot_id].info.is_occupied;
}

/* ============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */

static void Keystore_SecureClear(uint8_t slot_id)
{
    Keystore_Slot_t* slot = &g_key_slots[slot_id];
    
    /* Overwrite key data with zeros (secure erase) */
    volatile uint8_t* key_ptr = slot->key.data;
    for (size_t i = 0U; i < sizeof(slot->key.data); i++)
    {
        key_ptr[i] = 0U;
    }
    
    /* Clear slot metadata */
    slot->info.is_occupied = false;
    slot->info.algorithm = HSM_ALGO_NONE;
    slot->info.permissions = HSM_KEY_PERM_NONE;
    slot->info.key_size = 0U;
    slot->access_count = 0U;
}

static bool Keystore_IsValidSlotId(uint8_t slot_id)
{
    return (slot_id < HSM_MAX_KEY_SLOTS);
}
