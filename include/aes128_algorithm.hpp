/**
 * @file aes128_algorithm.hpp
 * @brief AES-128 algorithm implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "common.hpp"
#include "icrypto_algorithm.hpp"

namespace ehsm::services::crypto {

/**
 * @brief AES-128 encryption algorithm implementation.
 * 
 * This is a mock implementation for demonstration.
 * In production, replace with actual AES-128 implementation.
 */
class Aes128Algorithm final : public ICryptoAlgorithm {
public:
    /**
     * @brief Get algorithm type.
     * @return Algorithm::AES_128
     */
    [[nodiscard]] types::Algorithm getAlgorithmType() const override;

    /**
     * @brief Get key size.
     * @return 16 bytes (128 bits)
     */
    [[nodiscard]] size_t getKeySize() const override;

    /**
     * @brief Get block size.
     * @return 16 bytes (AES block size)
     */
    [[nodiscard]] size_t getBlockSize() const override;

    /**
     * @brief Encrypt data using AES-128.
     * @param[in] input Plaintext input.
     * @param[out] output Ciphertext output.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output) override;

    /**
     * @brief Decrypt data using AES-128.
     * @param[in] input Ciphertext input.
     * @param[out] output Plaintext output.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output) override;

private:
    /**
     * @brief Encrypt a single AES block.
     * @param[in] input 16-byte plaintext block.
     * @param[out] output 16-byte ciphertext block.
     * @param[in] key 16-byte AES key.
     */
    void encryptBlock(
        const uint8_t* input,
        uint8_t* output,
        const uint8_t* key) const;

    /**
     * @brief Decrypt a single AES block.
     * @param[in] input 16-byte ciphertext block.
     * @param[out] output 16-byte plaintext block.
     * @param[in] key 16-byte AES key.
     */
    void decryptBlock(
        const uint8_t* input,
        uint8_t* output,
        const uint8_t* key) const;
};

} // namespace ehsm::services::crypto
