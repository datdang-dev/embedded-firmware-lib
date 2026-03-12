/**
 * @file ehsm_types.h
 * @brief Common type definitions and status codes for the Embedded HSM firmware.
 * 
 * This header provides the foundational types used throughout the HSM stack.
 * All types follow MISRA-C guidelines with explicit-width integer types.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_TYPES_H
#define EHSM_TYPES_H

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
 * All HSM functions return ehsm_status_t to indicate success or failure.
 * Positive values indicate success, negative values indicate errors.
 */
typedef enum EhsmStatus
{
    EHSM_STATUS_OK                      =  0,   /**< Operation completed successfully */
    EHSM_STATUS_ERR_BUSY                = -1,   /**< HSM is busy processing another job */
    EHSM_STATUS_ERR_INVALID_PARAM       = -2,   /**< Invalid parameter provided */
    EHSM_STATUS_ERR_INVALID_KEY_ID      = -3,   /**< Key ID does not exist or is invalid */
    EHSM_STATUS_ERR_KEY_SLOT_FULL       = -4,   /**< No available key slots */
    EHSM_STATUS_ERR_INSUFFICIENT_BUFFER = -5,   /**< Output buffer too small */
    EHSM_STATUS_ERR_CRYPTO_FAILED       = -6,   /**< Cryptographic operation failed */
    EHSM_STATUS_ERR_AUTH_FAILED         = -7,   /**< Authentication/authorization failed */
    EHSM_STATUS_ERR_SESSION_INVALID     = -8,   /**< Session ID is invalid or expired */
    EHSM_STATUS_ERR_QUEUE_FULL          = -9,   /**< Job queue is full */
    EHSM_STATUS_ERR_HARDWARE            = -10,  /**< Hardware/driver error */
    EHSM_STATUS_ERR_NOT_INITIALIZED     = -11,  /**< HSM not initialized */
    EHSM_STATUS_ERR_UNSUPPORTED         = -12,  /**< Operation not supported */
    EHSM_STATUS_ERR_TIMEOUT             = -13,  /**< Operation timed out */
    EHSM_STATUS_ERR_INTERNAL            = -14   /**< Internal error (should not occur) */
} ehsm_status_t;

/**
 * @brief Cryptographic operation types supported by the HSM.
 */
typedef enum EhsmCryptoOp
{
    EHSM_CRYPTO_OP_NONE          = 0,    /**< No operation */
    EHSM_CRYPTO_OP_ENCRYPT       = 1,    /**< Encryption operation */
    EHSM_CRYPTO_OP_DECRYPT       = 2,    /**< Decryption operation */
    EHSM_CRYPTO_OP_SIGN          = 3,    /**< Digital signature generation */
    EHSM_CRYPTO_OP_VERIFY        = 4,    /**< Digital signature verification */
    EHSM_CRYPTO_OP_HASH          = 5,    /**< Hash computation */
    EHSM_CRYPTO_OP_KEY_GEN       = 6     /**< Key generation */
} ehsm_cryptoOp_t;

/**
 * @brief Supported encryption algorithms.
 */
typedef enum EhsmAlgorithm
{
    EHSM_ALGORITHM_NONE          = 0,    /**< No algorithm */
    EHSM_ALGORITHM_AES_128       = 1,    /**< AES-128 */
    EHSM_ALGORITHM_AES_256       = 2,    /**< AES-256 */
    EHSM_ALGORITHM_RSA_2048      = 3,    /**< RSA-2048 */
    EHSM_ALGORITHM_RSA_4096      = 4,    /**< RSA-4096 */
    EHSM_ALGORITHM_ECC_P256      = 5,    /**< ECC P-256 */
    EHSM_ALGORITHM_SHA_256       = 6     /**< SHA-256 */
} ehsm_algorithm_t;

/**
 * @brief Key access permission flags.
 */
typedef enum EhsmKeyPermission
{
    EHSM_KEY_PERM_NONE           = 0x00, /**< No permissions */
    EHSM_KEY_PERM_ENCRYPT        = 0x01, /**< Can be used for encryption */
    EHSM_KEY_PERM_DECRYPT        = 0x02, /**< Can be used for decryption */
    EHSM_KEY_PERM_SIGN           = 0x04, /**< Can be used for signing */
    EHSM_KEY_PERM_VERIFY         = 0x08, /**< Can be used for verification */
    EHSM_KEY_PERM_EXPORT         = 0x10, /**< Can be exported (rarely set) */
    EHSM_KEY_PERM_ALL            = 0xFF  /**< All permissions */
} ehsm_keyPermission_t;

/**
 * @brief Key slot information structure.
 */
typedef struct EhsmKeySlot
{
    uint8_t            id;                 /**< Key slot ID (0-based) */
    bool               isOccupied;         /**< Slot is occupied */
    ehsm_algorithm_t   algorithm;          /**< Algorithm associated with key */
    uint8_t            permissions;        /**< Access permission flags */
    uint32_t           keySize;            /**< Key size in bytes */
} ehsm_keySlot_t;

/**
 * @brief Session handle for tracking HSM operations.
 */
typedef uint32_t ehsm_sessionId_t;

