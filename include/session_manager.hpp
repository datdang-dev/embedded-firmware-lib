/**
 * @file session_manager.hpp
 * @brief Session manager implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "common.hpp"
#include "isession_manager.hpp"
#include "key_slot.hpp"
#include <array>

namespace ehsm::mw {

/**
 * @brief Session manager implementation.
 */
class SessionManager final : public ISessionManager {
public:
    SessionManager() = default;
    ~SessionManager() override = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    [[nodiscard]] Status init() override;
    [[nodiscard]] Status deinit() override;
    [[nodiscard]] Status createSession(types::SessionId& sessionId) override;
    [[nodiscard]] Status closeSession(types::SessionId sessionId) override;
    [[nodiscard]] bool isSessionValid(types::SessionId sessionId) const override;

private:
    struct Session {
        types::SessionId id{types::kInvalidSessionId};
        bool isActive{false};
        uint32_t jobCount{0U};
    };

    std::array<Session, types::kMaxSessions> sessions_;
    bool isInitialized_{false};
    types::SessionId nextSessionId_{1U};
};

} // namespace ehsm::mw
