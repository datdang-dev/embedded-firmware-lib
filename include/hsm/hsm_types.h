/**
 * @file hsm_types.h
 * @brief Common type definitions and status codes for the HSM firmware.
 * 
 * This header provides the foundational types used throughout the HSM stack.
 * All types follow MISRA-C guidelines with explicit-width integer types.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_TYPES_H
#define HSM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * TYPE DEFINITIONS
 * ============================================================================
 */

/**
 * @brief HSM status codes following MISRA-C error handling pattern.
 * 
 * All HSM functions return Hsm_Status_t to indicate success or failure.
 * Positive values indicate success, negative values indicate errors.
 */
typedef enum Hsm_Status
{
    HSM_OK                      =  0,   /**< Operation completed successfully */
    HSM_ERR_BUSY                = -1,   /**< HSM is busy processing another job */
    HSM_ERR_INVALID_PARAM       = -2,   /**< Invalid parameter provided */
    HSM_ERR_INVALID_KEY_ID      = -3,   /**< Key ID does not exist or is invalid */
    HSM_ERR_KEY_SLOT_FULL       = -4,   /**< No available key slots */
    HSM_ERR_INSUFFICIENT_BUFFER = -5,   /**< Output buffer too small */
    HSM_ERR_CRYPTO_FAILED       = -6,   /**< Cryptographic operation failed */
    HSM_ERR_AUTH_FAILED         = -7,   /**< Authentication/authorization failed */
    HSM_ERR_SESSION_INVALID     = -8,   /**< Session ID is invalid or expired */
    HSM_ERR_QUEUE_FULL          = -9,   /**< Job queue is full */
    HSM_ERR_HARDWARE            = -10,  /**< Hardware/driver error */
    HSM_ERR_NOT_INITIALIZED     = -11,  /**< HSM not initialized */
    HSM_ERR_UNSUPPORTED         = -12,  /**< Operation not supported */
    HSM_ERR_TIMEOUT             = -13,  /**< Operation timed out */
    HSM_ERR_INTERNAL            = -14   /**< Internal error (should not occur) */
} Hsm_Status_t;

/**
 * @brief Cryptographic operation types supported by the HSM.
 */
typedef enum Hsm_CryptoOp
{
    HSM_CRYPTO_OP_NONE          = 0,    /**< No operation */
    HSM_CRYPTO_OP_ENCRYPT       = 1,    /**< Encryption operation */
    HSM_CRYPTO_OP_DECRYPT       = 2,    /**< Decryption operation */
    HSM_CRYPTO_OP_SIGN          = 3,    /**< Digital signature generation */
    HSM_CRYPTO_OP_VERIFY        = 4,    /**< Digital signature verification */
    HSM_CRYPTO_OP_HASH          = 5,    /**< Hash computation */
    HSM_CRYPTO_OP_KEY_GEN       = 6     /**< Key generation */
} Hsm_CryptoOp_t;

/**
 * @brief Supported encryption algorithms.
 */
typedef enum Hsm_Algorithm
{
    HSM_ALGO_NONE               = 0,    /**< No algorithm */
    HSM_ALGO_AES_128            = 1,    /**< AES-128 */
    HSM_ALGO_AES_256            = 2,    /**< AES-256 */
    HSM_ALGO_RSA_2048           = 3,    /**< RSA-2048 */
    HSM_ALGO_RSA_4096           = 4,    /**< RSA-4096 */
    HSM_ALGO_ECC_P256           = 5,    /**< ECC P-256 */
    HSM_ALGO_SHA_256            = 6     /**< SHA-256 */
} Hsm_Algorithm_t;

/**
 * @brief Key access permission flags.
 */
typedef enum Hsm_KeyPermission
{
    HSM_KEY_PERM_NONE           = 0x00, /**< No permissions */
    HSM_KEY_PERM_ENCRYPT        = 0x01, /**< Can be used for encryption */
    HSM_KEY_PERM_DECRYPT        = 0x02, /**< Can be used for decryption */
    HSM_KEY_PERM_SIGN           = 0x04, /**< Can be used for signing */
    HSM_KEY_PERM_VERIFY         = 0x08, /**< Can be used for verification */
    HSM_KEY_PERM_EXPORT         = 0x10, /**< Can be exported (rarely set) */
    HSM_KEY_PERM_ALL            = 0xFF  /**< All permissions */
} Hsm_KeyPermission_t;

/**
 * @brief Key slot information structure.
 */
typedef struct Hsm_KeySlot
{
    uint8_t         id;                 /**< Key slot ID (0-based) */
    bool            is_occupied;        /**< Slot is occupied */
    Hsm_Algorithm_t algorithm;          /**< Algorithm associated with key */
    uint8_t         permissions;        /**< Access permission flags */
    uint32_t        key_size;           /**< Key size in bytes */
} Hsm_KeySlot_t;

