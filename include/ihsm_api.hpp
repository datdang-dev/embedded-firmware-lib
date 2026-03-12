/**
 * @file ihsm_api.hpp
 * @brief Public HSM API interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "common.hpp"
#include <span>

namespace ehsm::api {

/**
 * @brief Public interface for HSM operations.
 * 
 * This is the facade interface that applications use to interact
 * with the HSM subsystem. All operations are performed through
 * this interface.
 */
class IHsmApi {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IHsmApi() = default;

    /**
     * @brief Initialize the HSM subsystem.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status init() = 0;

    /**
     * @brief Deinitialize the HSM subsystem.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status deinit() = 0;

    /**
     * @brief Check if HSM is initialized.
     * @return true if initialized, false otherwise.
     */
    [[nodiscard]] virtual bool isReady() const = 0;

    /**
     * @brief Create a new HSM session.
     * @param[out] sessionId Reference to store the new session ID.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status createSession(types::SessionId& sessionId) = 0;

    /**
     * @brief Close an existing HSM session.
     * @param[in] sessionId Session ID to close.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status closeSession(types::SessionId sessionId) = 0;

    /**
     * @brief Check if a session ID is valid.
     * @param[in] sessionId Session ID to validate.
     * @return true if valid, false otherwise.
     */
    [[nodiscard]] virtual bool isSessionValid(types::SessionId sessionId) const = 0;

    /**
     * @brief Encrypt data using the specified key and algorithm.
     * @param[in] sessionId Valid session ID.
     * @param[in] keySlotId ID of the key slot containing the encryption key.
     * @param[in] algorithm Encryption algorithm to use.
     * @param[in] input Input plaintext data.
     * @param[out] output Output ciphertext buffer.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status encrypt(
        types::SessionId sessionId,
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) = 0;

    /**
     * @brief Decrypt data using the specified key and algorithm.
     * @param[in] sessionId Valid session ID.
     * @param[in] keySlotId ID of the key slot containing the decryption key.
     * @param[in] algorithm Decryption algorithm to use.
     * @param[in] input Input ciphertext data.
     * @param[out] output Output plaintext buffer.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status decrypt(
        types::SessionId sessionId,
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) = 0;

    /**
     * @brief Import a key into a key slot.
     * @param[in] keySlotId Target key slot ID.
     * @param[in] algorithm Algorithm associated with the key.
     * @param[in] keyData Key data span.
     * @param[in] permissions Access permission flags.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status importKey(
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> keyData,
        uint8_t permissions) = 0;

    /**
     * @brief Delete a key from a key slot.
     * @param[in] keySlotId Key slot ID to clear.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status deleteKey(uint8_t keySlotId) = 0;

    /**
     * @brief Get information about a key slot.
     * @param[in] keySlotId Key slot ID to query.
     * @param[out] info Reference to store key slot info.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status getKeySlotInfo(
        uint8_t keySlotId,
        types::KeySlotInfo& info) = 0;

    /**
     * @brief Clear all keys from the keystore.
     * @return Status::OK on success, error code otherwise.
     */
    [[nodiscard]] virtual Status clearAllKeys() = 0;

    /**
     * @brief Get HSM firmware version.
     * @return Version string (static, do not free).
     */
    [[nodiscard]] virtual const char* getVersion() const = 0;
};

} // namespace ehsm::api
