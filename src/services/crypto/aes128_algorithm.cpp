/**
 * @file aes128_algorithm.cpp
 * @brief AES-128 algorithm implementation.
 * 
 * NOTE: This is a MOCK implementation for demonstration only!
 * In production, use a real AES implementation (e.g., mbedTLS, hardware crypto).
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "ehsm/services/crypto/aes128_algorithm.hpp"
#include <cstring>

namespace ehsm::services::crypto {

// Mock encryption key XOR value
inline constexpr uint8_t kMockEncryptKey = 0x5AU;

types::Algorithm Aes128Algorithm::getAlgorithmType() const {
    return types::Algorithm::AES_128;
}

size_t Aes128Algorithm::getKeySize() const {
    return 16U;  // 128 bits = 16 bytes
}

size_t Aes128Algorithm::getBlockSize() const {
    return 16U;  // AES block size
}

Status Aes128Algorithm::encrypt(
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

Status Aes128Algorithm::decrypt(
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

void Aes128Algorithm::encryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* /*key*/) const
{
    // Mock block encryption (XOR)
    for (size_t i = 0U; i < 16U; ++i) {
        output[i] = input[i] ^ kMockEncryptKey;
    }
}

void Aes128Algorithm::decryptBlock(
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
