/**
 * @file test_crypto_service.cpp
 * @brief Unit tests for CryptoService using Catch2 + Trompeloeil.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <catch2/catch_all.hpp>
#include <trompeloeil.hpp>
#include "crypto_service.hpp"
#include "mock_services.hpp"
#include "aes256_algorithm.hpp"
#include "aes128_algorithm.hpp"
#include <array>
#include <vector>

namespace ehsm::services {

using namespace ehsm::mocks;

// TC_CRYPTO_001 - Encrypt with valid input
TEST_CASE("TC_CRYPTO_001 - Encrypt with valid input", "[crypto][encrypt]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgo = std::make_unique<MockCryptoAlgorithm>();
    auto* mockAlgoPtr = mockAlgo.get();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgo));

    REQUIRE_CALL(*mockKeystore, getKey(0, trompeloeil::_))
        .LR_RETURN(types::Status(types::StatusCode::OK));

    REQUIRE_CALL(*mockAlgoPtr, encrypt(trompeloeil::_, trompeloeil::_))
        .RETURN(types::Status(types::StatusCode::OK));

    std::array<uint8_t, 16> plaintext = {0x41, 0x42, 0x43, 0x44};
    std::array<uint8_t, 16> ciphertext = {0};

    auto status = cryptoService->encrypt(plaintext, ciphertext, 0);

    REQUIRE(status.isOk());
}

// TC_CRYPTO_002 - Encrypt with empty input
TEST_CASE("TC_CRYPTO_002 - Encrypt with empty input", "[crypto][encrypt]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgorithm = std::make_unique<MockCryptoAlgorithm>();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgorithm));

    std::array<uint8_t, 16> output{0};
    std::span<const uint8_t> emptyInput;

    auto status = cryptoService->encrypt(emptyInput, output, 0);

    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

// TC_CRYPTO_003 - Encrypt with output buffer too small
TEST_CASE("TC_CRYPTO_003 - Encrypt with output buffer too small", "[crypto][encrypt]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgorithm = std::make_unique<MockCryptoAlgorithm>();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgorithm));

    std::array<uint8_t, 32> input{0};
    std::array<uint8_t, 16> smallOutput{0};

    auto status = cryptoService->encrypt(input, smallOutput, 0);

    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

// TC_CRYPTO_004 - Encrypt with invalid key slot
TEST_CASE("TC_CRYPTO_004 - Encrypt with invalid key slot", "[crypto][encrypt]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgorithm = std::make_unique<MockCryptoAlgorithm>();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgorithm));

    std::array<uint8_t, 16> input{0};
    std::array<uint8_t, 16> output{0};
    uint8_t invalidSlotId = 255;

    REQUIRE_CALL(*mockKeystore, getKey(invalidSlotId, trompeloeil::_))
        .RETURN(types::Status(types::StatusCode::ERR_INVALID_KEY_ID));

    auto status = cryptoService->encrypt(input, output, invalidSlotId);

    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INVALID_KEY_ID);
}

// TC_CRYPTO_005 - Decrypt with valid input
TEST_CASE("TC_CRYPTO_005 - Decrypt with valid input", "[crypto][decrypt]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgo = std::make_unique<MockCryptoAlgorithm>();
    auto* mockAlgoPtr = mockAlgo.get();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgo));

    REQUIRE_CALL(*mockKeystore, getKey(0, trompeloeil::_))
        .LR_RETURN(types::Status(types::StatusCode::OK));

    REQUIRE_CALL(*mockAlgoPtr, decrypt(trompeloeil::_, trompeloeil::_))
        .RETURN(types::Status(types::StatusCode::OK));

    std::array<uint8_t, 16> ciphertext = {0x41, 0x42, 0x43, 0x44};
    std::array<uint8_t, 16> plaintext = {0};

    auto status = cryptoService->decrypt(ciphertext, plaintext, 0);

    REQUIRE(status.isOk());
}

// TC_CRYPTO_006 - Encrypt/Decrypt round-trip
TEST_CASE("TC_CRYPTO_006 - Encrypt/Decrypt round-trip", "[crypto][roundtrip]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto algorithm = std::make_unique<crypto::Aes256Algorithm>();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(algorithm));

    std::array<uint8_t, 32> plaintext = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    std::array<uint8_t, 32> ciphertext{0};
    std::array<uint8_t, 32> decrypted{0};

    ALLOW_CALL(*mockKeystore, getKey(0, trompeloeil::_))
        .LR_RETURN(types::Status(types::StatusCode::OK));

    auto encryptStatus = cryptoService->encrypt(plaintext, ciphertext, 0);
    REQUIRE(encryptStatus.isOk());

    auto decryptStatus = cryptoService->decrypt(ciphertext, decrypted, 0);
    REQUIRE(decryptStatus.isOk());

    REQUIRE(plaintext == decrypted);
}

// TC_CRYPTO_007 - Get algorithm type
TEST_CASE("TC_CRYPTO_007 - Get algorithm type", "[crypto][getter]") {
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto mockAlgo = std::make_unique<MockCryptoAlgorithm>();
    auto* mockAlgoPtr = mockAlgo.get();
    auto cryptoService = std::make_unique<CryptoService>(
        std::static_pointer_cast<IKeystoreService>(mockKeystore), 
        std::move(mockAlgo));

    ALLOW_CALL(*mockAlgoPtr, getAlgorithmType())
        .RETURN(types::Algorithm::AES_256);

    auto algoType = cryptoService->getAlgorithmType();

    REQUIRE(algoType == types::Algorithm::AES_256);
}

} // namespace ehsm::services
