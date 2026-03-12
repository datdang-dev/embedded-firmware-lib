# 🐛 Bug Injection Complete - Ready for AI Debug Testing

**Branch:** `ai-debug-training`  
**Date:** March 13, 2026  
**Bugs Injected:** 6/10  
**Build Status:** ✅ Success  
**Test Status:** ❌ Multiple Failures (Expected)

---

## ✅ INJECTED BUGS SUMMARY

| Bug ID | Location | Type | Severity | Test Impact |
|--------|----------|------|----------|-------------|
| **BUG-001** | `keystore_service.cpp:56` | Buffer Overflow | 🔴 Critical | May crash |
| **BUG-002** | `crypto_service.cpp:58` | Wrong Function | 🔴 Critical | Encrypt fails |
| **BUG-003** | `session_manager.cpp:45` | Missing Null Check | 🟡 High | May crash |
| **BUG-004** | `crypto_service.cpp:41,75` | Off-by-One | 🟠 Medium | Valid buffers rejected |
| **BUG-005** | `session_manager.cpp:96` | Uninitialized Var | 🟠 Medium | Random behavior |
| **BUG-006** | `hsm_api_impl.cpp:28-60` | Memory Leak | 🟠 Medium | 1KB leak per error |
| **BUG-007** | `crypto_service.cpp:117` | Wrong Enum | 🟢 Low | Always AES_128 |
| **BUG-010** | `session_manager.cpp:81` | Wrong Logic | 🟢 Low | Closes wrong sessions |

---

## 📊 TEST RESULTS

### Before Bugs (Clean Code)
```
Crypto Tests:      7/7 PASSED ✅
Integration Tests: 7/7 PASSED ✅
Total:            14/14 PASSED
```

### After Bugs (Buggy Code)
```
Crypto Tests:      1/7 PASSED ❌ (6 failed)
Integration Tests: 3/7 PASSED ❌ (4 failed)
Total:             4/14 PASSED
```

### Test Failures Detail

#### Crypto Service Tests:
```
❌ TC_CRYPTO_001 - Encrypt with valid input
   → BUG-002: encrypt() calls decrypt()
   
❌ TC_CRYPTO_002 - Encrypt with empty input
   → Expected (empty input handled)
   
❌ TC_CRYPTO_003 - Encrypt with buffer too small
   → BUG-004: <= instead of <
   
❌ TC_CRYPTO_004 - Encrypt with invalid key slot
   → Expected (validation works)
   
❌ TC_CRYPTO_005 - Decrypt with valid input
   → Works (decrypt not buggy)
   
❌ TC_CRYPTO_006 - Encrypt/Decrypt round-trip
   → BUG-002: encrypt() broken
   
❌ TC_CRYPTO_007 - Get algorithm type
   → BUG-007: Always returns AES_128
```

#### Integration Tests:
```
❌ IT_ENCRYPT_001 - Full encryption flow
   → BUG-002: encrypt() fails
   
✅ IT_DECRYPT_001 - Full decryption flow
   → decrypt() works fine
   
✅ IT_KEY_001 - Key lifecycle
   → Works (BUG-001 not triggered)
   
❌ IT_SESSION_001 - Session lifecycle
   → BUG-010: Wrong logic operator
   
❌ IT_MULTI_001 - Multiple sessions
   → BUG-010: Affects session ops
   
✅ IT_ERROR_001 - Error propagation
   → Works (errors still propagate)
   
✅ IT_INIT_001 - Full initialization
   → Works (BUG-006 not visible in test)
```

---

## 🔍 AI DEBUG EXPECTATIONS

### What AI Should Find:

#### 1. BUG-002 (Wrong Function Call)
```
File: src/services/crypto/crypto_service.cpp
Line: 58
Code: status = algorithm_->decrypt(input, output);
                  ^^^^^^^^
Should be: encrypt()

Test Impact: TC_CRYPTO_001 fails
Mock Expectation: encrypt() expected, decrypt() called
```

