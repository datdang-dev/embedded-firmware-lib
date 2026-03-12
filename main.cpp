/**
 * @file main.cpp
 * @brief Main entry point for Embedded HSM Firmware on HOST platform.
 * 
 * This is the composition root where all dependencies are wired together.
 * 
 * Build: cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build
 * Run:   ./build/ehsm_host
 * Debug: gdb ./build/ehsm_host
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "common.hpp"
#include "ihsm_api.hpp"
#include "hsm_api_impl.hpp"
#include "session_manager.hpp"
#include "crypto_service.hpp"
#include "aes256_algorithm.hpp"
#include "keystore_service.hpp"

#include <iostream>
#include <array>
#include <cstring>
#include <cstdlib>

using namespace ehsm;

// Test key for AES-256
inline constexpr std::array<uint8_t, 32> kTestKey = {
    0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U,
    0x08U, 0x09U, 0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU, 0x0FU,
    0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U,
    0x18U, 0x19U, 0x1AU, 0x1BU, 0x1CU, 0x1DU, 0x1EU, 0x1FU
};

inline constexpr const char* kPlaintext = "Hello, Embedded HSM C++! This is a secret message.";
inline constexpr uint8_t kTestKeySlotId = 0U;

[[nodiscard]] types::Status runEncryptDecryptDemo(
    api::IHsmApi& hsm,
    types::SessionId sessionId);

void printHsmInfo();

int main() {
    std::cout << "=================================================\n";
    std::cout << "  Embedded HSM Firmware - HOST Platform Demo\n";
    std::cout << "  C++ Implementation with Design Patterns\n";
    std::cout << "=================================================\n\n";

    // =========================================================================
    // Composition Root - Wire all dependencies
    // =========================================================================
    
    // Create services
    auto keystoreService = std::make_shared<services::KeystoreService>();
    auto cryptoAlgorithm = std::make_unique<services::crypto::Aes256Algorithm>();
    auto cryptoService = std::make_unique<services::CryptoService>(
        keystoreService, std::move(cryptoAlgorithm));
    auto sessionManager = std::make_unique<mw::SessionManager>();
    
    // Create API (Facade)
    auto hsmApi = std::make_unique<api::HsmApiImpl>(
        std::move(sessionManager),
        std::move(cryptoService),
        keystoreService);

    // =========================================================================
    // Initialize HSM
    // =========================================================================
    std::cout << "[MAIN] Initializing HSM subsystem...\n";
    types::Status status = hsmApi->init();
    if (!status.isOk()) {
        std::cout << "[MAIN] ERROR: init failed: " << status.message() << "\n";
        return EXIT_FAILURE;
    }
    
    std::cout << "[MAIN] HSM initialized successfully (version: " 
              << hsmApi->getVersion() << ")\n\n";
    
    printHsmInfo();

    // =========================================================================
    // Create session
    // =========================================================================
    std::cout << "[MAIN] Creating HSM session...\n";
    types::SessionId sessionId;
    status = hsmApi->createSession(sessionId);
    if (!status.isOk()) {
        std::cout << "[MAIN] ERROR: createSession failed: " << status.message() << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "[MAIN] Session created: ID=0x" << std::hex << sessionId << std::dec << "\n\n";

    // =========================================================================
    // Import key
    // =========================================================================
    std::cout << "[MAIN] Importing AES-256 key into slot " 
              << static_cast<int>(kTestKeySlotId) << "...\n";
    status = hsmApi->importKey(
        kTestKeySlotId,
        types::Algorithm::AES_256,
        kTestKey,
        static_cast<uint8_t>(types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT));
    if (!status.isOk()) {
        std::cout << "[MAIN] ERROR: importKey failed: " << status.message() << "\n";
        return EXIT_FAILURE;
    }
    std::cout << "[MAIN] Key imported successfully\n\n";

    // =========================================================================
    // Run demo
    // =========================================================================
    std::cout << "[MAIN] Running encryption/decryption demonstration...\n";
    std::cout << "-------------------------------------------------\n";
    status = runEncryptDecryptDemo(*hsmApi, sessionId);
    std::cout << "-------------------------------------------------\n\n";

    if (!status.isOk()) {
        std::cout << "[MAIN] ERROR: Demo failed: " << status.message() << "\n";
        return EXIT_FAILURE;
    }

    // =========================================================================
    // Cleanup
    // =========================================================================
    std::cout << "[MAIN] Cleaning up...\n";
    
    std::cout << "[MAIN] Deleting key from slot " 
              << static_cast<int>(kTestKeySlotId) << "...\n";
    (void)hsmApi->deleteKey(kTestKeySlotId);
    
    std::cout << "[MAIN] Closing session...\n";
    (void)hsmApi->closeSession(sessionId);
    
    std::cout << "[MAIN] Deinitializing HSM...\n";
    (void)hsmApi->deinit();

    std::cout << "\n=================================================\n";
    std::cout << "  Demo completed successfully!\n";
    std::cout << "=================================================\n";

    return EXIT_SUCCESS;
}

types::Status runEncryptDecryptDemo(
    api::IHsmApi& hsm,
    types::SessionId sessionId)
{
    std::array<uint8_t, 256> ciphertext{};
    std::array<uint8_t, 256> decrypted{};
    
    const auto* plaintext = reinterpret_cast<const uint8_t*>(kPlaintext);
    const size_t plaintextLen = std::strlen(kPlaintext);

    std::cout << "\n[DEMO] Original plaintext:\n";
    std::cout << "       \"" << kPlaintext << "\"\n";
    std::cout << "       Length: " << plaintextLen << " bytes\n";

    // Encrypt
    std::cout << "\n[DEMO] Calling encrypt()...\n";
    types::Status status = hsm.encrypt(
        sessionId,
        kTestKeySlotId,
        types::Algorithm::AES_256,
        std::span<const uint8_t>(plaintext, plaintextLen),
        ciphertext);

    if (!status.isOk()) {
        std::cout << "[DEMO] ERROR: encrypt failed: " << status.message() << "\n";
        return status;
    }

    std::cout << "[DEMO] Encryption successful!\n";
    std::cout << "       Ciphertext: ";
    for (size_t i = 0U; i < plaintextLen && i < 32U; ++i) {
        std::printf("%02X", ciphertext[i]);
        if ((i + 1U) % 8U == 0U) std::printf(" ");
    }
    std::printf("\n");

    // Decrypt
    std::cout << "\n[DEMO] Calling decrypt()...\n";
    status = hsm.decrypt(
        sessionId,
        kTestKeySlotId,
        types::Algorithm::AES_256,
        std::span<const uint8_t>(ciphertext.data(), plaintextLen),
        decrypted);

    if (!status.isOk()) {
        std::cout << "[DEMO] ERROR: decrypt failed: " << status.message() << "\n";
        return status;
    }

    std::cout << "[DEMO] Decryption successful!\n";
    decrypted[plaintextLen] = '\0';
    std::cout << "       Decrypted text: \"" << decrypted.data() << "\"\n";

    // Verify
    std::cout << "\n[DEMO] Verifying decryption...\n";
    if (std::memcmp(plaintext, decrypted.data(), plaintextLen) != 0) {
        std::cout << "[DEMO] ERROR: Data mismatch!\n";
        return types::Status(types::StatusCode::ERR_CRYPTO_FAILED);
    }

    std::cout << "[DEMO] Verification PASSED - decrypted data matches original!\n";
    return types::Status(types::StatusCode::OK);
}

void printHsmInfo() {
    std::cout << "HSM Configuration:\n";
    std::cout << "  - Max Key Slots:     " << static_cast<int>(types::kMaxKeySlots) << "\n";
    std::cout << "  - Max Sessions:      " << static_cast<int>(types::kMaxSessions) << "\n";
    std::cout << "  - Max Key Size:      " << types::kMaxKeySize << " bytes\n";
    std::cout << "\n";
}
