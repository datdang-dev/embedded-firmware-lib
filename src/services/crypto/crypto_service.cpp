/**
 * @file crypto_service.cpp
 * @brief CryptoService implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "crypto_service.hpp"
#include "ikeystore_service.hpp"
#include "icrypto_algorithm.hpp"

namespace ehsm::services {

CryptoService::CryptoService(
    std::shared_ptr<IKeystoreService> keystore,
    std::unique_ptr<ICryptoAlgorithm> algorithm)
    : keystore_(std::move(keystore))
    , algorithm_(std::move(algorithm))
{
}

Status CryptoService::init() {
    return Status(types::StatusCode::OK);
}

Status CryptoService::deinit() {
    return Status(types::StatusCode::OK);
}

Status CryptoService::encrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output,
    uint8_t keySlotId)
{
    // Validate algorithm
    if (!algorithm_) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }

    // Validate buffers
    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INSUFFICIENT_BUFFER);
    }

    // Validate keystore
    if (!keystore_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    // Get key from keystore (validates key slot)
    types::Key key;
    Status status = keystore_->getKey(keySlotId, key);
    if (!status.isOk()) {
        return status;
    }

    // Perform encryption using strategy
    status = algorithm_->encrypt(input, output);
    
    // Securely clear key from memory
    key.secureClear();

    return status;
}

Status CryptoService::decrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output,
    uint8_t keySlotId)
{
    // Validate algorithm
    if (!algorithm_) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }

    // Validate buffers
    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INSUFFICIENT_BUFFER);
    }

    // Validate keystore
    if (!keystore_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    // Get key from keystore (validates key slot)
    types::Key key;
    Status status = keystore_->getKey(keySlotId, key);
    if (!status.isOk()) {
        return status;
    }

    // Perform decryption using strategy
    status = algorithm_->decrypt(input, output);
    
    // Securely clear key from memory
    key.secureClear();

    return status;
}

Status CryptoService::hash(
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    // Hash not implemented in this mock
    (void)algorithm;
    (void)input;
    (void)output;
    return Status(types::StatusCode::ERR_UNSUPPORTED);
}

types::Algorithm CryptoService::getAlgorithmType() const {
    return algorithm_ ? algorithm_->getAlgorithmType() : types::Algorithm::NONE;
}

} // namespace ehsm::services
