/**
 * @file crypto_service.hpp
 * @brief CryptoService implementation header.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "ehsm/services/icrypto_service.hpp"
#include "ehsm/services/crypto/icrypto_algorithm.hpp"
#include <memory>
#include <span>

namespace ehsm::services {

/**
 * @brief CryptoService implements symmetric encryption operations.
 * 
 * This class uses the Strategy pattern to support multiple algorithms.
 * Dependencies are injected via constructor.
 */
class CryptoService final : public ICryptoService {
public:
    /**
     * @brief Construct CryptoService with dependencies.
     * @param keystore Keystore service for key retrieval.
     * @param algorithm Crypto algorithm strategy.
     */
    explicit CryptoService(
        std::shared_ptr<IKeystoreService> keystore,
        std::unique_ptr<ICryptoAlgorithm> algorithm);

    /**
     * @brief Default destructor.
     */
    ~CryptoService() override = default;

    /**
     * @brief Delete copy constructor.
     */
    CryptoService(const CryptoService&) = delete;

    /**
     * @brief Delete copy assignment.
     */
    CryptoService& operator=(const CryptoService&) = delete;

    /**
     * @brief Default move constructor.
     */
    CryptoService(CryptoService&&) noexcept = default;

    /**
     * @brief Default move assignment.
     */
    CryptoService& operator=(CryptoService&&) noexcept = default;

    // ICryptoService implementation
    [[nodiscard]] Status init() override;
    [[nodiscard]] Status deinit() override;
    [[nodiscard]] Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) override;
    [[nodiscard]] Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) override;
    [[nodiscard]] Status hash(
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) override;

    /**
     * @brief Get the algorithm type.
     * @return Current algorithm type.
     */
    [[nodiscard]] types::Algorithm getAlgorithmType() const;

private:
    std::unique_ptr<ICryptoAlgorithm> algorithm_;   ///< Algorithm strategy
    std::shared_ptr<IKeystoreService> keystore_;    ///< Keystore dependency
};

} // namespace ehsm::services
