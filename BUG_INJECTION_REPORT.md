# 🐛 AI Debug Training - Bug Injection Report

**Branch:** `ai-debug-training`  
**Date:** March 13, 2026  
**Purpose:** Test AI VSCode Debug Extension's ability to find bugs  
**Total Bugs Injected:** 10

---

## 🎯 BUG INJECTION STRATEGY

### Categories of Bugs:
1. **Memory Safety** (Buffer overflow, use-after-free)
2. **Logic Errors** (Wrong conditions, off-by-one)
3. **Null Pointer** (Missing checks)
4. **Race Conditions** (Data races)
5. **Resource Leaks** (Memory leaks)
6. **Type Errors** (Sign extension, wrong casts)

---

## 📋 INJECTED BUGS

### BUG-001: Buffer Overflow in Keystore (CRITICAL) 🔴

**Location:** `src/services/keystore_service.cpp::importKey()`

**Bug:** Removed key size validation
```cpp
// ORIGINAL (SAFE):
if (keyData.empty() || keyData.size() > types::kMaxKeySize) {
    return Status(types::StatusCode::ERR_INVALID_PARAM);
}

// BUGGY (REMOVED):
// Validation removed - allows overflow!
```

**Impact:** Can write beyond `slot.key.data` buffer (64 bytes)

**Test Case:**
```cpp
TEST_CASE("BUG-001 - Buffer overflow", "[bug]") {
    std::array<uint8_t, 128> oversizedKey = {0};  // > kMaxKeySize (64)
    auto status = keystore.importKey(0, Algorithm::AES_256, oversizedKey, 0xFF);
    // Should fail but doesn't!
    REQUIRE(status.isOk());  // ❌ Should be ERR_INVALID_PARAM
}
```

**Expected AI Debug Output:**
- Crash or memory corruption detected
- GDB stops at memcpy with segfault
- AI should identify: "Buffer overflow in keystore_service.cpp:68"

---

### BUG-002: Encryption/Decryption Swapped (CRITICAL) 🔴

**Location:** `src/services/crypto/crypto_service.cpp::encrypt()`

**Bug:** Calls decrypt instead of encrypt
```cpp
Status CryptoService::encrypt(...) {
    // BUG-002: Wrong algorithm method called!
    return algorithm_->decrypt(input, output);  // ❌ Should be encrypt()
}
```

**Impact:** Encrypt returns garbage, decrypt also returns garbage

**Test Case:**
```cpp
TEST_CASE("BUG-002 - Encrypt/Decrypt swapped", "[bug]") {
    std::array<uint8_t, 16> plaintext = {0x41, 0x42, 0x43};
    std::array<uint8_t, 16> ciphertext = {0};
    
    auto status = crypto->encrypt(plaintext, ciphertext, 0);
    REQUIRE(status.isOk());
    
    // BUG: ciphertext == plaintext (because decrypt called on plaintext)
    REQUIRE(ciphertext != plaintext);  // ❌ FAILS - they're equal!
}
```

**Expected AI Debug Output:**
- Test fails with assertion
- AI should trace: encrypt() → algorithm_->decrypt() [WRONG!]

---

### BUG-003: Null Pointer Dereference (HIGH) 🟡

**Location:** `src/middleware/session_manager.cpp::createSession()`

**Bug:** Missing null check
```cpp
Status SessionManager::createSession(SessionId& sessionId) {
    // BUG-003: No check if sessionId is null!
    // if (&sessionId == nullptr) return ERR_INVALID_PARAM;
    
    for (auto& session : sessions_) {
        if (!session.isActive) {
            session.id = nextSessionId_++;
            sessionId = session.id;  // ❌ Crashes if sessionId is null
            return Status(StatusCode::OK);
        }
    }
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-003 - Null pointer", "[bug]") {
    SessionId* nullSessionId = nullptr;
    auto status = sessionMgr->createSession(*nullSessionId);  // ❌ CRASH
}
```

**Expected AI Debug Output:**
- Segmentation fault
- GDB backtrace shows: session_manager.cpp:52
- AI should identify: "Null pointer dereference"

---

### BUG-004: Off-by-One Error (MEDIUM) 🟠

**Location:** `src/services/crypto/crypto_service.cpp::encrypt()`

**Bug:** Wrong buffer size check
```cpp
Status CryptoService::encrypt(...) {
    if (input.empty() || output.size() <= input.size()) {  // ❌ Should be <
        return Status(types::StatusCode::ERR_INSUFFICIENT_BUFFER);
    }
    // ...
}
```

