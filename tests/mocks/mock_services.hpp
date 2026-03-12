/**
 * @file mock_services.hpp
 * @brief Mock services using Trompeloeil for unit testing.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "common.hpp"
#include "icrypto_service.hpp"
#include "ikeystore_service.hpp"
#include "isession_manager.hpp"
#include "icrypto_algorithm.hpp"
#include <trompeloeil.hpp>
#include <memory>

namespace ehsm::mocks {

/**
 * @brief Mock ICryptoService implementation.
 */
class MockCryptoService : public services::ICryptoService {
public:
    MAKE_MOCK0(init, types::Status(), override);
    MAKE_MOCK0(deinit, types::Status(), override);
    MAKE_MOCK3(encrypt, types::Status(std::span<const uint8_t>, std::span<uint8_t>, uint8_t), override);
    MAKE_MOCK3(decrypt, types::Status(std::span<const uint8_t>, std::span<uint8_t>, uint8_t), override);
    MAKE_MOCK3(hash, types::Status(types::Algorithm, std::span<const uint8_t>, std::span<uint8_t>), override);
};

/**
 * @brief Mock IKeystoreService implementation.
 */
class MockKeystoreService : public services::IKeystoreService {
public:
    MAKE_MOCK0(init, types::Status(), override);
    MAKE_MOCK0(deinit, types::Status(), override);
    MAKE_MOCK4(importKey, types::Status(uint8_t, types::Algorithm, std::span<const uint8_t>, uint8_t), override);
    MAKE_MOCK1(deleteKey, types::Status(uint8_t), override);
    MAKE_MOCK2(getKey, types::Status(uint8_t, types::Key&), override);
    MAKE_MOCK2(getSlotInfo, types::Status(uint8_t, types::KeySlotInfo&), override);
    MAKE_MOCK0(clearAll, types::Status(), override);
    MAKE_MOCK1(isSlotOccupied, bool(uint8_t), const, override);
};

/**
 * @brief Mock ISessionManager implementation.
 */
class MockSessionManager : public mw::ISessionManager {
public:
    MAKE_MOCK0(init, types::Status(), override);
    MAKE_MOCK0(deinit, types::Status(), override);
    MAKE_MOCK1(createSession, types::Status(types::SessionId&), override);
    MAKE_MOCK1(closeSession, types::Status(types::SessionId), override);
    MAKE_MOCK1(isSessionValid, bool(types::SessionId), const, override);
};

/**
 * @brief Mock ICryptoAlgorithm implementation.
 */
class MockCryptoAlgorithm : public services::ICryptoAlgorithm {
public:
    MAKE_MOCK0(getAlgorithmType, types::Algorithm(), const, override);
    MAKE_MOCK0(getKeySize, size_t(), const, override);
    MAKE_MOCK0(getBlockSize, size_t(), const, override);
    MAKE_MOCK2(encrypt, types::Status(std::span<const uint8_t>, std::span<uint8_t>), override);
    MAKE_MOCK2(decrypt, types::Status(std::span<const uint8_t>, std::span<uint8_t>), override);
};

} // namespace ehsm::mocks
