/**
 * @file isession_manager.hpp
 * @brief Session manager interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "status.hpp"
#include "key_slot.hpp"

namespace ehsm::mw {

/**
 * @brief Interface for session management.
 */
class ISessionManager {
public:
    virtual ~ISessionManager() = default;

    [[nodiscard]] virtual Status init() = 0;
    [[nodiscard]] virtual Status deinit() = 0;
    [[nodiscard]] virtual Status createSession(types::SessionId& sessionId) = 0;
    [[nodiscard]] virtual Status closeSession(types::SessionId sessionId) = 0;
    [[nodiscard]] virtual bool isSessionValid(types::SessionId sessionId) const = 0;
};

} // namespace ehsm::mw
