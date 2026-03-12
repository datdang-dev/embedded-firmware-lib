/**
 * @file icrypto_algorithm.hpp
 * @brief Crypto algorithm strategy interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "common.hpp"

namespace ehsm::services {

/**
 * @brief Interface for cryptographic algorithms (Strategy pattern).
 * 
 * Each algorithm (AES-128, AES-256, etc.) implements this interface.
 * The CryptoService uses this interface to perform encryption/decryption
 * without knowing the specific algorithm.
 */
class ICryptoAlgorithm {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ICryptoAlgorithm() = default;

    /**
     * @brief Get the algorithm type.
     * @return Algorithm type identifier.
     */
    [[nodiscard]] virtual types::Algorithm getAlgorithmType() const = 0;

    /**
     * @brief Get the key size for this algorithm.
     * @return Key size in bytes.
     */
    [[nodiscard]] virtual size_t getKeySize() const = 0;

    /**
     * @brief Get the block size for this algorithm.
     * @return Block size in bytes (0 for stream ciphers).
     */
    [[nodiscard]] virtual size_t getBlockSize() const = 0;

    /**
     * @brief Encrypt data.
     * @param[in] input Plaintext input.
     * @param[out] output Ciphertext output.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output) = 0;

    /**
     * @brief Decrypt data.
     * @param[in] input Ciphertext input.
     * @param[out] output Plaintext output.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output) = 0;
};

} // namespace ehsm::services
