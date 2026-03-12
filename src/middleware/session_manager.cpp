/**
 * @file session_manager.cpp
 * @brief Session manager implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "ehsm/middleware/session_manager.hpp"

namespace ehsm::mw {

Status SessionManager::init() {
    if (isInitialized_) {
        return Status(types::StatusCode::OK);
    }

    for (auto& session : sessions_) {
        session.id = types::kInvalidSessionId;
        session.isActive = false;
        session.jobCount = 0U;
    }

    nextSessionId_ = 1U;
    isInitialized_ = true;
    return Status(types::StatusCode::OK);
}

Status SessionManager::deinit() {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    for (auto& session : sessions_) {
        session.id = types::kInvalidSessionId;
        session.isActive = false;
        session.jobCount = 0U;
    }

    isInitialized_ = false;
    return Status(types::StatusCode::OK);
}

Status SessionManager::createSession(types::SessionId& sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    // Find available slot
    for (auto& session : sessions_) {
        if (!session.isActive) {
            session.id = nextSessionId_++;
            session.isActive = true;
            session.jobCount = 0U;
            sessionId = session.id;

            // Handle wraparound
            if (nextSessionId_ == types::kInvalidSessionId) {
                nextSessionId_ = 1U;
            }

            return Status(types::StatusCode::OK);
        }
    }

    return Status(types::StatusCode::ERR_SESSION_INVALID);
}

Status SessionManager::closeSession(types::SessionId sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    for (auto& session : sessions_) {
        if (session.id == sessionId && session.isActive) {
            session.isActive = false;
            session.id = types::kInvalidSessionId;
            return Status(types::StatusCode::OK);
        }
    }

    return Status(types::StatusCode::ERR_SESSION_INVALID);
}

bool SessionManager::isSessionValid(types::SessionId sessionId) const {
    if (!isInitialized_) {
        return false;
    }

    for (const auto& session : sessions_) {
        if (session.id == sessionId && session.isActive) {
            return true;
        }
    }

    return false;
}

} // namespace ehsm::mw
