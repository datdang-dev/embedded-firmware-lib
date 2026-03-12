/**
 * @file test_integration.cpp
 * @brief Integration tests for Embedded HSM - Full stack testing.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <catch2/catch_all.hpp>
#include "hsm_api_impl.hpp"
#include "session_manager.hpp"
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes256_algorithm.hpp"
#include "aes128_algorithm.hpp"
#include <array>
#include <cstring>

namespace ehsm::integration {

using namespace ehsm::services;
using namespace ehsm::services::crypto;
using namespace ehsm::mw;
using namespace ehsm::api;

/**
 * IT_ENCRYPT_001 - Full Encryption Flow
 */
TEST_CASE("IT_ENCRYPT_001 - Full encryption flow", "[integration][encrypt]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    types::SessionId sessionId;
    REQUIRE(hsm->createSession(sessionId).isOk());

    std::array<uint8_t, 32> key = {0};
    for (size_t i = 0; i < 32; ++i) {
        key[i] = static_cast<uint8_t>(i);
    }
    REQUIRE(hsm->importKey(0, types::Algorithm::AES_256, key,
                           static_cast<uint8_t>(types::KeyPermission::ENCRYPT)).isOk());

    std::array<uint8_t, 16> plaintext = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x20, 0x57,
                                          0x6F, 0x72, 0x6C, 0x64, 0x21, 0x00, 0x00, 0x00};
    std::array<uint8_t, 16> ciphertext = {0};

    auto status = hsm->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext);

    REQUIRE(status.isOk());
    REQUIRE(ciphertext[0] != plaintext[0]);
}

/**
 * IT_DECRYPT_001 - Full Decryption Flow
 */
TEST_CASE("IT_DECRYPT_001 - Full decryption flow", "[integration][decrypt]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    types::SessionId sessionId;
    REQUIRE(hsm->createSession(sessionId).isOk());

    std::array<uint8_t, 32> key = {0};
    REQUIRE(hsm->importKey(0, types::Algorithm::AES_256, key,
                           static_cast<uint8_t>(types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT)).isOk());

    std::array<uint8_t, 16> plaintext = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                          0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> ciphertext = {0};
    
    REQUIRE(hsm->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext).isOk());

    std::array<uint8_t, 16> decrypted = {0};
    auto status = hsm->decrypt(sessionId, 0, types::Algorithm::AES_256, ciphertext, decrypted);

    REQUIRE(status.isOk());
    REQUIRE(plaintext == decrypted);
}

/**
 * IT_KEY_001 - Key Lifecycle
 */
TEST_CASE("IT_KEY_001 - Key lifecycle", "[integration][key]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    std::array<uint8_t, 32> keyData = {0};
    for (size_t i = 0; i < 32; ++i) {
        keyData[i] = static_cast<uint8_t>(i + 1);
    }
    
    REQUIRE(hsm->importKey(0, types::Algorithm::AES_256, keyData,
                           static_cast<uint8_t>(types::KeyPermission::ALL)).isOk());

    types::KeySlotInfo info;
    auto infoStatus = hsm->getKeySlotInfo(0, info);
    REQUIRE(infoStatus.isOk());
    REQUIRE(info.isOccupied);
    REQUIRE(info.algorithm == types::Algorithm::AES_256);
    REQUIRE(info.keySize == 32);

    REQUIRE(hsm->deleteKey(0).isOk());

    types::KeySlotInfo infoAfter;
    (void)hsm->getKeySlotInfo(0, infoAfter);
    REQUIRE_FALSE(infoAfter.isOccupied);

    REQUIRE_FALSE(hsm->deleteKey(0).isOk());
}

/**
 * IT_SESSION_001 - Session Lifecycle
 */
TEST_CASE("IT_SESSION_001 - Session lifecycle", "[integration][session]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    types::SessionId sessionId;
    auto createStatus = hsm->createSession(sessionId);
    REQUIRE(createStatus.isOk());
    REQUIRE(sessionId != types::kInvalidSessionId);
    REQUIRE(hsm->isSessionValid(sessionId));

    std::array<uint8_t, 32> key = {0};
    (void)hsm->importKey(0, types::Algorithm::AES_256, key,
                         static_cast<uint8_t>(types::KeyPermission::ENCRYPT));
    
    std::array<uint8_t, 16> plaintext = {0};
    std::array<uint8_t, 16> ciphertext = {0};
    REQUIRE(hsm->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext).isOk());

    REQUIRE(hsm->closeSession(sessionId).isOk());
    REQUIRE_FALSE(hsm->isSessionValid(sessionId));

    REQUIRE_FALSE(hsm->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext).isOk());
}

