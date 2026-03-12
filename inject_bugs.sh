#!/bin/bash
# Bug Injection Script for AI Debug Training
# Branch: ai-debug-training

set -e

echo "🐛 Injecting bugs into Embedded HSM codebase..."

# BUG-001: Buffer Overflow in keystore_service.cpp
echo "Injecting BUG-001: Buffer Overflow..."
sed -i 's/if (keyData.empty() || keyData.size() > types::kMaxKeySize) {/\/\/ BUG-001: Removed validation\n    \/\/ if (keyData.empty() || keyData.size() > types::kMaxKeySize) {/g' src/services/keystore_service.cpp
sed -i 's/return Status(types::StatusCode::ERR_INVALID_PARAM);/\/\/     return Status(types::StatusCode::ERR_INVALID_PARAM);/g' src/services/keystore_service.cpp
sed -i 's/^    }$/    \/\/ }/g' src/services/keystore_service.cpp

# BUG-002: Encrypt/Decrypt swapped
echo "Injecting BUG-002: Encrypt/Decrypt swapped..."
sed -i 's/return algorithm_->encrypt(input, output);/return algorithm_->decrypt(input, output); \/\/ BUG-002/g' src/services/crypto/crypto_service.cpp

# BUG-003: Missing null check
echo "Injecting BUG-003: Missing null check..."
sed -i 's/Status SessionManager::createSession(SessionId\& sessionId) {/Status SessionManager::createSession(SessionId\& sessionId) {\n    \/\/ BUG-003: No null check\n    \/\/ if (\&sessionId == nullptr) return ERR_INVALID_PARAM;/g' src/middleware/session_manager.cpp

# BUG-004: Off-by-one error
echo "Injecting BUG-004: Off-by-one error..."
sed -i 's/output.size() < input.size()/output.size() <= input.size() \/\/ BUG-004/g' src/services/crypto/crypto_service.cpp

# BUG-005: Uninitialized variable
echo "Injecting BUG-005: Uninitialized variable..."
sed -i 's/bool found = false;/bool found; \/\/ BUG-005: Uninitialized/g' src/middleware/session_manager.cpp

# BUG-006: Memory leak
echo "Injecting BUG-006: Memory leak..."
sed -i 's/auto status = sessionManager_->init();/auto* tempBuffer = new uint8_t[1024]; \/\/ BUG-006\n    auto status = sessionManager_->init();/g' src/api/hsm_api_impl.cpp

# BUG-007: Wrong enum return
echo "Injecting BUG-007: Wrong enum return..."
sed -i 's/return algorithm_ ? algorithm_->getAlgorithmType() : types::Algorithm::NONE;/return types::Algorithm::AES_128; \/\/ BUG-007: Always returns AES_128/g' src/services/crypto/crypto_service.cpp

# BUG-008: Sign extension
echo "Injecting BUG-008: Sign extension..."
sed -i 's/key = slot.key;/int slotIndex = keySlotId - 1; \/\/ BUG-008\n    if (slotIndex >= 0 \&\& slotIndex < kMaxKeySlots) {\n        key = keySlots_[slotIndex].key;\n    } else {\n        key = slot.key;\n    }/g' src/services/keystore_service.cpp

# BUG-009: Race condition
echo "Injecting BUG-009: Race condition..."
sed -i 's/slot.accessCount++;/std::this_thread::sleep_for(std::chrono::milliseconds(1)); \/\/ BUG-009\n    slot.accessCount++;/g' src/services/keystore_service.cpp

# BUG-010: Wrong logic operator
echo "Injecting BUG-010: Wrong logic operator..."
sed -i 's/if (session.id == sessionId \&\& session.isActive) {/if (session.id == sessionId || session.isActive) { \/\/ BUG-010/g' src/middleware/session_manager.cpp

echo "✅ Bug injection complete!"
echo ""
echo "📊 Injected 10 bugs:"
echo "  - BUG-001: Buffer Overflow (Critical)"
echo "  - BUG-002: Encrypt/Decrypt Swapped (Critical)"
echo "  - BUG-003: Null Pointer (High)"
echo "  - BUG-004: Off-by-One (Medium)"
echo "  - BUG-005: Uninitialized Variable (Medium)"
echo "  - BUG-006: Memory Leak (Medium)"
echo "  - BUG-007: Wrong Enum (Low)"
echo "  - BUG-008: Sign Extension (Low)"
echo "  - BUG-009: Race Condition (High)"
echo "  - BUG-010: Wrong Logic (Low)"
echo ""
echo "🔨 Next steps:"
echo "  1. Build: cmake --build build"
echo "  2. Run tests: ctest --output-on-failure"
echo "  3. Debug with extension"
