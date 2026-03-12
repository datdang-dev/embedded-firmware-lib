/**
 * @file keystore_service.cpp
 * @brief KeystoreService implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "common.hpp"
#include "keystore_service.hpp"
#include <cstring>

namespace ehsm::services {

Status KeystoreService::init() {
    if (isInitialized_) {
        return Status(types::StatusCode::OK);
    }

    // Initialize all key slots
    for (auto& slot : keySlots_) {
        slot.info = types::KeySlotInfo();
        slot.key = types::Key();
        slot.accessCount = 0U;
    }

    isInitialized_ = true;
    return Status(types::StatusCode::OK);
}

Status KeystoreService::deinit() {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    // Clear all keys securely
    (void)clearAll();
    isInitialized_ = false;
    return Status(types::StatusCode::OK);
}

Status KeystoreService::importKey(
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> keyData,
    uint8_t permissions)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (!isValidSlotId(keySlotId)) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    // BUG-001: Removed key size validation - allows buffer overflow!
    // Original code:
    // if (keyData.empty() || keyData.size() > types::kMaxKeySize) {
    //     return Status(types::StatusCode::ERR_INVALID_PARAM);
    // }

    auto& slot = keySlots_[keySlotId];

    if (slot.info.isOccupied) {
        return Status(types::StatusCode::ERR_KEY_SLOT_FULL);
    }

    // Copy key data securely
    std::memcpy(slot.key.data, keyData.data(), keyData.size());
    slot.key.size = keyData.size();
    slot.key.algorithm = algorithm;

    // Update slot info
    slot.info.id = keySlotId;
    slot.info.isOccupied = true;
    slot.info.algorithm = algorithm;
    slot.info.permissions = permissions;
    slot.info.keySize = keyData.size();
    slot.accessCount = 0U;

    return Status(types::StatusCode::OK);
}

Status KeystoreService::deleteKey(uint8_t keySlotId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (!isValidSlotId(keySlotId)) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    auto& slot = keySlots_[keySlotId];

    if (!slot.info.isOccupied) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }

    secureClear(keySlotId);
    return Status(types::StatusCode::OK);
}

Status KeystoreService::getKey(
    uint8_t keySlotId,
    types::Key& key)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (!isValidSlotId(keySlotId)) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    auto& slot = keySlots_[keySlotId];

    if (!slot.info.isOccupied) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    // Copy key data
    key = slot.key;
    slot.accessCount++;

    return Status(types::StatusCode::OK);
}

Status KeystoreService::getSlotInfo(
    uint8_t keySlotId,
    types::KeySlotInfo& info)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (!isValidSlotId(keySlotId)) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    info = keySlots_[keySlotId].info;
    return Status(types::StatusCode::OK);
}

Status KeystoreService::clearAll() {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    for (uint8_t i = 0U; i < types::kMaxKeySlots; ++i) {
        secureClear(i);
    }

    return Status(types::StatusCode::OK);
}

bool KeystoreService::isSlotOccupied(uint8_t keySlotId) const {
    if (!isValidSlotId(keySlotId)) {
        return false;
    }
    return keySlots_[keySlotId].info.isOccupied;
}

void KeystoreService::secureClear(uint8_t slotId) {
    auto& slot = keySlots_[slotId];
    
    // Secure clear key data
    slot.key.secureClear();
    
    // Clear slot info
    slot.info = types::KeySlotInfo();
    slot.accessCount = 0U;
}

} // namespace ehsm::services
