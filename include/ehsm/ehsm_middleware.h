/**
 * @file ehsm_middleware.h
 * @brief Middleware layer: Crypto Job Router and Session Manager.
 * 
 * The middleware layer acts as the central dispatcher for cryptographic jobs.
 * It manages sessions, queues jobs, and routes them to the appropriate
 * service layer components.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_MIDDLEWARE_H
#define EHSM_MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_types.h"

/* ============================================================================
 * INITIALIZATION
 * ============================================================================
 */

/**
 * @brief Initialize the middleware layer.
 * 
 * Initializes session management and job queue.
 * Must be called during HSM initialization.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mw_init(void);

/**
 * @brief Deinitialize the middleware layer.
 * 
 * Cleans up all sessions and clears the job queue.
 * 
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mw_deinit(void);

/* ============================================================================
 * SESSION MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Create a new session.
 * 
 * @param[out] sessionId Pointer to store the new session ID.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mw_createSession(ehsm_sessionId_t* sessionId);

/**
 * @brief Close an existing session.
 * 
 * @param[in] sessionId Session ID to close.
 * @return EHSM_STATUS_OK on success, error code otherwise.
 */
ehsm_status_t ehsm_mw_closeSession(ehsm_sessionId_t sessionId);

/**
 * @brief Check if a session ID is valid.
 * 
 * @param[in] sessionId Session ID to validate.
 * @return true if valid, false otherwise.
 */
bool ehsm_mw_isSessionValid(ehsm_sessionId_t sessionId);

/* ============================================================================
 * JOB DISPATCHING
 * ============================================================================
 */

/**
 * @brief Dispatch a cryptographic job for processing.
 * 
 * This function queues the job and routes it to the appropriate
 * crypto service based on the operation type.
 * 
 * @param[in,out] job Pointer to the job descriptor (modified in-place).
 * @return EHSM_STATUS_OK on success, error code otherwise.
 * 
 * @pre Job must have valid operation, algorithm, and keySlotId
 * @post Job result and isComplete fields are updated
 */
ehsm_status_t ehsm_mw_dispatchJob(ehsm_cryptoJob_t* job);

/**
 * @brief Get the current job queue depth.
 * 
 * @return Number of jobs currently in the queue.
 */
size_t ehsm_mw_getQueueDepth(void);

/**
 * @brief Check if the job queue is full.
 * 
 * @return true if queue is full, false otherwise.
 */
bool ehsm_mw_isQueueFull(void);

#ifdef __cplusplus
}
#endif

#endif /* EHSM_MIDDLEWARE_H */
