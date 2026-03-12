/**
 * @file algorithm.hpp
 * @brief Algorithm type definitions.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace ehsm::types {

/**
 * @brief Supported encryption algorithms.
 */
enum class Algorithm : uint8_t {
    NONE = 0,
    AES_128 = 1,
    AES_256 = 2,
    RSA_2048 = 3,
    RSA_4096 = 4,
    ECC_P256 = 5,
    SHA_256 = 6,
    CHACHA20 = 7
};

/**
 * @brief Key permission flags.
 */
enum class KeyPermission : uint8_t {
    NONE = 0x00,
    ENCRYPT = 0x01,
    DECRYPT = 0x02,
    SIGN = 0x04,
    VERIFY = 0x08,
    EXPORT = 0x10,
    ALL = 0xFF
};

/**
 * @brief Enable bitwise OR for KeyPermission.
 */
inline KeyPermission operator|(KeyPermission lhs, KeyPermission rhs) {
    return static_cast<KeyPermission>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

/**
 * @brief Enable bitwise AND for KeyPermission.
 */
inline KeyPermission operator&(KeyPermission lhs, KeyPermission rhs) {
    return static_cast<KeyPermission>(
        static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}

/**
 * @brief Get key size for algorithm.
 * @param algorithm Algorithm type.
 * @return Key size in bytes, 0 if unknown/variable.
 */
[[nodiscard]] constexpr size_t getKeySizeForAlgorithm(Algorithm algorithm) {
    switch (algorithm) {
        case Algorithm::AES_128:
            return 16U;
        case Algorithm::AES_256:
        case Algorithm::CHACHA20:
            return 32U;
        case Algorithm::RSA_2048:
            return 256U;
        case Algorithm::RSA_4096:
            return 512U;
        case Algorithm::ECC_P256:
            return 32U;
        default:
            return 0U;
    }
}

} // namespace ehsm::types
