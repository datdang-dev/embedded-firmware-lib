/**
 * @file ehsm_api.c
 * @brief Implementation of the Embedded HSM public API layer.
 * 
 * This file implements the functions declared in ehsm_api.h.
 * The API layer acts as a facade, delegating work to the middleware layer.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_api.h"
#include "ehsm/ehsm_middleware.h"
#include "ehsm/ehsm_keystore.h"
#include <string.h>

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** Firmware version string */
#define EHSM_VERSION_STRING     "1.0.0"

/** Magic number to verify initialization state */
#define EHSM_INIT_MAGIC         (0x4548534DU)  /* "EHSM" */

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** HSM initialization state flag */
static uint32_t g_ehsmInitialized = 0U;

/* ============================================================================
 * INITIALIZATION & SHUTDOWN IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_api_init(void)
{
    ehsm_status_t status;
    
    /* Check if already initialized */
    if (g_ehsmInitialized == EHSM_INIT_MAGIC)
    {
        return EHSM_STATUS_OK;
    }
    
    /* Initialize middleware layer (which initializes lower layers) */
    status = ehsm_mw_init();
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Initialize keystore */
    status = ehsm_keystore_init();
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Mark as initialized */
    g_ehsmInitialized = EHSM_INIT_MAGIC;
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_api_deinit(void)
{
    ehsm_status_t status;
    
    /* Check if initialized */
    if (g_ehsmInitialized != EHSM_INIT_MAGIC)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Clear all keys securely */
    status = ehsm_api_clearAllKeys();
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Deinitialize middleware */
    status = ehsm_mw_deinit();
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Clear initialization flag */
    g_ehsmInitialized = 0U;
    
    return EHSM_STATUS_OK;
}

bool ehsm_api_isReady(void)
{
    return (g_ehsmInitialized == EHSM_INIT_MAGIC);
}

/* ============================================================================
 * SESSION MANAGEMENT IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_api_createSession(ehsm_sessionId_t* sessionId)
{
    /* Validate output parameter */
    if (sessionId == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to middleware */
    return ehsm_mw_createSession(sessionId);
}

ehsm_status_t ehsm_api_closeSession(ehsm_sessionId_t sessionId)
{
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to middleware */
    return ehsm_mw_closeSession(sessionId);
}

bool ehsm_api_isSessionValid(ehsm_sessionId_t sessionId)
{
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return false;
    }
    
    /* Delegate to middleware */
    return ehsm_mw_isSessionValid(sessionId);
}

/* ============================================================================
 * CRYPTOGRAPHIC OPERATIONS IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_api_encrypt(
    ehsm_sessionId_t sessionId,
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t* outputLen
)
{
    ehsm_status_t status;
    ehsm_cryptoJob_t job;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL) || (outputLen == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (inputLen == 0U)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Validate session */
    if (!ehsm_api_isSessionValid(sessionId))
    {
        return EHSM_STATUS_ERR_SESSION_INVALID;
    }
    
    /* Validate key slot */
    if (keySlotId >= EHSM_MAX_KEY_SLOTS)
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    /* Check key slot permissions */
    ehsm_keySlot_t keySlot;
    status = ehsm_api_getKeySlotInfo(keySlotId, &keySlot);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    if ((keySlot.permissions & EHSM_KEY_PERM_ENCRYPT) == 0U)
    {
        return EHSM_STATUS_ERR_AUTH_FAILED;
    }
    
    /* Prepare crypto job */
    (void)memset(&job, 0, sizeof(job));
    job.operation     = EHSM_CRYPTO_OP_ENCRYPT;
    job.algorithm     = algorithm;
    job.keySlotId     = keySlotId;
    job.sessionId     = sessionId;
    job.input.data    = (uint8_t*)input;
    job.input.length  = inputLen;
    job.input.size    = inputLen;
    job.output.data   = output;
    job.output.size   = *outputLen;
    job.isComplete    = false;
    
    /* Dispatch job through middleware */
    status = ehsm_mw_dispatchJob(&job);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Update output length */
    *outputLen = job.output.length;
    
    return job.result;
}

