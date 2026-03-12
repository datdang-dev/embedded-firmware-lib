/**
 * @file aes256_algorithm.cpp
 * @brief AES-256 algorithm implementation.
 * 
 * NOTE: This is a MOCK implementation for demonstration only!
 * In production, use a real AES implementation (e.g., mbedTLS, hardware crypto).
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "aes256_algorithm.hpp"
#include <cstring>

namespace ehsm::services::crypto {

// Mock encryption key XOR value
inline constexpr uint8_t kMockEncryptKey = 0x5AU;

types::Algorithm Aes256Algorithm::getAlgorithmType() const {
    return types::Algorithm::AES_256;
}

size_t Aes256Algorithm::getKeySize() const {
    return 32U;  // 256 bits = 32 bytes
}

size_t Aes256Algorithm::getBlockSize() const {
    return 16U;  // AES block size
}

Status Aes256Algorithm::encrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    // Validate buffers
    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INSUFFICIENT_BUFFER);
    }

    // Mock encryption (XOR-based for demo only!)
    for (size_t i = 0U; i < input.size(); ++i) {
        output[i] = input[i] ^ kMockEncryptKey;
    }

    return Status(types::StatusCode::OK);
}

Status Aes256Algorithm::decrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    // Validate buffers
    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INSUFFICIENT_BUFFER);
    }

    // Mock decryption (XOR is symmetric)
    for (size_t i = 0U; i < input.size(); ++i) {
        output[i] = input[i] ^ kMockEncryptKey;
    }

    return Status(types::StatusCode::OK);
}

void Aes256Algorithm::encryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* /*key*/) const
{
    // Mock block encryption (XOR)
    for (size_t i = 0U; i < 16U; ++i) {
        output[i] = input[i] ^ kMockEncryptKey;
    }
}

void Aes256Algorithm::decryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* /*key*/) const
{
    // Mock block decryption (XOR is symmetric)
    for (size_t i = 0U; i < 16U; ++i) {
        output[i] = input[i] ^ kMockEncryptKey;
    }
}

} // namespace ehsm::services::crypto