**Impact:** Rejects valid buffers where output.size() == input.size()

**Test Case:**
```cpp
TEST_CASE("BUG-004 - Off-by-one", "[bug]") {
    std::array<uint8_t, 16> plaintext = {0};
    std::array<uint8_t, 16> ciphertext = {0};  // Exact size
    
    auto status = crypto->encrypt(plaintext, ciphertext, 0);
    REQUIRE(status.isOk());  // ❌ FAILS - returns ERR_INSUFFICIENT_BUFFER
}
```

**Expected AI Debug Output:**
- Test fails
- AI should identify: "Buffer size check uses <= instead of <"

---

### BUG-005: Uninitialized Variable (MEDIUM) 🟠

**Location:** `src/middleware/session_manager.cpp::isSessionValid()`

**Bug:** Variable not initialized
```cpp
bool SessionManager::isSessionValid(SessionId sessionId) const {
    bool found;  // ❌ BUG-005: Not initialized!
    
    for (const auto& session : sessions_) {
        if (session.id == sessionId && session.isActive) {
            found = true;
            break;
        }
    }
    
    return found;  // ❌ Returns garbage if not found
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-005 - Uninitialized variable", "[bug]") {
    bool result = sessionMgr->isSessionValid(999);  // Invalid session
    REQUIRE(result == false);  // ❌ May return true (garbage value)
}
```

**Expected AI Debug Output:**
- Inconsistent test results
- GDB shows: `print found` → random value
- AI should identify: "Uninitialized variable in line 92"

---

### BUG-006: Resource Leak (MEDIUM) 🟠

**Location:** `src/api/hsm_api_impl.cpp::init()`

**Bug:** Memory leak on error path
```cpp
Status HsmApiImpl::init() {
    auto* tempBuffer = new uint8_t[1024];  // Allocated
    
    auto status = sessionManager_->init();
    if (!status.isOk()) {
        return status;  // ❌ BUG-006: tempBuffer not deleted!
    }
    
    status = keystoreService_->init();
    if (!status.isOk()) {
        return status;  // ❌ BUG-006: tempBuffer not deleted!
    }
    
    delete[] tempBuffer;  // Only reached if all succeed
    return Status(StatusCode::OK);
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-006 - Memory leak", "[bug]") {
    // Mock sessionManager to fail
    EXPECT_CALL(*mockSessionMgr, init())
        .WillOnce(Return(Status(StatusCode::ERR_HARDWARE)));
    
    auto status = hsmApi->init();
    // Memory leaked! (1024 bytes)
}
```

**Expected AI Debug Output:**
- Valgrind reports: "1024 bytes lost"
- AI should trace error path and identify leak

---

### BUG-007: Wrong Enum Value (LOW) 🟢

**Location:** `src/services/crypto/crypto_service.cpp::getAlgorithmType()`

**Bug:** Returns wrong algorithm
```cpp
types::Algorithm CryptoService::getAlgorithmType() const {
    return types::Algorithm::AES_128;  // ❌ BUG-007: Always returns AES_128
    // Should return: algorithm_->getAlgorithmType()
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-007 - Wrong enum", "[bug]") {
    auto crypto256 = std::make_unique<CryptoService>(
        keystore, std::make_unique<Aes256Algorithm>());
    
    auto algo = crypto256->getAlgorithmType();
    REQUIRE(algo == types::Algorithm::AES_256);  // ❌ FAILS - returns AES_128
}
```

**Expected AI Debug Output:**
- Test fails
- AI should identify: "Hardcoded return value"

---

### BUG-008: Sign Extension Error (LOW) 🟢

**Location:** `src/services/keystore_service.cpp::getKey()`

**Bug:** Signed/unsigned conversion
```cpp
Status KeystoreService::getKey(uint8_t keySlotId, types::Key& key) {
    // BUG-008: keySlotId is uint8_t but used in int expression
    int slotIndex = keySlotId - 1;  // ❌ If keySlotId=0, becomes 255!
    
    if (slotIndex >= 0 && slotIndex < kMaxKeySlots) {
        // Accesses wrong slot!
        key = keySlots_[slotIndex].key;
    }
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-008 - Sign extension", "[bug]") {
    types::Key key;
    auto status = keystore.getKey(0, key);  // slotIndex = 255!
    // Should fail but accesses slot 255 (out of bounds)
}
```

**Expected AI Debug Output:**
- Out-of-bounds access
- AI should identify: "Integer underflow in subtraction"

