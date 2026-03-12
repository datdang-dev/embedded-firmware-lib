/**
 * @file ehsm_middleware.c
 * @brief Implementation of the Embedded HSM middleware layer.
 * 
 * Implements session management, job queuing, and crypto job routing.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm/ehsm_middleware.h"
#include "ehsm/ehsm_crypto.h"
#include <string.h>

/* ============================================================================
 * PRIVATE TYPE DEFINITIONS
 * ============================================================================
 */

/**
 * @brief Session descriptor for tracking active sessions.
 */
typedef struct EhsmSession
{
    ehsm_sessionId_t id;
    bool             isActive;
    uint32_t         jobCount;
} ehsm_session_t;

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

#define SESSION_MAGIC           (0x53455353U)
#define SESSION_ID_START        (0x00000001U)
#define JOB_ID_START            (0x00000001U)

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

static ehsm_session_t g_sessions[EHSM_MAX_SESSIONS];
static ehsm_cryptoJob_t g_jobQueue[EHSM_MAX_JOB_QUEUE_DEPTH];
static size_t g_queueHead = 0U;
static size_t g_queueTail = 0U;
static size_t g_queueCount = 0U;
static ehsm_sessionId_t g_nextSessionId = SESSION_ID_START;
static ehsm_jobId_t g_nextJobId = JOB_ID_START;
static bool g_middlewareInitialized = false;

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================
 */

static ehsm_session_t* findSession(ehsm_sessionId_t sessionId)
{
    for (size_t i = 0U; i < EHSM_MAX_SESSIONS; i++)
    {
        if ((g_sessions[i].id == sessionId) && g_sessions[i].isActive)
        {
            return &g_sessions[i];
        }
    }
    return NULL;
}

static ehsm_session_t* allocateSession(void)
{
    for (size_t i = 0U; i < EHSM_MAX_SESSIONS; i++)
    {
        if (!g_sessions[i].isActive)
        {
            return &g_sessions[i];
        }
    }
    return NULL;
}

static ehsm_jobId_t generateJobId(void)
{
    ehsm_jobId_t jobId = g_nextJobId++;
    if (g_nextJobId == EHSM_JOB_ID_INVALID)
    {
        g_nextJobId = JOB_ID_START;
    }
    return jobId;
}

static ehsm_status_t processJob(ehsm_cryptoJob_t* job)
{
    ehsm_status_t status;
    
    switch (job->operation)
    {
        case EHSM_CRYPTO_OP_ENCRYPT:
            status = ehsm_crypto_encrypt(
                job->algorithm,
                job->keySlotId,
                &job->input,
                &job->output
            );
            break;
            
        case EHSM_CRYPTO_OP_DECRYPT:
            status = ehsm_crypto_decrypt(
                job->algorithm,
                job->keySlotId,
                &job->input,
                &job->output
            );
            break;
            
        default:
            status = EHSM_STATUS_ERR_INVALID_PARAM;
            break;
    }
    
    return status;
}

/* ============================================================================
 * PUBLIC IMPLEMENTATION
 * ============================================================================
 */

ehsm_status_t ehsm_mw_init(void)
{
    if (g_middlewareInitialized)
    {
        return EHSM_STATUS_OK;
    }
    
    (void)memset(g_sessions, 0, sizeof(g_sessions));
    for (size_t i = 0U; i < EHSM_MAX_SESSIONS; i++)
    {
        g_sessions[i].id = EHSM_SESSION_ID_INVALID;
    }
    
    (void)memset(g_jobQueue, 0, sizeof(g_jobQueue));
    g_queueHead = 0U;
    g_queueTail = 0U;
    g_queueCount = 0U;
    g_nextSessionId = SESSION_ID_START;
    g_nextJobId = JOB_ID_START;
    g_middlewareInitialized = true;
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mw_deinit(void)
{
    if (!g_middlewareInitialized)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    for (size_t i = 0U; i < EHSM_MAX_SESSIONS; i++)
    {
        g_sessions[i].id = EHSM_SESSION_ID_INVALID;
        g_sessions[i].isActive = false;
    }
    
    (void)memset(g_jobQueue, 0, sizeof(g_jobQueue));
    g_queueHead = 0U;
    g_queueTail = 0U;
    g_queueCount = 0U;
    g_middlewareInitialized = false;
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mw_createSession(ehsm_sessionId_t* sessionId)
{
    ehsm_session_t* session;
    
    if (sessionId == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (!g_middlewareInitialized)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    session = allocateSession();
    if (session == NULL)
    {
        return EHSM_STATUS_ERR_SESSION_INVALID;
    }
    
    session->id = g_nextSessionId++;
    session->isActive = true;
    session->jobCount = 0U;
    *sessionId = session->id;
    
    return EHSM_STATUS_OK;
}

ehsm_status_t ehsm_mw_closeSession(ehsm_sessionId_t sessionId)
{
    ehsm_session_t* session;
    
    if (!g_middlewareInitialized)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    session = findSession(sessionId);
    if (session == NULL)
    {
        return EHSM_STATUS_ERR_SESSION_INVALID;
    }
    
    session->isActive = false;
    session->id = EHSM_SESSION_ID_INVALID;
    
    return EHSM_STATUS_OK;
}

bool ehsm_mw_isSessionValid(ehsm_sessionId_t sessionId)
{
    if (!g_middlewareInitialized)
    {
        return false;
    }
    
    ehsm_session_t* session = findSession(sessionId);
    return (session != NULL) && session->isActive;
}

ehsm_status_t ehsm_mw_dispatchJob(ehsm_cryptoJob_t* job)
{
    ehsm_status_t status;
    
    if (job == NULL)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (!g_middlewareInitialized)
    {
        return EHSM_STATUS_ERR_NOT_INITIALIZED;
    }
    
    if (job->operation == EHSM_CRYPTO_OP_NONE)
    {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    if (!ehsm_mw_isSessionValid(job->sessionId))
    {
        return EHSM_STATUS_ERR_SESSION_INVALID;
    }
    
    job->jobId = generateJobId();
    
    if (g_queueCount >= EHSM_MAX_JOB_QUEUE_DEPTH)
    {
        return EHSM_STATUS_ERR_QUEUE_FULL;
    }
    
    g_jobQueue[g_queueTail] = *job;
    g_queueTail = (g_queueTail + 1U) % EHSM_MAX_JOB_QUEUE_DEPTH;
    g_queueCount++;
    
    status = processJob(job);
    job->isComplete = true;
    job->result = status;
    
    ehsm_session_t* session = findSession(job->sessionId);
    if (session != NULL)
    {
        session->jobCount++;
    }
    
    return status;
}

size_t ehsm_mw_getQueueDepth(void)
{
    return g_queueCount;
}

bool ehsm_mw_isQueueFull(void)
{
    return (g_queueCount >= EHSM_MAX_JOB_QUEUE_DEPTH);
}
