/**
 * @file hsm_middleware.h
 * @brief Middleware layer: Crypto Job Router and Session Manager.
 * 
 * The middleware layer acts as the central dispatcher for cryptographic jobs.
 * It manages sessions, queues jobs, and routes them to the appropriate
 * service layer components.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef HSM_MIDDLEWARE_H
#define HSM_MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "hsm/hsm_types.h"

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
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Middleware_Init(void);

/**
 * @brief Deinitialize the middleware layer.
 * 
 * Cleans up all sessions and clears the job queue.
 * 
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Middleware_Deinit(void);

/* ============================================================================
 * SESSION MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Create a new session.
 * 
 * @param[out] session_id Pointer to store the new session ID.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Middleware_CreateSession(Hsm_SessionId_t* session_id);

/**
 * @brief Close an existing session.
 * 
 * @param session_id Session ID to close.
 * @return HSM_OK on success, error code otherwise.
 */
Hsm_Status_t Hsm_Middleware_CloseSession(Hsm_SessionId_t session_id);

/**
 * @brief Check if a session ID is valid.
 * 
 * @param session_id Session ID to validate.
 * @return true if valid, false otherwise.
 */
bool Hsm_Middleware_IsSessionValid(Hsm_SessionId_t session_id);

/**
 * @brief Get the session associated with a job.
 * 
 * @param job_id Job ID to query.
 * @return Session ID, or HSM_SESSION_ID_INVALID if not found.
 */
Hsm_SessionId_t Hsm_Middleware_GetJobSession(Hsm_JobId_t job_id);

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
 * @param job Pointer to the job descriptor (modified in-place).
 * @return HSM_OK on success, error code otherwise.
 * 
 * @pre Job must have valid operation, algorithm, and key_slot_id
 * @post Job result and is_complete fields are updated
 */
Hsm_Status_t Hsm_Middleware_DispatchJob(Hsm_CryptoJob_t* job);

/**
 * @brief Get the current job queue depth.
 * 
 * @return Number of jobs currently in the queue.
 */
size_t Hsm_Middleware_GetQueueDepth(void);

/**
 * @brief Check if the job queue is full.
 * 
 * @return true if queue is full, false otherwise.
 */
bool Hsm_Middleware_IsQueueFull(void);

#ifdef __cplusplus
}
#endif

#endif /* HSM_MIDDLEWARE_H */