/**
 * IT_MULTI_001 - Multiple Sessions
 */
TEST_CASE("IT_MULTI_001 - Multiple sessions", "[integration][multi]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    std::array<uint8_t, 32> key = {0};
    REQUIRE(hsm->importKey(0, types::Algorithm::AES_256, key,
                           static_cast<uint8_t>(types::KeyPermission::ENCRYPT)).isOk());
    REQUIRE(hsm->importKey(1, types::Algorithm::AES_256, key,
                           static_cast<uint8_t>(types::KeyPermission::ENCRYPT)).isOk());
    REQUIRE(hsm->importKey(2, types::Algorithm::AES_256, key,
                           static_cast<uint8_t>(types::KeyPermission::ENCRYPT)).isOk());

    types::SessionId sessionId1, sessionId2, sessionId3;
    REQUIRE(hsm->createSession(sessionId1).isOk());
    REQUIRE(hsm->createSession(sessionId2).isOk());
    REQUIRE(hsm->createSession(sessionId3).isOk());

    REQUIRE(hsm->isSessionValid(sessionId1));
    REQUIRE(hsm->isSessionValid(sessionId2));
    REQUIRE(hsm->isSessionValid(sessionId3));

    std::array<uint8_t, 16> plaintext1 = {0x01};
    std::array<uint8_t, 16> plaintext2 = {0x02};
    std::array<uint8_t, 16> plaintext3 = {0x03};
    std::array<uint8_t, 16> ciphertext1 = {0};
    std::array<uint8_t, 16> ciphertext2 = {0};
    std::array<uint8_t, 16> ciphertext3 = {0};

    REQUIRE(hsm->encrypt(sessionId1, 0, types::Algorithm::AES_256, plaintext1, ciphertext1).isOk());
    REQUIRE(hsm->encrypt(sessionId2, 1, types::Algorithm::AES_256, plaintext2, ciphertext2).isOk());
    REQUIRE(hsm->encrypt(sessionId3, 2, types::Algorithm::AES_256, plaintext3, ciphertext3).isOk());

    REQUIRE(ciphertext1[0] != ciphertext2[0]);
    REQUIRE(ciphertext2[0] != ciphertext3[0]);
    REQUIRE(ciphertext1[0] != ciphertext3[0]);

    REQUIRE(hsm->closeSession(sessionId1).isOk());
    REQUIRE(hsm->closeSession(sessionId2).isOk());
    REQUIRE(hsm->closeSession(sessionId3).isOk());

    REQUIRE_FALSE(hsm->isSessionValid(sessionId1));
    REQUIRE_FALSE(hsm->isSessionValid(sessionId2));
    REQUIRE_FALSE(hsm->isSessionValid(sessionId3));
}

/**
 * IT_ERROR_001 - Error Propagation
 */
TEST_CASE("IT_ERROR_001 - Error propagation", "[integration][error]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    REQUIRE(hsm->init().isOk());

    types::SessionId sessionId;
    REQUIRE(hsm->createSession(sessionId).isOk());

    // Do NOT import key - slot 0 is empty
    // Attempt to encrypt with empty slot - will fail at permission check
    // since empty slot has no ENCRYPT permission
    std::array<uint8_t, 16> plaintext = {0};
    std::array<uint8_t, 16> ciphertext = {0};
    auto status = hsm->encrypt(sessionId, 0, types::Algorithm::AES_256, plaintext, ciphertext);

    // Error propagates from keystore (slot empty = no permission = ERR_AUTH_FAILED)
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_AUTH_FAILED);
}

/**
 * IT_INIT_001 - Full Initialization Sequence
 */
TEST_CASE("IT_INIT_001 - Full initialization sequence", "[integration][init]") {
    auto keystore = std::make_shared<KeystoreService>();
    auto crypto = std::make_unique<CryptoService>(
        keystore,
        std::make_unique<Aes256Algorithm>()
    );
    auto sessionMgr = std::make_unique<SessionManager>();
    auto hsm = std::make_unique<HsmApiImpl>(
        std::move(sessionMgr),
        std::move(crypto),
        keystore
    );

    auto initStatus = hsm->init();
    REQUIRE(initStatus.isOk());
    REQUIRE(hsm->isReady());

    const char* version = hsm->getVersion();
    REQUIRE(version != nullptr);
    REQUIRE(std::string(version) == "1.0.0");

    auto secondInit = hsm->init();
    REQUIRE(secondInit.isOk());
}

} // namespace ehsm::integration
