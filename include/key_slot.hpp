/**
 * @file key_slot.hpp
 * @brief Key slot type definitions.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "algorithm.hpp"
#include <cstdint>
#include <cstddef>

namespace ehsm::types {

/**
 * @brief Maximum key size in bytes (512-bit RSA).
 */
inline constexpr size_t kMaxKeySize = 64U;

/**
 * @brief Maximum number of key slots.
 */
inline constexpr uint8_t kMaxKeySlots = 8U;

/**
 * @brief Maximum number of sessions.
 */
inline constexpr uint8_t kMaxSessions = 4U;

/**
 * @brief Invalid key slot ID.
 */
inline constexpr uint8_t kInvalidKeySlotId = 0xFFU;

/**
 * @brief Cryptographic key storage structure.
 * 
 * Keys are stored securely and never exposed outside the keystore.
 */
struct Key {
    uint8_t data[kMaxKeySize];    ///< Key data buffer
    size_t size;                   ///< Actual key size in bytes
    Algorithm algorithm;           ///< Algorithm associated with key

    /**
     * @brief Default constructor (zeroes all data).
     */
    constexpr Key() : data{}, size(0U), algorithm(Algorithm::NONE) {}

    /**
     * @brief Secure clear the key data.
     */
    void secureClear() {
        volatile uint8_t* ptr = data;
        for (size_t i = 0U; i < kMaxKeySize; ++i) {
            ptr[i] = 0U;
        }
        size = 0U;
        algorithm = Algorithm::NONE;
    }
};

/**
 * @brief Key slot information structure.
 */
struct KeySlotInfo {
    uint8_t id;                  ///< Slot ID (0-based)
    bool isOccupied;             ///< Slot is occupied
    Algorithm algorithm;         ///< Algorithm associated with key
    uint8_t permissions;         ///< Access permission flags
    size_t keySize;              ///< Key size in bytes

    /**
     * @brief Default constructor.
     */
    constexpr KeySlotInfo()
        : id(0U)
        , isOccupied(false)
        , algorithm(Algorithm::NONE)
        , permissions(0U)
        , keySize(0U) {}
};

/**
 * @brief Session ID type.
 */
using SessionId = uint32_t;

/**
 * @brief Invalid session ID constant.
 */
inline constexpr SessionId kInvalidSessionId = 0xFFFFFFFFU;

/**
 * @brief Job ID type.
 */
using JobId = uint32_t;

/**
 * @brief Invalid job ID constant.
 */
inline constexpr JobId kInvalidJobId = 0xFFFFFFFFU;

} // namespace ehsm::types
