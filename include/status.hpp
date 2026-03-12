/**
 * @file status.hpp
 * @brief Status class for error handling without exceptions.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include <cstdint>

namespace ehsm::types {

/**
 * @brief Status codes for HSM operations.
 */
enum class StatusCode : int32_t {
    OK = 0,
    ERR_INVALID_PARAM = -1,
    ERR_INSUFFICIENT_BUFFER = -2,
    ERR_CRYPTO_FAILED = -3,
    ERR_SESSION_INVALID = -4,
    ERR_NOT_INITIALIZED = -5,
    ERR_KEY_SLOT_FULL = -6,
    ERR_INVALID_KEY_ID = -7,
    ERR_AUTH_FAILED = -8,
    ERR_QUEUE_FULL = -9,
    ERR_HARDWARE = -10,
    ERR_UNSUPPORTED = -11,
    ERR_TIMEOUT = -12,
    ERR_INTERNAL = -13,
    ERR_BUSY = -14
};

/**
 * @brief Status class for error handling (no exceptions).
 * 
 * This class provides type-safe error handling without using C++ exceptions.
 * Usage:
 * @code
 * Status result = someOperation();
 * if (!result.isOk()) {
 *     handleError(result);
 * }
 * @endcode
 */
class Status {
public:
    /**
     * @brief Default constructor (OK status).
     */
    constexpr Status() noexcept : code_(StatusCode::OK) {}

    /**
     * @brief Construct from StatusCode.
     * @param code Status code to wrap.
     */
    constexpr explicit Status(StatusCode code) noexcept : code_(code) {}

    /**
     * @brief Check if status is OK.
     * @return true if status is OK, false otherwise.
     */
    [[nodiscard]] constexpr bool isOk() const noexcept {
        return code_ == StatusCode::OK;
    }

    /**
     * @brief Get the status code.
     * @return The status code.
     */
    [[nodiscard]] constexpr StatusCode code() const noexcept {
        return code_;
    }

    /**
     * @brief Get human-readable message.
     * @return Error message string.
     */
    [[nodiscard]] const char* message() const;

    /**
     * @brief Explicit bool conversion (true if OK).
     * @return true if status is OK, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return isOk();
    }

    /**
     * @brief Equality comparison.
     * @param other Other status to compare.
     * @return true if equal, false otherwise.
     */
    [[nodiscard]] constexpr bool operator==(const Status& other) const noexcept {
        return code_ == other.code_;
    }

    /**
     * @brief Inequality comparison.
     * @param other Other status to compare.
     * @return true if not equal, false otherwise.
     */
    [[nodiscard]] constexpr bool operator!=(const Status& other) const noexcept {
        return code_ != other.code_;
    }

private:
    StatusCode code_;
};

/**
 * @brief Get string representation of StatusCode.
 * @param code Status code to convert.
 * @return String representation.
 */
[[nodiscard]] const char* toString(StatusCode code);

/**
 * @brief Get string representation of Status.
 * @param status Status to convert.
 * @return String representation.
 */
[[nodiscard]] inline const char* toString(const Status& status) {
    return toString(status.code());
}

} // namespace ehsm::types