---

### BUG-009: Race Condition (HIGH) 🟡

**Location:** `src/services/keystore_service.cpp::getKey()`

**Bug:** No synchronization
```cpp
Status KeystoreService::getKey(uint8_t keySlotId, types::Key& key) {
    auto& slot = keySlots_[keySlotId];
    
    // BUG-009: Race condition - another thread could delete key here
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    key = slot.key;  // ❌ May read partially-modified data
    slot.accessCount++;  // ❌ Data race
    return Status(StatusCode::OK);
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-009 - Race condition", "[bug][thread]") {
    std::thread t1([&]() { keystore.importKey(0, AES_256, keyData, 0xFF); });
    std::thread t2([&]() { 
        types::Key key; 
        keystore.getKey(0, key); 
    });
    
    t1.join(); t2.join();
    // May crash or return corrupted key
}
```

**Expected AI Debug Output:**
- Thread sanitizer reports data race
- AI should identify: "Unsynchronized access to shared data"

---

### BUG-010: Logic Error in Validation (LOW) 🟢

**Location:** `src/middleware/session_manager.cpp::closeSession()`

**Bug:** Wrong logic operator
```cpp
Status SessionManager::closeSession(SessionId sessionId) {
    for (auto& session : sessions_) {
        if (session.id == sessionId || session.isActive) {  // ❌ Should be &&
            session.isActive = false;
            return Status(StatusCode::OK);
        }
    }
    return Status(StatusCode::ERR_SESSION_INVALID);
}
```

**Test Case:**
```cpp
TEST_CASE("BUG-010 - Wrong logic", "[bug]") {
    // Create session 1
    SessionId id1;
    sessionMgr->createSession(id1);
    
    // Try to close session 999 (doesn't exist)
    auto status = sessionMgr->closeSession(999);
    REQUIRE(status.isOk() == false);  // ❌ FAILS - succeeds because of ||
}
```

**Expected AI Debug Output:**
- Test fails
- AI should identify: "Wrong logic operator: || should be &&"

---

## 🎯 AI DEBUG TESTING WORKFLOW

### Step 1: Build with Bugs
```bash
cd /home/datdang/working/common_dev/embedded_hsm
cmake -DTARGET_PLATFORM=HOST -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
```

### Step 2: Run Tests (They Should Fail)
```bash
cd build
ctest --output-on-failure
# Expected: Multiple test failures
```

### Step 3: Debug with Extension
```bash
# Launch debug session via extension API
curl -X POST http://localhost:9999/api/debug \
  -d '{
    "operation": "launch",
    "params": {
      "program": "./build/tests/test_crypto_service",
      "stopOnEntry": false
    }
  }'

# Set breakpoints at failure points
# Step through code
# Inspect variables
```

### Step 4: AI Analysis
```bash
# Ask AI to analyze crash
# AI should:
# 1. Identify crash location
# 2. Trace back to root cause
# 3. Suggest fix
```

---

## 📊 EXPECTED AI DEBUG PERFORMANCE

| Bug ID | Difficulty | AI Should Find | Time Estimate |
|--------|------------|----------------|---------------|
| BUG-001 | 🔴 Critical | Buffer overflow | < 2 min |
| BUG-002 | 🔴 Critical | Wrong function call | < 1 min |
| BUG-003 | 🟡 High | Null pointer | < 1 min |
| BUG-004 | 🟠 Medium | Off-by-one | < 2 min |
| BUG-005 | 🟠 Medium | Uninitialized var | < 3 min |
| BUG-006 | 🟠 Medium | Memory leak | < 5 min |
| BUG-007 | 🟢 Low | Wrong return value | < 1 min |
| BUG-008 | 🟢 Low | Sign extension | < 2 min |
| BUG-009 | 🟡 High | Race condition | < 10 min |
| BUG-010 | 🟢 Low | Logic operator | < 1 min |

---

## ✅ SUCCESS CRITERIA

AI Debug Extension thành công nếu:

1. ✅ **Phát hiện được 8/10 bugs** (80% accuracy)
2. ✅ **Thời gian trung bình < 3 phút/bug**
3. ✅ **Đưa ra suggested fix chính xác**
4. ✅ **Không có false positives**

---

## 🚀 NEXT STEPS

1. **Inject tất cả bugs** vào code
2. **Build với debug symbols**
3. **Run tests** để verify failures
4. **Debug với extension**
5. **Measure AI performance**
6. **Report results**

---

**Ready to break the code!** 🐛💥