/**
 * @brief Session handle for tracking HSM operations.
 */
typedef uint32_t Hsm_SessionId_t;

/**
 * @brief Job identifier for tracking queued operations.
 */
typedef uint32_t Hsm_JobId_t;

/**
 * @brief Cryptographic key data structure.
 * 
 * Keys are stored in secure memory and never exposed directly.
 */
typedef struct Hsm_Key
{
    uint8_t         data[64];           /**< Key data (max 512-bit for RSA) */
    uint32_t        size;               /**< Actual key size in bytes */
    Hsm_Algorithm_t algorithm;          /**< Algorithm type */
} Hsm_Key_t;

/**
 * @brief Input/output buffer descriptor for crypto operations.
 */
typedef struct Hsm_Buffer
{
    uint8_t*        data;               /**< Pointer to buffer */
    size_t          size;               /**< Buffer capacity in bytes */
    size_t          length;             /**< Actual data length in bytes */
} Hsm_Buffer_t;

/**
 * @brief Crypto job descriptor passed through the middleware.
 */
typedef struct Hsm_CryptoJob
{
    Hsm_JobId_t         job_id;         /**< Unique job identifier */
    Hsm_SessionId_t     session_id;     /**< Associated session */
    Hsm_CryptoOp_t      operation;      /**< Crypto operation type */
    Hsm_Algorithm_t     algorithm;      /**< Algorithm to use */
    uint8_t             key_slot_id;    /**< Key slot containing the key */
    Hsm_Buffer_t        input;          /**< Input data buffer */
    Hsm_Buffer_t        output;         /**< Output data buffer */
    void*               context;        /**< Operation-specific context */
    bool                is_complete;    /**< Job completion flag */
    Hsm_Status_t        result;         /**< Job result status */
} Hsm_CryptoJob_t;

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

/** Maximum number of key slots in the keystore */
#define HSM_MAX_KEY_SLOTS           (8U)

/** Maximum job queue depth */
#define HSM_MAX_JOB_QUEUE_DEPTH     (16U)

/** Maximum number of concurrent sessions */
#define HSM_MAX_SESSIONS            (4U)

/** Invalid session ID value */
#define HSM_SESSION_ID_INVALID      (0xFFFFFFFFU)

/** Invalid job ID value */
#define HSM_JOB_ID_INVALID          (0xFFFFFFFFU)

/** Maximum key size in bytes (512-bit RSA) */
#define HSM_MAX_KEY_SIZE            (64U)

/** Maximum data block size for encryption/decryption */
#define HSM_MAX_BLOCK_SIZE          (256U)

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================
 */

/** Check if status indicates success */
#define HSM_STATUS_IS_OK(status)    ((status) == HSM_OK)

/** Check if status indicates an error */
#define HSM_STATUS_IS_ERROR(status) ((status) != HSM_OK)

/** Get string representation of status (for debugging) */
#define HSM_STATUS_TO_STRING(status) \
    ((status) == HSM_OK ? "HSM_OK" : \
     (status) == HSM_ERR_BUSY ? "HSM_ERR_BUSY" : \
     (status) == HSM_ERR_INVALID_PARAM ? "HSM_ERR_INVALID_PARAM" : \
     (status) == HSM_ERR_INVALID_KEY_ID ? "HSM_ERR_INVALID_KEY_ID" : \
     (status) == HSM_ERR_KEY_SLOT_FULL ? "HSM_ERR_KEY_SLOT_FULL" : \
     (status) == HSM_ERR_INSUFFICIENT_BUFFER ? "HSM_ERR_INSUFFICIENT_BUFFER" : \
     (status) == HSM_ERR_CRYPTO_FAILED ? "HSM_ERR_CRYPTO_FAILED" : \
     (status) == HSM_ERR_AUTH_FAILED ? "HSM_ERR_AUTH_FAILED" : \
     (status) == HSM_ERR_SESSION_INVALID ? "HSM_ERR_SESSION_INVALID" : \
     (status) == HSM_ERR_QUEUE_FULL ? "HSM_ERR_QUEUE_FULL" : \
     (status) == HSM_ERR_HARDWARE ? "HSM_ERR_HARDWARE" : \
     (status) == HSM_ERR_NOT_INITIALIZED ? "HSM_ERR_NOT_INITIALIZED" : \
     (status) == HSM_ERR_UNSUPPORTED ? "HSM_ERR_UNSUPPORTED" : \
     (status) == HSM_ERR_TIMEOUT ? "HSM_ERR_TIMEOUT" : \
     "HSM_ERR_INTERNAL")

#ifdef __cplusplus
}
#endif

#endif /* HSM_TYPES_H */
