/**
 * @file hsm_middleware.c
 * @brief Implementation of the HSM middleware layer.
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
#include "hsm/hsm_middleware.h"
#include "hsm/hsm_crypto_service.h"
#include <string.h>

/* ============================================================================
 * PRIVATE TYPE DEFINITIONS
 * ============================================================================
 */

/**
 * @brief Session descriptor for tracking active sessions.
 */
typedef struct Hsm_Session
{
    Hsm_SessionId_t id;         /**< Session identifier */
    bool            is_active;  /**< Session is active */
    uint32_t        job_count;  /**< Number of jobs processed */
} Hsm_Session_t;

/**
 * @brief Job queue entry.
 */
typedef struct Hsm_JobQueueEntry
{
    Hsm_CryptoJob_t* job;       /**< Pointer to job descriptor */
    bool             is_valid;  /**< Entry is valid */
} Hsm_JobQueueEntry_t;

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */

/** Session magic number for validation */
#define SESSION_MAGIC           (0x53455353U)  /* "SESS" */

/** Initial session ID value */
#define SESSION_ID_START        (0x00000001U)

/** Initial job ID value */
#define JOB_ID_START            (0x00000001U)

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */

/** Session table */
static Hsm_Session_t g_sessions[HSM_MAX_SESSIONS];

/** Job queue */
static Hsm_JobQueueEntry_t g_job_queue[HSM_MAX_JOB_QUEUE_DEPTH];

/** Current job queue head (for circular queue) */
static size_t g_queue_head = 0U;

/** Current job queue tail */
static size_t g_queue_tail = 0U;

/** Number of jobs in queue */
static size_t g_queue_count = 0U;

/** Next session ID counter */
static Hsm_SessionId_t g_next_session_id = SESSION_ID_START;

/** Next job ID counter */
static Hsm_JobId_t g_next_job_id = JOB_ID_START;

/** Middleware initialization flag */
static bool g_middleware_initialized = false;

/* ============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * ============================================================================
 */

/**
 * @brief Find a session by ID.
 * 
 * @param session_id Session ID to find.
 * @return Pointer to session, or NULL if not found.
 */
static Hsm_Session_t* Middleware_FindSession(Hsm_SessionId_t session_id);

/**
 * @brief Allocate a new session slot.
 * 
 * @return Pointer to allocated session, or NULL if none available.
 */
static Hsm_Session_t* Middleware_AllocateSession(void);

/**
 * @brief Generate a unique job ID.
 * 
 * @return New job ID.
 */
static Hsm_JobId_t Middleware_GenerateJobId(void);

/**
 * @brief Add a job to the queue.
 * 
 * @param job Job to queue.
 * @return HSM_OK on success, error code otherwise.
 */
static Hsm_Status_t Middleware_QueueJob(Hsm_CryptoJob_t* job);

/**
 * @brief Process a job from the queue.
 * 
 * @param job Job to process.
 * @return HSM_OK on success, error code otherwise.
 */
static Hsm_Status_t Middleware_ProcessJob(Hsm_CryptoJob_t* job);

/* ============================================================================
 * INITIALIZATION IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Middleware_Init(void)
{
    /* Check if already initialized */
    if (g_middleware_initialized)
    {
        return HSM_OK;
    }
    
    /* Initialize sessions */
    (void)memset(g_sessions, 0, sizeof(g_sessions));
    for (size_t i = 0U; i < HSM_MAX_SESSIONS; i++)
    {
        g_sessions[i].id = HSM_SESSION_ID_INVALID;
    }
    
    /* Initialize job queue */
    (void)memset(g_job_queue, 0, sizeof(g_job_queue));
    g_queue_head = 0U;
    g_queue_tail = 0U;
    g_queue_count = 0U;
    
    /* Reset ID counters */
    g_next_session_id = SESSION_ID_START;
    g_next_job_id = JOB_ID_START;
    
    g_middleware_initialized = true;
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Middleware_Deinit(void)
{
    /* Check if initialized */
    if (!g_middleware_initialized)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Clear all sessions */
    for (size_t i = 0U; i < HSM_MAX_SESSIONS; i++)
    {
        g_sessions[i].id = HSM_SESSION_ID_INVALID;
        g_sessions[i].is_active = false;
    }
    
    /* Clear job queue */
    (void)memset(g_job_queue, 0, sizeof(g_job_queue));
    g_queue_head = 0U;
    g_queue_tail = 0U;
    g_queue_count = 0U;
    
    g_middleware_initialized = false;
    
    return HSM_OK;
}

/* ============================================================================
 * SESSION MANAGEMENT IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Middleware_CreateSession(Hsm_SessionId_t* session_id)
{
    Hsm_Session_t* session;
    
    /* Validate output parameter */
    if (session_id == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if initialized */
    if (!g_middleware_initialized)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Allocate a new session */
    session = Middleware_AllocateSession();
    if (session == NULL)
    {
        return HSM_ERR_SESSION_INVALID;  /* No sessions available */
    }
    
    /* Initialize session */
    session->id = g_next_session_id++;
    session->is_active = true;
    session->job_count = 0U;
    
    /* Return session ID */
    *session_id = session->id;
    
    return HSM_OK;
}

