/**
 * @file hsm_api_impl.hpp
 * @brief HSM API implementation header.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "ehsm/api/ihsm_api.hpp"
#include "ehsm/services/icrypto_service.hpp"
#include "ehsm/services/ikeystore_service.hpp"
#include <memory>

namespace ehsm::mw {
class ISessionManager;
}

namespace ehsm::api {

/**
 * @brief HSM API implementation (Facade pattern).
 * 
 * This class implements the IHsmApi interface and acts as a facade,
 * delegating work to the middleware and service layers.
 */
class HsmApiImpl final : public IHsmApi {
public:
    /**
     * @brief Construct HsmApiImpl with dependencies.
     * @param sessionManager Session manager for session handling.
     * @param cryptoService Crypto service for encryption/decryption.
     * @param keystoreService Keystore service for key management.
     */
    explicit HsmApiImpl(
        std::shared_ptr<mw::ISessionManager> sessionManager,
        std::shared_ptr<services::ICryptoService> cryptoService,
        std::shared_ptr<services::IKeystoreService> keystoreService);

    /**
     * @brief Default destructor.
     */
    ~HsmApiImpl() override = default;

    /**
     * @brief Delete copy operations.
     */
    HsmApiImpl(const HsmApiImpl&) = delete;
    HsmApiImpl& operator=(const HsmApiImpl&) = delete;

    // IHsmApi implementation
    [[nodiscard]] Status init() override;
    [[nodiscard]] Status deinit() override;
    [[nodiscard]] bool isReady() const override;
    [[nodiscard]] Status createSession(types::SessionId& sessionId) override;
    [[nodiscard]] Status closeSession(types::SessionId sessionId) override;
    [[nodiscard]] bool isSessionValid(types::SessionId sessionId) const override;
    
    [[nodiscard]] Status encrypt(
        types::SessionId sessionId,
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) override;
    
    [[nodiscard]] Status decrypt(
        types::SessionId sessionId,
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> input,
        std::span<uint8_t> output) override;
    
    [[nodiscard]] Status importKey(
        uint8_t keySlotId,
        types::Algorithm algorithm,
        std::span<const uint8_t> keyData,
        uint8_t permissions) override;
    
    [[nodiscard]] Status deleteKey(uint8_t keySlotId) override;
    [[nodiscard]] Status getKeySlotInfo(uint8_t keySlotId, types::KeySlotInfo& info) override;
    [[nodiscard]] Status clearAllKeys() override;
    [[nodiscard]] const char* getVersion() const override;

private:
    std::shared_ptr<mw::ISessionManager> sessionManager_;
    std::shared_ptr<services::ICryptoService> cryptoService_;
    std::shared_ptr<services::IKeystoreService> keystoreService_;
    bool isInitialized_ = false;
};

} // namespace ehsm::api
