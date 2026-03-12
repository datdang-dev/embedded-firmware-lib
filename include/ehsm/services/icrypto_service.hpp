/**
 * @file icrypto_service.hpp
 * @brief Crypto service interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "ehsm/types/status.hpp"
#include "ehsm/types/algorithm.hpp"
#include <span>

namespace ehsm::services {

/**
 * @brief Interface for cryptographic operations.
 * 
 * All crypto service implementations must inherit from this interface.
 * This enables dependency injection and testing with mocks.
 */
class ICryptoService {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ICryptoService() = default;

    /**
     * @brief Initialize the crypto service.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status init() = 0;

    /**
     * @brief Deinitialize the crypto service.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status deinit() = 0;

    /**
     * @brief Encrypt data using the configured algorithm.
     * @param[in] input Plaintext input data.
     * @param[out] output Ciphertext output buffer.
     * @param[in] keySlotId Key slot containing encryption key.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) = 0;

    /**
     * @brief Decrypt data using the configured algorithm.
     * @param[in] input Ciphertext input data.
     * @param[out] output Plaintext output buffer.
     * @param[in] keySlotId Key slot containing decryption key.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) = 0;

    /**
     * @brief Compute hash of input data.
     * @param[in] algorithm Hash algorithm to use.
     * @param[in] input Input data.
     * @param[out] output Hash digest output.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status hash(
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) = 0;
};

} // namespace ehsm::services