/**
 * @brief Job identifier for tracking queued operations.
 */
typedef uint32_t ehsm_jobId_t;

/**
 * @brief Cryptographic key data structure.
 * 
 * Keys are stored in secure memory and never exposed directly.
 */
typedef struct EhsmKey
{
    uint8_t          data[64];             /**< Key data (max 512-bit for RSA) */
    uint32_t         size;                 /**< Actual key size in bytes */
    ehsm_algorithm_t algorithm;            /**< Algorithm type */
} ehsm_key_t;

/**
 * @brief Input/output buffer descriptor for crypto operations.
 */
typedef struct EhsmBuffer
{
    uint8_t* data;                         /**< Pointer to buffer */
    size_t   size;                         /**< Buffer capacity in bytes */
    size_t   length;                       /**< Actual data length in bytes */
} ehsm_buffer_t;

/**
 * @brief Cryptographic job descriptor passed through the middleware.
 */
typedef struct EhsmCryptoJob
{
    ehsm_jobId_t       jobId;              /**< Unique job identifier */
    ehsm_sessionId_t   sessionId;          /**< Associated session */
    ehsm_cryptoOp_t    operation;          /**< Crypto operation type */
    ehsm_algorithm_t   algorithm;          /**< Algorithm to use */
    uint8_t            keySlotId;          /**< Key slot containing the key */
    ehsm_buffer_t      input;              /**< Input data buffer */
    ehsm_buffer_t      output;             /**< Output data buffer */
    void*              context;            /**< Operation-specific context */
    bool               isComplete;         /**< Job completion flag */
    ehsm_status_t      result;             /**< Job result status */
} ehsm_cryptoJob_t;

/* ============================================================================
 * CONSTANTS
 * ============================================================================
 */

/** Maximum number of key slots in the keystore */
#define EHSM_MAX_KEY_SLOTS            (8U)

/** Maximum job queue depth */
#define EHSM_MAX_JOB_QUEUE_DEPTH      (16U)

/** Maximum number of concurrent sessions */
#define EHSM_MAX_SESSIONS             (4U)

/** Invalid session ID value */
#define EHSM_SESSION_ID_INVALID       (0xFFFFFFFFU)

/** Invalid job ID value */
#define EHSM_JOB_ID_INVALID           (0xFFFFFFFFU)

/** Maximum key size in bytes (512-bit RSA) */
#define EHSM_MAX_KEY_SIZE             (64U)

/** Maximum data block size for encryption/decryption */
#define EHSM_MAX_BLOCK_SIZE           (256U)

/* ============================================================================
 * UTILITY MACROS
 * ============================================================================
 */

/** Check if status indicates success */
#define EHSM_STATUS_IS_OK(status)     ((status) == EHSM_STATUS_OK)

/** Check if status indicates an error */
#define EHSM_STATUS_IS_ERROR(status)  ((status) != EHSM_STATUS_OK)

/** Get string representation of status (for debugging) */
#define EHSM_STATUS_TO_STRING(status) \
    ((status) == EHSM_STATUS_OK ? "EHSM_STATUS_OK" : \
     (status) == EHSM_STATUS_ERR_BUSY ? "EHSM_STATUS_ERR_BUSY" : \
     (status) == EHSM_STATUS_ERR_INVALID_PARAM ? "EHSM_STATUS_ERR_INVALID_PARAM" : \
     (status) == EHSM_STATUS_ERR_INVALID_KEY_ID ? "EHSM_STATUS_ERR_INVALID_KEY_ID" : \
     (status) == EHSM_STATUS_ERR_KEY_SLOT_FULL ? "EHSM_STATUS_ERR_KEY_SLOT_FULL" : \
     (status) == EHSM_STATUS_ERR_INSUFFICIENT_BUFFER ? "EHSM_STATUS_ERR_INSUFFICIENT_BUFFER" : \
     (status) == EHSM_STATUS_ERR_CRYPTO_FAILED ? "EHSM_STATUS_ERR_CRYPTO_FAILED" : \
     (status) == EHSM_STATUS_ERR_AUTH_FAILED ? "EHSM_STATUS_ERR_AUTH_FAILED" : \
     (status) == EHSM_STATUS_ERR_SESSION_INVALID ? "EHSM_STATUS_ERR_SESSION_INVALID" : \
     (status) == EHSM_STATUS_ERR_QUEUE_FULL ? "EHSM_STATUS_ERR_QUEUE_FULL" : \
     (status) == EHSM_STATUS_ERR_HARDWARE ? "EHSM_STATUS_ERR_HARDWARE" : \
     (status) == EHSM_STATUS_ERR_NOT_INITIALIZED ? "EHSM_STATUS_ERR_NOT_INITIALIZED" : \
     (status) == EHSM_STATUS_ERR_UNSUPPORTED ? "EHSM_STATUS_ERR_UNSUPPORTED" : \
     (status) == EHSM_STATUS_ERR_TIMEOUT ? "EHSM_STATUS_ERR_TIMEOUT" : \
     "EHSM_STATUS_ERR_INTERNAL")

#ifdef __cplusplus
}
#endif

#endif /* EHSM_TYPES_H */
