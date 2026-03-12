/**
 * @file test_aes_algorithms.cpp
 * @brief Unit tests for AES128 and AES256 algorithms using Catch2.
 *
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include <catch2/catch_all.hpp>
#include "aes128_algorithm.hpp"
#include "aes256_algorithm.hpp"
#include <array>
#include <vector>
#include <cstring>

namespace ehsm::services::crypto {

// =============================================================================
// AES-128 Algorithm Tests
// =============================================================================

TEST_CASE("AES128Algorithm getAlgorithmType returns AES_128", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;

    // Act
    auto algoType = algorithm.getAlgorithmType();

    // Assert
    REQUIRE(algoType == types::Algorithm::AES_128);
}

TEST_CASE("AES128Algorithm getKeySize returns 16 bytes", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;

    // Act
    auto keySize = algorithm.getKeySize();

    // Assert
    REQUIRE(keySize == 16U);
}

TEST_CASE("AES128Algorithm getBlockSize returns 16 bytes", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;

    // Act
    auto blockSize = algorithm.getBlockSize();

    // Assert
    REQUIRE(blockSize == 16U);
}

TEST_CASE("AES128Algorithm encrypt single block", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> plaintext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> ciphertext{0};

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Verify encryption happened (XOR with 0x5A in mock)
    REQUIRE(ciphertext[0] == (plaintext[0] ^ 0x5A));
    REQUIRE(ciphertext[15] == (plaintext[15] ^ 0x5A));
}

TEST_CASE("AES128Algorithm decrypt single block", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> ciphertext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> plaintext{0};

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    // Verify decryption happened (XOR with 0x5A in mock)
    REQUIRE(plaintext[0] == (ciphertext[0] ^ 0x5A));
    REQUIRE(plaintext[15] == (ciphertext[15] ^ 0x5A));
}

TEST_CASE("AES128Algorithm encrypt/decrypt round-trip", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> original{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> encrypted{0};
    std::array<uint8_t, 16> decrypted{0};

    // Act - Encrypt
    auto encryptStatus = algorithm.encrypt(original, encrypted);
    REQUIRE(encryptStatus.isOk());

    // Act - Decrypt
    auto decryptStatus = algorithm.decrypt(encrypted, decrypted);
    REQUIRE(decryptStatus.isOk());

    // Assert - Verify round-trip
    REQUIRE(original == decrypted);
}

TEST_CASE("AES128Algorithm encrypt with empty input", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> output{0};
    std::span<const uint8_t> emptyInput;

    // Act
    auto status = algorithm.encrypt(emptyInput, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES128Algorithm decrypt with empty input", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> output{0};
    std::span<const uint8_t> emptyInput;

    // Act
    auto status = algorithm.decrypt(emptyInput, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES128Algorithm encrypt with output buffer too small", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 32> input{0xAB};
    std::array<uint8_t, 16> smallOutput{0};

    // Act
    auto status = algorithm.encrypt(input, smallOutput);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES128Algorithm decrypt with output buffer too small", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 32> input{0xAB};
    std::array<uint8_t, 16> smallOutput{0};

    // Act
    auto status = algorithm.decrypt(input, smallOutput);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES128Algorithm encrypt multiple blocks", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::vector<uint8_t> plaintext(64, 0x42);  // 4 blocks
    std::vector<uint8_t> ciphertext(64, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Verify all bytes were encrypted
    for (size_t i = 0; i < 64; ++i) {
        REQUIRE(ciphertext[i] == (plaintext[i] ^ 0x5A));
    }
}

TEST_CASE("AES128Algorithm decrypt multiple blocks", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::vector<uint8_t> ciphertext(64, 0x42);  // 4 blocks
    std::vector<uint8_t> plaintext(64, 0);

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    // Verify all bytes were decrypted
    for (size_t i = 0; i < 64; ++i) {
        REQUIRE(plaintext[i] == (ciphertext[i] ^ 0x5A));
    }
}

TEST_CASE("AES128Algorithm encrypt single byte", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 1> plaintext{0x42};
    std::array<uint8_t, 1> ciphertext{0};

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(ciphertext[0] == (0x42 ^ 0x5A));
}

TEST_CASE("AES128Algorithm decrypt single byte", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 1> ciphertext{0x42};
    std::array<uint8_t, 1> plaintext{0};

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(plaintext[0] == (0x42 ^ 0x5A));
}

TEST_CASE("AES128Algorithm encrypt input not multiple of block size", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::vector<uint8_t> plaintext(20, 0x42);  // Not multiple of 16
    std::vector<uint8_t> ciphertext(20, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Mock implementation handles any size
    REQUIRE(ciphertext[0] == (plaintext[0] ^ 0x5A));
}

TEST_CASE("AES128Algorithm key size verification", "[aes][aes128]") {
    // Arrange
    Aes128Algorithm algorithm;

    // Act & Assert
    REQUIRE(algorithm.getKeySize() == 16U);
    REQUIRE(algorithm.getKeySize() == types::getKeySizeForAlgorithm(types::Algorithm::AES_128));
}

// =============================================================================
// AES-256 Algorithm Tests
// =============================================================================

TEST_CASE("AES256Algorithm getAlgorithmType returns AES_256", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;

    // Act
    auto algoType = algorithm.getAlgorithmType();

    // Assert
    REQUIRE(algoType == types::Algorithm::AES_256);
}

TEST_CASE("AES256Algorithm getKeySize returns 32 bytes", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;

    // Act
    auto keySize = algorithm.getKeySize();

    // Assert
    REQUIRE(keySize == 32U);
}

TEST_CASE("AES256Algorithm getBlockSize returns 16 bytes", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;

    // Act
    auto blockSize = algorithm.getBlockSize();

    // Assert
    REQUIRE(blockSize == 16U);
}

TEST_CASE("AES256Algorithm encrypt single block", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 16> plaintext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> ciphertext{0};

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Verify encryption happened (XOR with 0x5A in mock)
    REQUIRE(ciphertext[0] == (plaintext[0] ^ 0x5A));
    REQUIRE(ciphertext[15] == (plaintext[15] ^ 0x5A));
}

TEST_CASE("AES256Algorithm decrypt single block", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 16> ciphertext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> plaintext{0};

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    // Verify decryption happened (XOR with 0x5A in mock)
    REQUIRE(plaintext[0] == (ciphertext[0] ^ 0x5A));
    REQUIRE(plaintext[15] == (ciphertext[15] ^ 0x5A));
}

TEST_CASE("AES256Algorithm encrypt/decrypt round-trip", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 32> original{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    std::array<uint8_t, 32> encrypted{0};
    std::array<uint8_t, 32> decrypted{0};

    // Act - Encrypt
    auto encryptStatus = algorithm.encrypt(original, encrypted);
    REQUIRE(encryptStatus.isOk());

    // Act - Decrypt
    auto decryptStatus = algorithm.decrypt(encrypted, decrypted);
    REQUIRE(decryptStatus.isOk());

    // Assert - Verify round-trip
    REQUIRE(original == decrypted);
}

TEST_CASE("AES256Algorithm encrypt with empty input", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 16> output{0};
    std::span<const uint8_t> emptyInput;

    // Act
    auto status = algorithm.encrypt(emptyInput, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES256Algorithm decrypt with empty input", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 16> output{0};
    std::span<const uint8_t> emptyInput;

    // Act
    auto status = algorithm.decrypt(emptyInput, output);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES256Algorithm encrypt with output buffer too small", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 64> input{0xAB};
    std::array<uint8_t, 32> smallOutput{0};

    // Act
    auto status = algorithm.encrypt(input, smallOutput);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES256Algorithm decrypt with output buffer too small", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 64> input{0xAB};
    std::array<uint8_t, 32> smallOutput{0};

    // Act
    auto status = algorithm.decrypt(input, smallOutput);

    // Assert
    REQUIRE_FALSE(status.isOk());
    REQUIRE(status.code() == types::StatusCode::ERR_INSUFFICIENT_BUFFER);
}

TEST_CASE("AES256Algorithm encrypt multiple blocks", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::vector<uint8_t> plaintext(128, 0x42);  // 8 blocks
    std::vector<uint8_t> ciphertext(128, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Verify all bytes were encrypted
    for (size_t i = 0; i < 128; ++i) {
        REQUIRE(ciphertext[i] == (plaintext[i] ^ 0x5A));
    }
}

TEST_CASE("AES256Algorithm decrypt multiple blocks", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::vector<uint8_t> ciphertext(128, 0x42);  // 8 blocks
    std::vector<uint8_t> plaintext(128, 0);

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    // Verify all bytes were decrypted
    for (size_t i = 0; i < 128; ++i) {
        REQUIRE(plaintext[i] == (ciphertext[i] ^ 0x5A));
    }
}

TEST_CASE("AES256Algorithm encrypt single byte", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 1> plaintext{0x42};
    std::array<uint8_t, 1> ciphertext{0};

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(ciphertext[0] == (0x42 ^ 0x5A));
}

TEST_CASE("AES256Algorithm decrypt single byte", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 1> ciphertext{0x42};
    std::array<uint8_t, 1> plaintext{0};

    // Act
    auto status = algorithm.decrypt(ciphertext, plaintext);

    // Assert
    REQUIRE(status.isOk());
    REQUIRE(plaintext[0] == (0x42 ^ 0x5A));
}

TEST_CASE("AES256Algorithm input size not multiple of block size", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::vector<uint8_t> plaintext(20, 0x42);  // Not multiple of 16
    std::vector<uint8_t> ciphertext(20, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    // Mock implementation handles any size
    REQUIRE(ciphertext[0] == (plaintext[0] ^ 0x5A));
}

TEST_CASE("AES256Algorithm key size verification", "[aes][aes256]") {
    // Arrange
    Aes256Algorithm algorithm;

    // Act & Assert
    REQUIRE(algorithm.getKeySize() == 32U);
    REQUIRE(algorithm.getKeySize() == types::getKeySizeForAlgorithm(types::Algorithm::AES_256));
}

// =============================================================================
// AES Algorithm Comparison Tests
// =============================================================================

TEST_CASE("AES128 vs AES256 key size difference", "[aes][comparison]") {
    // Arrange
    Aes128Algorithm aes128;
    Aes256Algorithm aes256;

    // Act & Assert
    REQUIRE(aes128.getKeySize() == 16U);
    REQUIRE(aes256.getKeySize() == 32U);
    REQUIRE(aes128.getKeySize() != aes256.getKeySize());
}

TEST_CASE("AES128 vs AES256 block size same", "[aes][comparison]") {
    // Arrange
    Aes128Algorithm aes128;
    Aes256Algorithm aes256;

    // Act & Assert
    REQUIRE(aes128.getBlockSize() == 16U);
    REQUIRE(aes256.getBlockSize() == 16U);
    REQUIRE(aes128.getBlockSize() == aes256.getBlockSize());
}

TEST_CASE("AES128 vs AES256 algorithm type different", "[aes][comparison]") {
    // Arrange
    Aes128Algorithm aes128;
    Aes256Algorithm aes256;

    // Act & Assert
    REQUIRE(aes128.getAlgorithmType() == types::Algorithm::AES_128);
    REQUIRE(aes256.getAlgorithmType() == types::Algorithm::AES_256);
    REQUIRE(aes128.getAlgorithmType() != aes256.getAlgorithmType());
}

TEST_CASE("AES algorithms produce same encryption result (mock)", "[aes][comparison]") {
    // Arrange
    Aes128Algorithm aes128;
    Aes256Algorithm aes256;
    std::array<uint8_t, 16> plaintext{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
    std::array<uint8_t, 16> ciphertext128{0};
    std::array<uint8_t, 16> ciphertext256{0};

    // Act
    aes128.encrypt(plaintext, ciphertext128);
    aes256.encrypt(plaintext, ciphertext256);

    // Assert - Mock implementations use same XOR key
    REQUIRE(ciphertext128 == ciphertext256);
}

// =============================================================================
// AES Algorithm Edge Cases
// =============================================================================

TEST_CASE("AES128Algorithm encrypt large buffer", "[aes][aes128][edge]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::vector<uint8_t> plaintext(1024, 0xAB);
    std::vector<uint8_t> ciphertext(1024, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    for (size_t i = 0; i < 1024; ++i) {
        REQUIRE(ciphertext[i] == (plaintext[i] ^ 0x5A));
    }
}

TEST_CASE("AES256Algorithm encrypt large buffer", "[aes][aes256][edge]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::vector<uint8_t> plaintext(1024, 0xAB);
    std::vector<uint8_t> ciphertext(1024, 0);

    // Act
    auto status = algorithm.encrypt(plaintext, ciphertext);

    // Assert
    REQUIRE(status.isOk());
    for (size_t i = 0; i < 1024; ++i) {
        REQUIRE(ciphertext[i] == (plaintext[i] ^ 0x5A));
    }
}

TEST_CASE("AES128Algorithm encrypt/decrypt with all zero data", "[aes][aes128][edge]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> zeros{0};
    std::array<uint8_t, 16> encrypted{0};
    std::array<uint8_t, 16> decrypted{0};

    // Act
    algorithm.encrypt(zeros, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(zeros == decrypted);
    // Encrypted should not be all zeros (XOR with 0x5A)
    REQUIRE(encrypted[0] == 0x5A);
}

TEST_CASE("AES256Algorithm encrypt/decrypt with all zero data", "[aes][aes256][edge]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 32> zeros{0};
    std::array<uint8_t, 32> encrypted{0};
    std::array<uint8_t, 32> decrypted{0};

    // Act
    algorithm.encrypt(zeros, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(zeros == decrypted);
    // Encrypted should not be all zeros (XOR with 0x5A)
    REQUIRE(encrypted[0] == 0x5A);
}

TEST_CASE("AES128Algorithm encrypt/decrypt with all 0xFF data", "[aes][aes128][edge]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> allOnes;
    allOnes.fill(0xFF);
    std::array<uint8_t, 16> encrypted{0};
    std::array<uint8_t, 16> decrypted{0};

    // Act
    algorithm.encrypt(allOnes, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(allOnes == decrypted);
}

TEST_CASE("AES256Algorithm encrypt/decrypt with all 0xFF data", "[aes][aes256][edge]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 32> allOnes;
    allOnes.fill(0xFF);
    std::array<uint8_t, 32> encrypted{0};
    std::array<uint8_t, 32> decrypted{0};

    // Act
    algorithm.encrypt(allOnes, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(allOnes == decrypted);
}

TEST_CASE("AES128Algorithm encrypt/decrypt with alternating pattern", "[aes][aes128][edge]") {
    // Arrange
    Aes128Algorithm algorithm;
    std::array<uint8_t, 16> pattern;
    for (size_t i = 0; i < 16; ++i) {
        pattern[i] = (i % 2 == 0) ? 0xAA : 0x55;
    }
    std::array<uint8_t, 16> encrypted{0};
    std::array<uint8_t, 16> decrypted{0};

    // Act
    algorithm.encrypt(pattern, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(pattern == decrypted);
}

TEST_CASE("AES256Algorithm encrypt/decrypt with sequential pattern", "[aes][aes256][edge]") {
    // Arrange
    Aes256Algorithm algorithm;
    std::array<uint8_t, 32> pattern;
    for (size_t i = 0; i < 32; ++i) {
        pattern[i] = static_cast<uint8_t>(i);
    }
    std::array<uint8_t, 32> encrypted{0};
    std::array<uint8_t, 32> decrypted{0};

    // Act
    algorithm.encrypt(pattern, encrypted);
    algorithm.decrypt(encrypted, decrypted);

    // Assert
    REQUIRE(pattern == decrypted);
}

} // namespace ehsm::services::crypto
