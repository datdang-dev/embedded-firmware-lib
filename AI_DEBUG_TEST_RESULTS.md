# 🐛 AI Debug Extension - Test Results with Injected Bugs

**Branch:** `ai-debug-training`  
**Date:** March 13, 2026  
**Bugs Injected:** 2 (BUG-002, BUG-004)  
**Tests Run:** 7  
**Tests Failed:** 1 (expected)

---

## ✅ BUG INJECTION STATUS

### Successfully Injected Bugs:

| Bug ID | Location | Type | Status | Test Impact |
|--------|----------|------|--------|-------------|
| **BUG-002** | `crypto_service.cpp:58` | Wrong function call | ✅ Injected | Test fails |
| **BUG-004** | `crypto_service.cpp:41,75` | Off-by-one error | ✅ Injected | May fail |

---

## 📊 TEST RESULTS WITH BUGS

### Before Bug Injection (Clean Code)
```
===============================================================================
All tests passed (12 assertions in 7 test cases)
```

### After Bug Injection (BUG-002, BUG-004)
```
TC_CRYPTO_001 - Encrypt with valid input
...
REQUIRE( status.isOk() ) with expansion: false

Unfulfilled expectation:
Expected *mockAlgoPtr.encrypt(...) to be called once,
actually never called

===============================================================================
test cases: 1 | 1 failed
assertions: 2 | 2 failed
```

---

## 🔍 AI DEBUG ANALYSIS

### What AI Should Find:

#### 1. Test Failure Analysis
```
Test: TC_CRYPTO_001 - Encrypt with valid input
Location: test_crypto_service.cpp:23
Assertion: REQUIRE(status.isOk()) - FAILED
```

#### 2. Mock Expectation Failure
```
File: test_crypto_service.cpp:34
Expected: mockAlgoPtr.encrypt() called once
Actual: encrypt() NEVER called
```

#### 3. Root Cause Trace
```
Flow:
1. Test calls cryptoService->encrypt(...)
2. CryptoService::encrypt() executes
3. Expected: algorithm_->encrypt(input, output)
4. Actual: algorithm_->decrypt(input, output) ← BUG!
5. Mock expects encrypt(), gets decrypt() → FAIL
```

#### 4. Bug Location
```
File: src/services/crypto/crypto_service.cpp
Line: 58
Code: status = algorithm_->decrypt(input, output);
                         ^^^^^^^^
Should be: encrypt()
```

---

## 🎯 AI DEBUG EXTENSION TEST WORKFLOW

### Step 1: Run Test with Extension
```bash
cd /home/datdang/working/common_dev/embedded_hsm/build

# Launch debug session
curl -X POST http://localhost:9999/api/debug \
  -d '{
    "operation": "launch",
    "params": {
      "program": "./tests/test_crypto_service",
      "stopOnEntry": false
    }
  }'
```

### Step 2: AI Analyzes Failure
```
AI receives:
- Test output (failure message)
- Stack trace
- Mock expectation failure

AI should output:
"Test TC_CRYPTO_001 failed because encrypt() method calls
decrypt() instead of encrypt() at line 58 in
crypto_service.cpp"
```

### Step 3: AI Suggests Fix
```diff
- status = algorithm_->decrypt(input, output);
+ status = algorithm_->encrypt(input, output);
```

---

## 📋 REMAINING BUGS TO INJECT

### Not Yet Injected (8 bugs):

| Bug ID | Type | Priority | Status |
|--------|------|----------|--------|
| BUG-001 | Buffer Overflow | 🔴 Critical | Pending |
| BUG-003 | Null Pointer | 🟡 High | Pending |
| BUG-005 | Uninitialized Var | 🟠 Medium | Pending |
| BUG-006 | Memory Leak | 🟠 Medium | Pending |
| BUG-007 | Wrong Enum | 🟢 Low | Pending |
| BUG-008 | Sign Extension | 🟢 Low | Pending |
| BUG-009 | Race Condition | 🟡 High | Pending |
| BUG-010 | Wrong Logic | 🟢 Low | Pending |

---

## 🎯 SUCCESS CRITERIA FOR AI DEBUG

AI Debug Extension thành công nếu:

### ✅ Level 1: Basic Debug (Must Have)
- [ ] Identifies test failure location
- [ ] Shows stack trace
- [ ] Displays variable values

### ✅ Level 2: Root Cause (Should Have)
- [ ] Traces to wrong function call
- [ ] Identifies encrypt() vs decrypt()
- [ ] Shows line 58 as problem

### ✅ Level 3: Suggested Fix (Nice to Have)
- [ ] Suggests changing decrypt() to encrypt()
- [ ] Provides diff
- [ ] Explains why it's wrong

---

## 🚀 NEXT STEPS

### Immediate:
1. ✅ BUG-002 injected and verified
2. ✅ Test fails as expected
3. ⏳ **Test with AI Debug Extension** ← YOU ARE HERE

### Short Term:
4. Inject remaining 8 bugs
5. Test each bug with extension
6. Measure AI accuracy

### Long Term:
7. Improve AI based on results
8. Add more bug patterns
9. Create automated bug detection

---

## 📝 CONCLUSION

**BUG-002 Successfully Injected!**

- ✅ Code compiles
- ✅ Test fails (expected)
- ✅ Error message clear
- ✅ Mock expectation violation helpful

**Ready for AI Debug Extension testing!**

The extension should be able to:
1. Detect the test failure
2. Trace to root cause (wrong function call)
3. Suggest the fix

---

**Test Session:** March 13, 2026  
**Next:** Test with AI Debug Extension → Measure accuracy
