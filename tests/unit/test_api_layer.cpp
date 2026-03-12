/**
 * @file test_api_layer.cpp
 * @brief Unit tests for HSM API layer using Catch2 + Trompeloeil.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <catch2/catch_all.hpp>
#include <trompeloeil.hpp>
#include "hsm_api_impl.hpp"
#include "mock_services.hpp"
#include <array>
#include <vector>

namespace ehsm::api {

using namespace mocks;

TEST_CASE("HsmApiImpl initializes successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Setup mocks
    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    // Act
    auto status = api->init();

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl double initialization succeeds", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    // Act
    api->init();
    auto secondInit = api->init();

    // Assert
    REQUIRE(secondInit.isOk());
}

TEST_CASE("HsmApiImpl deinitialize successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, deinit())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, deinit())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, deinit())
        .RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->deinit();

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl deinitialize without init fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    auto status = api->deinit();

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl isReady returns correct state", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Assert - Before init
    REQUIRE_FALSE(api->isReady());

    // Setup mocks for init
    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    // Act - Init
    api->init();

    // Assert - After init
    REQUIRE(api->isReady());
}

TEST_CASE("HsmApiImpl encrypt without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    // Act
    auto status = api->encrypt(1, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl decrypt without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    // Act
    auto status = api->decrypt(1, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl full encrypt flow with mocked services", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::SessionId sessionId = 1;
    std::array<uint8_t, 16> plaintext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> ciphertext{0};

    // Setup mocks
    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(sessionId))
        .RETURN(true);

    types::KeySlotInfo info;
    info.permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);
    ALLOW_CALL(*mockKeystoreService, getSlotInfo(0, _))
        .LR_RETURN(Status(types::StatusCode::OK));

    ALLOW_CALL(*mockCryptoService, encrypt(_, _, 0))
        .RETURN(Status(types::StatusCode::OK));

    // Init
    api->init();

    // Act
    auto status = api->encrypt(sessionId, 0, types::Algorithm::AES_128, plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl full decrypt flow with mocked services", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::SessionId sessionId = 1;
    std::array<uint8_t, 16> ciphertext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> plaintext{0};

    // Setup mocks
    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(sessionId))
        .RETURN(true);

    types::KeySlotInfo info;
    info.permissions = static_cast<uint8_t>(types::KeyPermission::DECRYPT);
    ALLOW_CALL(*mockKeystoreService, getSlotInfo(0, _))
        .LR_RETURN(Status(types::StatusCode::OK));

    ALLOW_CALL(*mockCryptoService, decrypt(_, _, 0))
        .RETURN(Status(types::StatusCode::OK));

    // Init
    api->init();

    // Act
    auto status = api->decrypt(sessionId, 0, types::Algorithm::AES_128, ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl encrypt with invalid session fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(_))
        .RETURN(false);

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    api->init();

    // Act
    auto status = api->encrypt(999, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_SESSION_INVALID);
}

TEST_CASE("HsmApiImpl decrypt with invalid session fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(_))
        .RETURN(false);

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    api->init();

    // Act
    auto status = api->decrypt(999, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_SESSION_INVALID);
}

TEST_CASE("HsmApiImpl encrypt permission check fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::SessionId sessionId = 1;
    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(sessionId))
        .RETURN(true);

    // Key has DECRYPT permission only, not ENCRYPT
    types::KeySlotInfo info;
    info.permissions = static_cast<uint8_t>(types::KeyPermission::DECRYPT);
    ALLOW_CALL(*mockKeystoreService, getSlotInfo(0, _))
        .LR_RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->encrypt(sessionId, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_AUTH_FAILED);
}

TEST_CASE("HsmApiImpl decrypt permission check fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::SessionId sessionId = 1;
    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(sessionId))
        .RETURN(true);

    // Key has ENCRYPT permission only, not DECRYPT
    types::KeySlotInfo info;
    info.permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);
    ALLOW_CALL(*mockKeystoreService, getSlotInfo(0, _))
        .LR_RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->decrypt(sessionId, 0, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_AUTH_FAILED);
}

TEST_CASE("HsmApiImpl encrypt with empty input fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    std::span<const uint8_t> emptyInput;
    std::array<uint8_t, 16> output{0};

    api->init();

    // Act
    auto status = api->encrypt(1, 0, types::Algorithm::AES_128, emptyInput, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_PARAM);
}

TEST_CASE("HsmApiImpl encrypt with invalid key slot ID", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(_))
        .RETURN(true);

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};

    api->init();

    // Act
    auto status = api->encrypt(1, 0xFF, types::Algorithm::AES_128, input, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

TEST_CASE("HsmApiImpl import key successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    std::array<uint8_t, 16> keyData{0};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, importKey(0, types::Algorithm::AES_128, _, permissions))
        .RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->importKey(0, types::Algorithm::AES_128, keyData, permissions);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl import key without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    std::array<uint8_t, 16> keyData{0};

    // Act
    auto status = api->importKey(0, types::Algorithm::AES_128, keyData, 0);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl delete key successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, deleteKey(0))
        .RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->deleteKey(0);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl delete key without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    auto status = api->deleteKey(0);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl get key slot info successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    types::KeySlotInfo info;
    info.id = 0;
    info.isOccupied = true;
    info.algorithm = types::Algorithm::AES_128;
    info.permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT);
    info.keySize = 16;

    ALLOW_CALL(*mockKeystoreService, getSlotInfo(0, _))
        .LR_RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    types::KeySlotInfo result;
    auto status = api->getKeySlotInfo(0, result);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl get key slot info without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::KeySlotInfo info;

    // Act
    auto status = api->getKeySlotInfo(0, info);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl clear all keys successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, clearAll())
        .RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->clearAllKeys();

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl clear all keys without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    auto status = api->clearAllKeys();

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl get version string", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    const char* version = api->getVersion();

    // Assert
    REQUIRE(version != nullptr);
    REQUIRE(std::string(version) == "1.0.0");
}

TEST_CASE("HsmApiImpl create session successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));

    types::SessionId sessionId = 1;
    ALLOW_CALL(*mockSessionManager, createSession(_))
        .LR_RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    types::SessionId result;
    auto status = api->createSession(result);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl create session without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    types::SessionId sessionId;

    // Act
    auto status = api->createSession(sessionId);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl close session successfully", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, closeSession(1))
        .RETURN(Status(types::StatusCode::OK));

    api->init();

    // Act
    auto status = api->closeSession(1);

    // Assert
    REQUIRE(status.isOk());
}

TEST_CASE("HsmApiImpl close session without initialization fails", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    auto status = api->closeSession(1);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl is session valid", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    ALLOW_CALL(*mockSessionManager, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockCryptoService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockKeystoreService, init())
        .RETURN(Status(types::StatusCode::OK));
    ALLOW_CALL(*mockSessionManager, isSessionValid(1))
        .RETURN(true);
    ALLOW_CALL(*mockSessionManager, isSessionValid(999))
        .RETURN(false);

    api->init();

    // Act & Assert
    REQUIRE(api->isSessionValid(1));
    REQUIRE_FALSE(api->isSessionValid(999));
}

TEST_CASE("HsmApiImpl is session valid without initialization", "[api]") {
    // Arrange
    auto mockSessionManager = std::make_shared<MockSessionManager>();
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        mockSessionManager, mockCryptoService, mockKeystoreService);

    // Act
    auto isValid = api->isSessionValid(1);

    // Assert
    REQUIRE_FALSE(isValid);
}

TEST_CASE("HsmApiImpl init with null dependencies fails", "[api]") {
    // Arrange
    std::shared_ptr<mw::ISessionManager> nullSession;
    auto mockCryptoService = std::make_shared<MockCryptoService>();
    auto mockKeystoreService = std::make_shared<MockKeystoreService>();
    auto api = std::make_unique<HsmApiImpl>(
        nullSession, mockCryptoService, mockKeystoreService);

    // Act
    auto status = api->init();

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_NOT_INITIALIZED);
}

TEST_CASE("HsmApiImpl encrypt/decrypt round-trip integration", "[api][integration]") {
    // Arrange - Use real services for integration test
    auto sessionManager = std::make_shared<mw::SessionManager>();
    auto keystoreService = std::make_shared<services::KeystoreService>();
    auto algorithm = std::make_unique<services::crypto::Aes256Algorithm>();
    auto cryptoService = std::make_shared<services::CryptoService>(
        keystoreService, std::move(algorithm));
    auto api = std::make_unique<HsmApiImpl>(
        sessionManager, cryptoService, keystoreService);

    // Init
    api->init();

    // Create session
    types::SessionId sessionId;
    api->createSession(sessionId);

    // Import key with ENCRYPT and DECRYPT permissions
    std::array<uint8_t, 32> keyData{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    uint8_t permissions = static_cast<uint8_t>(types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
    api->importKey(0, types::Algorithm::AES_256, keyData, permissions);

    std::array<uint8_t, 32> plaintext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    std::array<uint8_t, 32> ciphertext{0};
    std::array<uint8_t, 32> decrypted{0};

    // Act - Encrypt
    auto encryptStatus = api->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext);
    REQUIRE(encryptStatus.isOk());

    // Act - Decrypt
    auto decryptStatus = api->decrypt(sessionId, 0, types::Algorithm::AES_256, ciphertext, decrypted);
    REQUIRE(decryptStatus.isOk());

    // Assert - Verify round-trip
    REQUIRE(plaintext == decrypted);
}

} // namespace ehsm::api
