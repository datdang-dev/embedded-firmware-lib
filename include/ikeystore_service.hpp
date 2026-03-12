/**
 * @file ikeystore_service.hpp
 * @brief Keystore service interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "status.hpp"
#include "algorithm.hpp"
#include "key_slot.hpp"
#include <span>

namespace ehsm::services {

/**
 * @brief Interface for keystore operations.
 * 
 * Provides secure key storage and retrieval.
 */
class IKeystoreService {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IKeystoreService() = default;

    /**
     * @brief Initialize the keystore.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status init() = 0;

    /**
     * @brief Deinitialize the keystore.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status deinit() = 0;

    /**
     * @brief Import a key into a key slot.
     * @param[in] keySlotId Target slot ID.
     * @param[in] algorithm Algorithm associated with the key.
     * @param[in] keyData Key data.
     * @param[in] permissions Access permission flags.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status importKey(
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> keyData,
        uint8_t permissions) = 0;

    /**
     * @brief Delete a key from a slot.
     * @param[in] keySlotId Slot ID to clear.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status deleteKey(uint8_t keySlotId) = 0;

    /**
     * @brief Get a key from a slot (for crypto operations).
     * @param[in] keySlotId Slot ID to read from.
     * @param[out] key Reference to store key data.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status getKey(
        uint8_t keySlotId,
        types::Key& key) = 0;

    /**
     * @brief Get information about a key slot.
     * @param[in] keySlotId Slot ID to query.
     * @param[out] info Reference to store slot info.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status getSlotInfo(
        uint8_t keySlotId,
        types::KeySlotInfo& info) = 0;

    /**
     * @brief Clear all keys from the keystore.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status clearAll() = 0;

    /**
     * @brief Check if a key slot is occupied.
     * @param[in] keySlotId Slot ID to check.
     * @return true if occupied, false if empty.
     */
    [[nodiscard]] virtual bool isSlotOccupied(uint8_t keySlotId) const = 0;
};

} // namespace ehsm::services
