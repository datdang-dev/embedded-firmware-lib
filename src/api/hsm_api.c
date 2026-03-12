/**
 * @file hsm_api.c
 * @brief Implementation of the HSM public API layer.
 * 
 * This file implements the functions declared in hsm_api.h.
 * The API layer acts as a facade, delegating work to the middleware layer.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_api.h"
#include "hsm/hsm_middleware.h"
#include "hsm/hsm_keystore.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** Firmware version string */
#define HSM_VERSION_STRING      "1.0.0"

/** Magic number to verify initialization state */
#define HSM_INIT_MAGIC          (0x48534D01U)  /* "HSM" + version */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** HSM initialization state flag */
static uint32_t g_hsm_initialized = 0U;

/* ============================================================================
 * INITIALIZATION & SHUTDOWN IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Init(void)
{
    Hsm_Status_t status;
    
    /* Check if already initialized */
    if (g_hsm_initialized == HSM_INIT_MAGIC)
    {
        return HSM_OK;
    }
    
    /* Initialize middleware layer (which initializes lower layers) */
    status = Hsm_Middleware_Init();
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Initialize keystore */
    status = Hsm_Keystore_Init();
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Mark as initialized */
    g_hsm_initialized = HSM_INIT_MAGIC;
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Deinit(void)
{
    Hsm_Status_t status;
    
    /* Check if initialized */
    if (g_hsm_initialized != HSM_INIT_MAGIC)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Clear all keys securely */
    status = Hsm_ClearAllKeys();
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Deinitialize middleware */
    status = Hsm_Middleware_Deinit();
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Clear initialization flag */
    g_hsm_initialized = 0U;
    
    return HSM_OK;
}

bool Hsm_IsReady(void)
{
    return (g_hsm_initialized == HSM_INIT_MAGIC);
}

/* ============================================================================
 * SESSION MANAGEMENT IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_CreateSession(Hsm_SessionId_t* session_id)
{
    /* Validate output parameter */
    if (session_id == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to middleware */
    return Hsm_Middleware_CreateSession(session_id);
}

Hsm_Status_t Hsm_CloseSession(Hsm_SessionId_t session_id)
{
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to middleware */
    return Hsm_Middleware_CloseSession(session_id);
}

bool Hsm_IsSessionValid(Hsm_SessionId_t session_id)
{
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return false;
    }
    
    /* Delegate to middleware */
    return Hsm_Middleware_IsSessionValid(session_id);
}

/* ============================================================================
 * CRYPTOGRAPHIC OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Encrypt(
    Hsm_SessionId_t session_id,
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t* output_len
)
{
    Hsm_Status_t status;
    Hsm_CryptoJob_t job;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL) || (output_len == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    if (input_len == 0U)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate session */
    if (!Hsm_IsSessionValid(session_id))
    {
        return HSM_ERR_SESSION_INVALID;
    }
    
    /* Validate key slot */
    if (key_slot_id >= HSM_MAX_KEY_SLOTS)
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    /* Check key slot permissions */
    Hsm_KeySlot_t key_slot;
    status = Hsm_GetKeySlotInfo(key_slot_id, &key_slot);
    if (status != HSM_OK)
    {
        return status;
    }
    
    if ((key_slot.permissions & HSM_KEY_PERM_ENCRYPT) == 0U)
    {
        return HSM_ERR_AUTH_FAILED;
    }
    
    /* Prepare crypto job */
    (void)memset(&job, 0, sizeof(job));
    job.operation     = HSM_CRYPTO_OP_ENCRYPT;
    job.algorithm     = algorithm;
    job.key_slot_id   = key_slot_id;
    job.session_id    = session_id;
    job.input.data    = (uint8_t*)input;
    job.input.length  = input_len;
    job.input.size    = input_len;
    job.output.data   = output;
    job.output.size   = *output_len;
    job.is_complete   = false;
    
    /* Dispatch job through middleware */
    status = Hsm_Middleware_DispatchJob(&job);
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Update output length */
    *output_len = job.output.length;
    
    return job.result;
}

