/**
 * @file keystore_service.hpp
 * @brief KeystoreService implementation header.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "ehsm/services/ikeystore_service.hpp"
#include "ehsm/types/key_slot.hpp"
#include <array>

namespace ehsm::services {

/**
 * @brief KeystoreService implements secure key storage.
 * 
 * Keys are stored in secure memory and never exposed outside
 * the service boundary.
 */
class KeystoreService final : public IKeystoreService {
public:
    /**
     * @brief Default constructor.
     */
    KeystoreService() = default;

    /**
     * @brief Default destructor.
     */
    ~KeystoreService() override = default;

    /**
     * @brief Delete copy constructor.
     */
    KeystoreService(const KeystoreService&) = delete;

    /**
     * @brief Delete copy assignment.
     */
    KeystoreService& operator=(const KeystoreService&) = delete;

    // IKeystoreService implementation
    [[nodiscard]] Status init() override;
    [[nodiscard]] Status deinit() override;
    [[nodiscard]] Status importKey(
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> keyData,
        uint8_t permissions) override;
    [[nodiscard]] Status deleteKey(uint8_t keySlotId) override;
    [[nodiscard]] Status getKey(
        uint8_t keySlotId,
        types::Key& key) override;
    [[nodiscard]] Status getSlotInfo(
        uint8_t keySlotId,
        types::KeySlotInfo& info) override;
    [[nodiscard]] Status clearAll() override;
    [[nodiscard]] bool isSlotOccupied(uint8_t keySlotId) const override;

private:
    /**
     * @brief Securely clear a key slot.
     * @param slotId Slot ID to clear.
     */
    void secureClear(uint8_t slotId);

    /**
     * @brief Check if slot ID is valid.
     * @param slotId Slot ID to validate.
     * @return true if valid, false otherwise.
     */
    [[nodiscard]] constexpr bool isValidSlotId(uint8_t slotId) const {
        return slotId < types::kMaxKeySlots;
    }

    /**
     * @brief Key slot storage.
     */
    struct KeySlotStorage {
        types::KeySlotInfo info;
        types::Key key;
        uint32_t accessCount;
    };

    std::array<KeySlotStorage, types::kMaxKeySlots> keySlots_;  ///< Key slot array
    bool isInitialized_ = false;                                 ///< Initialization flag
};

} // namespace ehsm::services
