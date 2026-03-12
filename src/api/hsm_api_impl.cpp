/**
 * @file hsm_api_impl.cpp
 * @brief HSM API implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "common.hpp"
#include "hsm_api_impl.hpp"
#include "isession_manager.hpp"

namespace ehsm::api {

inline constexpr const char* kVersionString = "1.0.0";

HsmApiImpl::HsmApiImpl(
    std::shared_ptr<mw::ISessionManager> sessionManager,
    std::shared_ptr<services::ICryptoService> cryptoService,
    std::shared_ptr<services::IKeystoreService> keystoreService)
    : sessionManager_(std::move(sessionManager))
    , cryptoService_(std::move(cryptoService))
    , keystoreService_(std::move(keystoreService))
{
}

Status HsmApiImpl::init() {
    if (isInitialized_) {
        return Status(types::StatusCode::OK);
    }

    if (!sessionManager_ || !cryptoService_ || !keystoreService_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    Status status = sessionManager_->init();
    if (!status.isOk()) {
        return status;
    }

    status = keystoreService_->init();
    if (!status.isOk()) {
        return status;
    }

    status = cryptoService_->init();
    if (!status.isOk()) {
        return status;
    }

    isInitialized_ = true;
    return Status(types::StatusCode::OK);
}

Status HsmApiImpl::deinit() {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    Status status = cryptoService_->deinit();
    if (!status.isOk()) {
        return status;
    }

    status = keystoreService_->deinit();
    if (!status.isOk()) {
        return status;
    }

    status = sessionManager_->deinit();
    if (!status.isOk()) {
        return status;
    }

    isInitialized_ = false;
    return Status(types::StatusCode::OK);
}

bool HsmApiImpl::isReady() const {
    return isInitialized_;
}

Status HsmApiImpl::createSession(types::SessionId& sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return sessionManager_->createSession(sessionId);
}

Status HsmApiImpl::closeSession(types::SessionId sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return sessionManager_->closeSession(sessionId);
}

bool HsmApiImpl::isSessionValid(types::SessionId sessionId) const {
    if (!isInitialized_) {
        return false;
    }
    return sessionManager_->isSessionValid(sessionId);
}

Status HsmApiImpl::encrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }

    if (!isSessionValid(sessionId)) {
        return Status(types::StatusCode::ERR_SESSION_INVALID);
    }

    if (keySlotId >= types::kMaxKeySlots) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    // Check key permissions
    types::KeySlotInfo info;
    Status status = keystoreService_->getSlotInfo(keySlotId, info);
    if (!status.isOk()) {
        return status;
    }

    if ((info.permissions & static_cast<uint8_t>(types::KeyPermission::ENCRYPT)) == 0U) {
        return Status(types::StatusCode::ERR_AUTH_FAILED);
    }

    (void)algorithm;  // Algorithm is handled by CryptoService
    return cryptoService_->encrypt(input, output, keySlotId);
}

Status HsmApiImpl::decrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }

    if (input.empty() || output.size() < input.size()) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }

    if (!isSessionValid(sessionId)) {
        return Status(types::StatusCode::ERR_SESSION_INVALID);
    }

    if (keySlotId >= types::kMaxKeySlots) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }

    // Check key permissions
    types::KeySlotInfo info;
    Status status = keystoreService_->getSlotInfo(keySlotId, info);
    if (!status.isOk()) {
        return status;
    }

    if ((info.permissions & static_cast<uint8_t>(types::KeyPermission::DECRYPT)) == 0U) {
        return Status(types::StatusCode::ERR_AUTH_FAILED);
    }

    (void)algorithm;  // Algorithm is handled by CryptoService
    return cryptoService_->decrypt(input, output, keySlotId);
}

Status HsmApiImpl::importKey(
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> keyData,
    uint8_t permissions)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return keystoreService_->importKey(keySlotId, algorithm, keyData, permissions);
}

Status HsmApiImpl::deleteKey(uint8_t keySlotId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return keystoreService_->deleteKey(keySlotId);
}

Status HsmApiImpl::getKeySlotInfo(
    uint8_t keySlotId,
    types::KeySlotInfo& info)
{
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return keystoreService_->getSlotInfo(keySlotId, info);
}

Status HsmApiImpl::clearAllKeys() {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    return keystoreService_->clearAll();
}

const char* HsmApiImpl::getVersion() const {
    return kVersionString;
}

} // namespace ehsm::api