Hsm_Status_t Hsm_Decrypt(
    Hsm_SessionId_t session_id,
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* input,
    size_t input_len,
    uint8_t* output,
    size_t* output_len
)
{
    Hsm_Status_t status;
    Hsm_CryptoJob_t job;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL) || (output_len == NULL))
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    if (input_len == 0U)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate session */
    if (!Hsm_IsSessionValid(session_id))
    {
        return HSM_ERR_SESSION_INVALID;
    }
    
    /* Validate key slot */
    if (key_slot_id >= HSM_MAX_KEY_SLOTS)
    {
        return HSM_ERR_INVALID_KEY_ID;
    }
    
    /* Check key slot permissions */
    Hsm_KeySlot_t key_slot;
    status = Hsm_GetKeySlotInfo(key_slot_id, &key_slot);
    if (status != HSM_OK)
    {
        return status;
    }
    
    if ((key_slot.permissions & HSM_KEY_PERM_DECRYPT) == 0U)
    {
        return HSM_ERR_AUTH_FAILED;
    }
    
    /* Prepare crypto job */
    (void)memset(&job, 0, sizeof(job));
    job.operation     = HSM_CRYPTO_OP_DECRYPT;
    job.algorithm     = algorithm;
    job.key_slot_id   = key_slot_id;
    job.session_id    = session_id;
    job.input.data    = (uint8_t*)input;
    job.input.length  = input_len;
    job.input.size    = input_len;
    job.output.data   = output;
    job.output.size   = *output_len;
    job.is_complete   = false;
    
    /* Dispatch job through middleware */
    status = Hsm_Middleware_DispatchJob(&job);
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Update output length */
    *output_len = job.output.length;
    
    return job.result;
}

/* ============================================================================
 * KEY MANAGEMENT IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_ImportKey(
    uint8_t key_slot_id,
    Hsm_Algorithm_t algorithm,
    const uint8_t* key_data,
    size_t key_size,
    uint8_t permissions
)
{
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return Hsm_Keystore_ImportKey(key_slot_id, algorithm, key_data, key_size, permissions);
}

Hsm_Status_t Hsm_DeleteKey(uint8_t key_slot_id)
{
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return Hsm_Keystore_DeleteKey(key_slot_id);
}

Hsm_Status_t Hsm_GetKeySlotInfo(uint8_t key_slot_id, Hsm_KeySlot_t* key_slot)
{
    /* Validate output parameter */
    if (key_slot == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return Hsm_Keystore_GetSlotInfo(key_slot_id, key_slot);
}

Hsm_Status_t Hsm_ClearAllKeys(void)
{
    /* Check if HSM is ready */
    if (!Hsm_IsReady())
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return Hsm_Keystore_ClearAll();
}

/* ============================================================================
 * DIAGNOSTICS IMPLEMENTATION
 * ============================================================================
 */

const char* Hsm_GetVersion(void)
{
    return HSM_VERSION_STRING;
}

const char* Hsm_GetErrorString(Hsm_Status_t status)
{
    switch (status)
    {
        case HSM_OK:                      return "Success";
        case HSM_ERR_BUSY:                return "HSM is busy";
        case HSM_ERR_INVALID_PARAM:       return "Invalid parameter";
        case HSM_ERR_INVALID_KEY_ID:      return "Invalid key ID";
        case HSM_ERR_KEY_SLOT_FULL:       return "Key slot full";
        case HSM_ERR_INSUFFICIENT_BUFFER: return "Insufficient buffer";
        case HSM_ERR_CRYPTO_FAILED:       return "Crypto operation failed";
        case HSM_ERR_AUTH_FAILED:         return "Authentication failed";
        case HSM_ERR_SESSION_INVALID:     return "Invalid session";
        case HSM_ERR_QUEUE_FULL:          return "Job queue full";
        case HSM_ERR_HARDWARE:            return "Hardware error";
        case HSM_ERR_NOT_INITIALIZED:     return "HSM not initialized";
        case HSM_ERR_UNSUPPORTED:         return "Operation unsupported";
        case HSM_ERR_TIMEOUT:             return "Operation timeout";
        case HSM_ERR_INTERNAL:            return "Internal error";
        default:                          return "Unknown error";
    }
}