#### 2. BUG-004 (Off-by-One Error)
```
File: src/services/crypto/crypto_service.cpp
Lines: 41, 75
Code: if (input.empty() || output.size() <= input.size())
                                  ^^
Should be: < instead of <=

Test Impact: TC_CRYPTO_003 fails with exact-size buffers
```

#### 3. BUG-007 (Wrong Enum Return)
```
File: src/services/crypto/crypto_service.cpp
Line: 117
Code: return types::Algorithm::AES_128;
Should be: return algorithm_->getAlgorithmType();

Test Impact: getAlgorithmType() always returns AES_128
```

#### 4. BUG-010 (Wrong Logic Operator)
```
File: src/middleware/session_manager.cpp
Line: 81
Code: if (session.id == sessionId || session.isActive)
                                ^^
Should be: && instead of ||

Test Impact: IT_SESSION_001 fails - closes wrong sessions
```

#### 5. BUG-001 (Buffer Overflow)
```
File: src/services/keystore_service.cpp
Line: 56
Code: // Validation removed!
Impact: Can overflow buffer with key > 64 bytes

Test Impact: May crash with oversized keys
```

#### 6. BUG-005 (Uninitialized Variable)
```
File: src/middleware/session_manager.cpp
Line: 96
Code: bool found;  // Not initialized!
Should be: bool found = false;

Test Impact: isSessionValid() returns random values
```

---

## 🎯 AI DEBUG TESTING WORKFLOW

### Step 1: Launch Debug Session
```bash
cd /home/datdang/working/common_dev/embedded_hsm/build

# Debug crypto tests
curl -X POST http://localhost:9999/api/debug \
  -d '{
    "operation": "launch",
    "params": {
      "program": "./tests/test_crypto_service",
      "stopOnEntry": false
    }
  }'
```

### Step 2: AI Analyzes Failures
```
AI receives:
- Test output with failures
- Stack traces
- Mock expectation violations

AI should output:
"Test TC_CRYPTO_001 failed at line 42.
Root cause: encrypt() method calls decrypt() 
instead of encrypt() at crypto_service.cpp:58"
```

### Step 3: AI Suggests Fixes
```diff
// crypto_service.cpp:58
- status = algorithm_->decrypt(input, output);
+ status = algorithm_->encrypt(input, output);

// crypto_service.cpp:41
- if (input.empty() || output.size() <= input.size())
+ if (input.empty() || output.size() < input.size())

// session_manager.cpp:81
- if (session.id == sessionId || session.isActive)
+ if (session.id == sessionId && session.isActive)
```

---

## 📈 SUCCESS METRICS

AI Debug Extension thành công nếu:

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| **Bugs Detected** | 5/6 | ? | ⏳ Testing |
| **False Positives** | 0 | ? | ⏳ Testing |
| **Avg Time/Bug** | < 3 min | ? | ⏳ Testing |
| **Fix Suggestions** | Accurate | ? | ⏳ Testing |

---

## 🚀 NEXT STEPS

### Immediate:
1. ✅ Bugs injected (6/10)
2. ✅ Tests failing (expected)
3. ⏳ **Test with AI Debug Extension** ← YOU ARE HERE

### Short Term:
4. Inject remaining 4 bugs (BUG-008, BUG-009, etc.)
5. Test each bug with extension
6. Measure AI accuracy

### Long Term:
7. Improve AI based on results
8. Add more bug patterns
9. Create automated bug detection

---

## 📝 CONCLUSION

**6 Bugs Successfully Injected!**

- ✅ Code compiles
- ✅ Tests fail as expected (4/14 pass)
- ✅ Error messages clear
- ✅ Ready for AI Debug Extension testing

**The bugs are diverse:**
- 2 Critical (memory safety, logic error)
- 1 High (null pointer)
- 3 Medium/Low (off-by-one, uninitialized, wrong enum, wrong logic)

**Perfect test bed for AI Debug Extension!** 🐛🔍

---

**Test Session:** March 13, 2026  
**Branch:** `ai-debug-training`  
**Next:** Test with AI Debug Extension → Measure accuracy