ehsm_status_t ehsm_api_decrypt(
    ehsm_sessionId_t sessionId,
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t* outputLen
)
{
    ehsm_status_t status;
    ehsm_cryptoJob_t job;
    
    /* Validate parameters */
    if ((input == NULL) || (output == NULL) || (outputLen == NULL))
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (inputLen == 0U)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Validate session */
    if (!ehsm_api_isSessionValid(sessionId))
    {
        return EHSM_STATUS_ERR_SESSION_INVALID;
    }
    
    /* Validate key slot */
    if (keySlotId >= EHSM_MAX_KEY_SLOTS)
    {
        return EHSM_STATUS_ERR_INVALID_KEY_ID;
    }
    
    /* Check key slot permissions */
    ehsm_keySlot_t keySlot;
    status = ehsm_api_getKeySlotInfo(keySlotId, &keySlot);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    if ((keySlot.permissions & EHSM_KEY_PERM_DECRYPT) == 0U)
    {
        return EHSM_STATUS_ERR_AUTH_FAILED;
    }
    
    /* Prepare crypto job */
    (void)memset(&job, 0, sizeof(job));
    job.operation     = EHSM_CRYPTO_OP_DECRYPT;
    job.algorithm     = algorithm;
    job.keySlotId     = keySlotId;
    job.sessionId     = sessionId;
    job.input.data    = (uint8_t*)input;
    job.input.length  = inputLen;
    job.input.size    = inputLen;
    job.output.data   = output;
    job.output.size   = *outputLen;
    job.isComplete    = false;
    
    /* Dispatch job through middleware */
    status = ehsm_mw_dispatchJob(&job);
    if (status != EHSM_STATUS_OK)
    {
        return status;
    }
    
    /* Update output length */
    *outputLen = job.output.length;
    
    return job.result;
}

/* ============================================================================
 * KEY MANAGEMENT IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_api_importKey(
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* keyData,
    size_t keySize,
    uint8_t permissions
)
{
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return ehsm_keystore_importKey(keySlotId, algorithm, keyData, keySize, permissions);
}

ehsm_status_t ehsm_api_deleteKey(uint8_t keySlotId)
{
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return ehsm_keystore_deleteKey(keySlotId);
}

ehsm_status_t ehsm_api_getKeySlotInfo(uint8_t keySlotId, ehsm_keySlot_t* keySlot)
{
    /* Validate output parameter */
    if (keySlot == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return ehsm_keystore_getSlotInfo(keySlotId, keySlot);
}

ehsm_status_t ehsm_api_clearAllKeys(void)
{
    /* Check if HSM is ready */
    if (!ehsm_api_isReady())
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    /* Delegate to keystore */
    return ehsm_keystore_clearAll();
}

/* ============================================================================
 * DIAGNOSTICS IMPLEMENTATION
 * ============================================================================
 */

const char* ehsm_api_getVersion(void)
{
    return EHSM_VERSION_STRING;
}

const char* ehsm_api_getErrorString(ehsm_status_t status)
{
    switch (status)
    {
        case EHSM_STATUS_OK:                      return "Success";
        case EHSM_STATUS_ERR_BUSY:                return "HSM is busy";
        case EHSM_STATUS_ERR_INVALID_PARAM:       return "Invalid parameter";
        case EHSM_STATUS_ERR_INVALID_KEY_ID:      return "Invalid key ID";
        case EHSM_STATUS_ERR_KEY_SLOT_FULL:       return "Key slot full";
        case EHSM_STATUS_ERR_INSUFFICIENT_BUFFER: return "Insufficient buffer";
        case EHSM_STATUS_ERR_CRYPTO_FAILED:       return "Crypto operation failed";
        case EHSM_STATUS_ERR_AUTH_FAILED:         return "Authentication failed";
        case EHSM_STATUS_ERR_SESSION_INVALID:     return "Invalid session";
        case EHSM_STATUS_ERR_QUEUE_FULL:          return "Job queue full";
        case EHSM_STATUS_ERR_HARDWARE:            return "Hardware error";
        case EHSM_STATUS_ERR_NOT_INITIALIZED:     return "HSM not initialized";
        case EHSM_STATUS_ERR_UNSUPPORTED:         return "Operation unsupported";
        case EHSM_STATUS_ERR_TIMEOUT:             return "Operation timeout";
        case EHSM_STATUS_ERR_INTERNAL:            return "Internal error";
        default:                                  return "Unknown error";
    }
}
