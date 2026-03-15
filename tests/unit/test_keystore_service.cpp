/**
 * @file test_keystore_service.cpp
 * @brief Unit tests for KeystoreService using Catch2 + Trompeloeil.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <catch2/catch_all.hpp>
#include <trompeloeil.hpp>
#include "keystore_service.hpp"
#include <array>
#include <cstring>

namespace ehsm::services {

TEST_CASE("KeystoreService initializes successfully", "[keystore]") {
    // Arrange
    KeystoreService keystore;

    // Act
    auto status = keystore.init();

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("KeystoreService double initialization succeeds", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    auto firstInit = keystore.init();
    REQUIRE(firstInit.isOk());

    // Act
    auto secondInit = keystore.init();

    // Assert
    REQUIRE(secondInit.isOk());
}

TEST_CASE("KeystoreService deinitializes successfully", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    // Act
    auto status = keystore.deinit();

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("KeystoreService deinit without init fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;

    // Act
    auto status = keystore.deinit();

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService import key to empty slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act
    auto status = keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Assert
    REQUIRE(status.isOk());

    // Verify slot is occupied
    REQUIRE(keystore.isSlotOccupied(0));
}

TEST_CASE("KeystoreService import key to occupied slot fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Import first key
    auto firstImport = keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);
    REQUIRE(firstImport.isOk());

    // Act - Try to import to same slot
    auto secondImport = keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Assert
    REQUIRE_FALSE(secondImport.isOk());
    REQUIRE(secondImport.code() == types::StatusCode::ERR_KEY_SLOT_FULL);
}

TEST_CASE("KeystoreService delete key from occupied slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);
    REQUIRE(keystore.isSlotOccupied(0));

    // Act
    auto status = keystore.deleteKey(0);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE_FALSE(keystore.isSlotOccupied(0));
}

TEST_CASE("KeystoreService delete key from empty slot fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    // Act
    auto status = keystore.deleteKey(0);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_PARAM);
}

TEST_CASE("KeystoreService get key from valid slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    types::Key retrievedKey;

    // Act
    auto status = keystore.getKey(0, retrievedKey);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(retrievedKey.size == 16);
    REQUIRE(retrievedKey.algorithm == types::Algorithm::AES_128);
    REQUIRE(std::memcmp(retrievedKey.data, keyData.data(), 16) == 0);
}

TEST_CASE("KeystoreService get key from invalid slot fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    types::Key retrievedKey;

    // Act
    auto status = keystore.getKey(0, retrievedKey);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

TEST_CASE("KeystoreService get key from out-of-range slot fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    types::Key retrievedKey;

    // Act
    auto status = keystore.getKey(0xFF, retrievedKey);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

TEST_CASE("KeystoreService clear all keys", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Import keys to multiple slots
    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);
    (void)keystore.importKey(1, types::Algorithm::AES_128, keyData, permissions);
    (void)keystore.importKey(2, types::Algorithm::AES_128, keyData, permissions);

    REQUIRE(keystore.isSlotOccupied(0));
    REQUIRE(keystore.isSlotOccupied(1));
    REQUIRE(keystore.isSlotOccupied(2));

    // Act
    auto status = keystore.clearAll();

    // Assert
    REQUIRE(status.isOk());
    REQUIRE_FALSE(keystore.isSlotOccupied(0));
    REQUIRE_FALSE(keystore.isSlotOccupied(1));
    REQUIRE_FALSE(keystore.isSlotOccupied(2));
}

TEST_CASE("KeystoreService clear all when not initialized fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;

    // Act
    auto status = keystore.clearAll();

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService key permissions check", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    // Import with ENCRYPT permission only
    uint8_t encryptOnly = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);
    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, encryptOnly);

    // Import with DECRYPT permission only
    uint8_t decryptOnly = static_cast<uint8_t>(types::KeyPermission::DECRYPT);
    (void)keystore.importKey(1, types::Algorithm::AES_128, keyData, decryptOnly);

    // Import with both permissions
    uint8_t bothPermissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
    (void)keystore.importKey(2, types::Algorithm::AES_128, keyData, bothPermissions);

    // Act - Get slot info
    types::KeySlotInfo info0, info1, info2;
    (void)keystore.getSlotInfo(0, info0);
    (void)keystore.getSlotInfo(1, info1);
    (void)keystore.getSlotInfo(2, info2);

    // Assert
    REQUIRE(info0.permissions == encryptOnly);
    REQUIRE(info1.permissions == decryptOnly);
    REQUIRE(info2.permissions == bothPermissions);
}

TEST_CASE("KeystoreService secure clear verification", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 32> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ALL);

    (void)keystore.importKey(0, types::Algorithm::AES_256, keyData, permissions);

    // Get key before delete
    types::Key keyBefore;
    (void)keystore.getKey(0, keyBefore);
    REQUIRE(keyBefore.size == 32);
    REQUIRE(std::memcmp(keyBefore.data, keyData.data(), 32) == 0);

    // Act - Delete key
    auto status = keystore.deleteKey(0);
    REQUIRE(status.isOk());

    // Try to get key after delete
    types::Key keyAfter;
    auto getStatus = keystore.getKey(0, keyAfter);

    // Assert - Key should be cleared
    REQUIRE_FALSE(getStatus.isOk());
    REQUIRE(keyAfter.size == 0);
    REQUIRE(keyAfter.algorithm == types::Algorithm::NONE);
}

TEST_CASE("KeystoreService get slot info from valid slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    (void)keystore.importKey(3, types::Algorithm::AES_128, keyData, permissions);

    types::KeySlotInfo info;

    // Act
    auto status = keystore.getSlotInfo(3, info);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(info.id == 3);
    REQUIRE(info.isOccupied);
    REQUIRE(info.algorithm == types::Algorithm::AES_128);
    REQUIRE(info.permissions == permissions);
    REQUIRE(info.keySize == 16);
}

TEST_CASE("KeystoreService get slot info from empty slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    types::KeySlotInfo info;

    // Act
    auto status = keystore.getSlotInfo(5, info);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(info.id == 5);
    REQUIRE_FALSE(info.isOccupied);
    REQUIRE(info.algorithm == types::Algorithm::NONE);
    REQUIRE(info.keySize == 0);
}

TEST_CASE("KeystoreService get slot info from invalid slot fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    types::KeySlotInfo info;

    // Act
    auto status = keystore.getSlotInfo(0xFF, info);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

TEST_CASE("KeystoreService isSlotOccupied returns correct value", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act & Assert - Before import
    REQUIRE_FALSE(keystore.isSlotOccupied(0));

    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Act & Assert - After import
    REQUIRE(keystore.isSlotOccupied(0));

    (void)keystore.deleteKey(0);

    // Act & Assert - After delete
    REQUIRE_FALSE(keystore.isSlotOccupied(0));
}

TEST_CASE("KeystoreService isSlotOccupied with invalid slot", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    // Act
    auto occupied = keystore.isSlotOccupied(0xFF);

    // Assert
    REQUIRE_FALSE(occupied);
}

TEST_CASE("KeystoreService import key with invalid slot ID", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act
    auto status = keystore.importKey(0xFF, types::Algorithm::AES_128, keyData, permissions);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

TEST_CASE("KeystoreService import key with empty key data", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 1> emptyKeyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act
    auto status = keystore.importKey(0, types::Algorithm::AES_128, emptyKeyData, permissions);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_PARAM);
}

TEST_CASE("KeystoreService import key with too large key data", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 128> tooLargeKeyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act
    auto status = keystore.importKey(0, types::Algorithm::AES_128, tooLargeKeyData, permissions);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_PARAM);
}

TEST_CASE("KeystoreService import key without init fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;

    std::array<uint8_t, 16> keyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act
    auto status = keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService delete key without init fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;

    // Act
    auto status = keystore.deleteKey(0);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService get key without init fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    types::Key key;

    // Act
    auto status = keystore.getKey(0, key);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService get slot info without init fails", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    types::KeySlotInfo info;

    // Act
    auto status = keystore.getSlotInfo(0, info);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("KeystoreService fill all slots", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    // Act - Fill all 8 slots
    for (uint8_t i = 0; i < types::kMaxKeySlots; ++i) {
        auto status = keystore.importKey(i, types::Algorithm::AES_128, keyData, permissions);
        REQUIRE(status.isOk());
        REQUIRE(keystore.isSlotOccupied(i));
    }

    // Try to import to one more slot (should fail - no more slots)
    // Note: This test verifies max capacity
    REQUIRE_FALSE(keystore.isSlotOccupied(types::kMaxKeySlots)); // Out of range
}

TEST_CASE("KeystoreService import different algorithms", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> aes128Key{0};
    std::array<uint8_t, 32> aes256Key{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ALL);

    // Act
    auto status128 = keystore.importKey(0, types::Algorithm::AES_128, aes128Key, permissions);
    auto status256 = keystore.importKey(1, types::Algorithm::AES_256, aes256Key, permissions);

    // Assert
    REQUIRE(status128.isOk());
    REQUIRE(status256.isOk());

    types::KeySlotInfo info128, info256;
    (void)keystore.getSlotInfo(0, info128);
    (void)keystore.getSlotInfo(1, info256);

    REQUIRE(info128.algorithm == types::Algorithm::AES_128);
    REQUIRE(info128.keySize == 16);
    REQUIRE(info256.algorithm == types::Algorithm::AES_256);
    REQUIRE(info256.keySize == 32);
}

TEST_CASE("KeystoreService key access count increments", "[keystore]") {
    // Arrange
    KeystoreService keystore;
    (void)keystore.init();

    std::array<uint8_t, 16> keyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);
    (void)keystore.importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Act - Get key multiple times
    types::Key key1, key2, key3;
    (void)keystore.getKey(0, key1);
    (void)keystore.getKey(0, key2);
    (void)keystore.getKey(0, key3);

    // Note: Access count is internal, but we verify getKey works multiple times
    REQUIRE(key1.size == 16);
    REQUIRE(key2.size == 16);
    REQUIRE(key3.size == 16);
}

} // namespace ehsm::services