Hsm_Status_t Hsm_Middleware_CloseSession(Hsm_SessionId_t session_id)
{
    Hsm_Session_t* session;
    
    /* Check if initialized */
    if (!g_middleware_initialized)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Find and close the session */
    session = Middleware_FindSession(session_id);
    if (session == NULL)
    {
        return HSM_ERR_SESSION_INVALID;
    }
    
    /* Mark session as inactive */
    session->is_active = false;
    session->id = HSM_SESSION_ID_INVALID;
    
    return HSM_OK;
}

bool Hsm_Middleware_IsSessionValid(Hsm_SessionId_t session_id)
{
    Hsm_Session_t* session;
    
    /* Check if initialized */
    if (!g_middleware_initialized)
    {
        return false;
    }
    
    /* Find session */
    session = Middleware_FindSession(session_id);
    
    return (session != NULL) && session->is_active;
}

Hsm_SessionId_t Hsm_Middleware_GetJobSession(Hsm_JobId_t job_id)
{
    /* This is a placeholder - in a real implementation,
       we would track job-to-session mappings */
    (void)job_id;
    return HSM_SESSION_ID_INVALID;
}

/* ============================================================================
 * JOB DISPATCHING IMPLEMENTATION
 * ============================================================================
 */

Hsm_Status_t Hsm_Middleware_DispatchJob(Hsm_CryptoJob_t* job)
{
    Hsm_Status_t status;
    
    /* Validate job parameter */
    if (job == NULL)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Check if initialized */
    if (!g_middleware_initialized)
    {
        return HSM_ERR_NOT_INITIALIZED;
    }
    
    /* Validate job operation */
    if (job->operation == HSM_CRYPTO_OP_NONE)
    {
        return HSM_ERR_INVALID_PARAM;
    }
    
    /* Validate session */
    if (!Hsm_Middleware_IsSessionValid(job->session_id))
    {
        return HSM_ERR_SESSION_INVALID;
    }
    
    /* Assign job ID */
    job->job_id = Middleware_GenerateJobId();
    
    /* Queue the job */
    status = Middleware_QueueJob(job);
    if (status != HSM_OK)
    {
        return status;
    }
    
    /* Process the job synchronously (for this PoC) */
    status = Middleware_ProcessJob(job);
    
    /* Mark job as complete */
    job->is_complete = true;
    job->result = status;
    
    /* Update session job count */
    Hsm_Session_t* session = Middleware_FindSession(job->session_id);
    if (session != NULL)
    {
        session->job_count++;
    }
    
    return status;
}

size_t Hsm_Middleware_GetQueueDepth(void)
{
    return g_queue_count;
}

bool Hsm_Middleware_IsQueueFull(void)
{
    return (g_queue_count >= HSM_MAX_JOB_QUEUE_DEPTH);
}

/* ============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */

static Hsm_Session_t* Middleware_FindSession(Hsm_SessionId_t session_id)
{
    for (size_t i = 0U; i < HSM_MAX_SESSIONS; i++)
    {
        if ((g_sessions[i].id == session_id) && g_sessions[i].is_active)
        {
            return &g_sessions[i];
        }
    }
    return NULL;
}

static Hsm_Session_t* Middleware_AllocateSession(void)
{
    for (size_t i = 0U; i < HSM_MAX_SESSIONS; i++)
    {
        if (!g_sessions[i].is_active)
        {
            return &g_sessions[i];
        }
    }
    return NULL;
}

static Hsm_JobId_t Middleware_GenerateJobId(void)
{
    Hsm_JobId_t job_id = g_next_job_id++;
    
    /* Handle wraparound */
    if (g_next_job_id == HSM_JOB_ID_INVALID)
    {
        g_next_job_id = JOB_ID_START;
    }
    
    return job_id;
}

static Hsm_Status_t Middleware_QueueJob(Hsm_CryptoJob_t* job)
{
    /* Check if queue is full */
    if (g_queue_count >= HSM_MAX_JOB_QUEUE_DEPTH)
    {
        return HSM_ERR_QUEUE_FULL;
    }
    
    /* Add job to queue */
    g_job_queue[g_queue_tail].job = job;
    g_job_queue[g_queue_tail].is_valid = true;
    
    /* Update tail pointer (circular) */
    g_queue_tail = (g_queue_tail + 1U) % HSM_MAX_JOB_QUEUE_DEPTH;
    g_queue_count++;
    
    return HSM_OK;
}

static Hsm_Status_t Middleware_ProcessJob(Hsm_CryptoJob_t* job)
{
    Hsm_Status_t status;
    
    /* Route job based on operation type */
    switch (job->operation)
    {
        case HSM_CRYPTO_OP_ENCRYPT:
            status = Hsm_CryptoService_Encrypt(
                job->algorithm,
                job->key_slot_id,
                &job->input,
                &job->output
            );
            break;
            
        case HSM_CRYPTO_OP_DECRYPT:
            status = Hsm_CryptoService_Decrypt(
                job->algorithm,
                job->key_slot_id,
                &job->input,
                &job->output
            );
            break;
            
        case HSM_CRYPTO_OP_SIGN:
        case HSM_CRYPTO_OP_VERIFY:
        case HSM_CRYPTO_OP_HASH:
        case HSM_CRYPTO_OP_KEY_GEN:
            /* Not implemented in this PoC */
            status = HSM_ERR_UNSUPPORTED;
            break;
            
        default:
            status = HSM_ERR_INVALID_PARAM;
            break;
    }
    
    /* Log job completion via UART (for tracing) */
    if (status == HSM_OK)
    {
        /* Success - output length is set by crypto service */
    }
    
    return status;
}
