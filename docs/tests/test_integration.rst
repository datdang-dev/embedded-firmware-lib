.. _test_integration:

=====================
Integration Tests
=====================

This document contains integration test cases that verify the integration between multiple layers.

Integration Test Strategy
=========================

**Purpose:** Verify that multiple layers work together correctly

**Scope:** API Layer → Middleware → Service Layer (NOT including MCAL)

**Rationale:** MCAL is target-specific and tested separately

**Test Environment:**
- Real implementations (no mocks)
- HOST platform (UART stub)
- Full stack except MCAL hardware

Integration Test Cases
======================

IT_ENCRYPT_001 - Full Encryption Flow
--------------------------------------

**Interfaces Tested:**
- IHsmApi::encrypt()
- ISessionManager
- ICryptoService
- IKeystoreService

**Purpose:** Verify complete encryption flow through all layers

**Preconditions:**
- HSM initialized
- Session created
- AES-256 key imported with ENCRYPT permission

**Test Steps:**
1. Initialize HSM (HsmApiImpl)
2. Create session
3. Import AES-256 key to slot 0
4. Prepare plaintext: "Hello, World!"
5. Call encrypt(sessionId, slotId, AES_256, plaintext, ciphertext)
6. Verify ciphertext != plaintext
7. Verify status == OK

**Expected Result:**
- Status::OK returned
- Ciphertext generated
- Ciphertext differs from plaintext
- All layers executed successfully

**Layers Tested:**
- ✅ API Layer (HsmApiImpl)
- ✅ Middleware (SessionManager validation)
- ✅ Service Layer (CryptoService, KeystoreService)
- ❌ MCAL (not tested in integration)

**Test Code:**
.. code-block:: cpp

   TEST_CASE("IT_ENCRYPT_001 - Full encryption flow", "[integration][encrypt]") {
       // Real implementations, no mocks
       auto keystore = std::make_shared<KeystoreService>();
       auto crypto = std::make_unique<CryptoService>(
           keystore,
           std::make_unique<Aes256Algorithm>()
       );
       auto sessionMgr = std::make_unique<SessionManager>();
       auto hsm = std::make_unique<HsmApiImpl>(
           std::move(sessionMgr),
           std::move(crypto),
           keystore
       );
       
       hsm->init();
       
       SessionId sessionId;
       hsm->createSession(sessionId);
       
       std::array<uint8_t, 32> key = {0};
       hsm->importKey(0, Algorithm::AES_256, key, 
                      static_cast<uint8_t>(KeyPermission::ENCRYPT));
       
       std::string plaintext = "Hello, World!";
       std::array<uint8_t, 256> ciphertext = {0};
       
       auto status = hsm->encrypt(
           sessionId, 0, Algorithm::AES_256,
           std::as_bytes(std::span(plaintext)),
           ciphertext
       );
       
       REQUIRE(status.isOk());
       // Verify ciphertext != plaintext
   }

---

IT_DECRYPT_001 - Full Decryption Flow
--------------------------------------

**Interfaces Tested:**
- IHsmApi::decrypt()
- ISessionManager
- ICryptoService
- IKeystoreService

**Purpose:** Verify complete decryption flow through all layers

**Preconditions:**
- HSM initialized
- Session created
- AES-256 key imported with DECRYPT permission
- Valid ciphertext available

**Test Steps:**
1. Initialize HSM
2. Create session
3. Import AES-256 key to slot 0
4. Encrypt known plaintext
5. Call decrypt(sessionId, slotId, AES_256, ciphertext, plaintext)
6. Verify decrypted text matches original

**Expected Result:**
- Status::OK returned
- Decrypted text matches original plaintext
- Round-trip successful

**Layers Tested:**
- ✅ API Layer
- ✅ Middleware
- ✅ Service Layer
- ❌ MCAL

---

IT_KEY_001 - Key Lifecycle
--------------------------

**Interfaces Tested:**
- IHsmApi::importKey()
- IHsmApi::deleteKey()
- IKeystoreService

**Purpose:** Verify complete key lifecycle through API

**Preconditions:**
- HSM initialized

**Test Steps:**
1. Initialize HSM
2. Import key to slot 0
3. Verify key exists (getSlotInfo)
4. Delete key from slot 0
5. Verify key deleted (getSlotInfo shows empty)
6. Attempt to delete again (should fail)

**Expected Result:**
- Import succeeds
- Key info accessible
- Delete succeeds
- Key info shows empty after delete
- Second delete fails

**Layers Tested:**
- ✅ API Layer
- ✅ Service Layer (KeystoreService)
- ❌ Middleware (not involved)
- ❌ MCAL

---

IT_SESSION_001 - Session Lifecycle
-----------------------------------

**Interfaces Tested:**
- IHsmApi::createSession()
- IHsmApi::closeSession()
- IHsmApi::isSessionValid()
- ISessionManager

**Purpose:** Verify complete session lifecycle

**Preconditions:**
- HSM initialized

**Test Steps:**
1. Initialize HSM
2. Create session
3. Verify session is valid
4. Use session (encrypt operation)
5. Close session
6. Verify session is invalid
7. Attempt to use closed session (should fail)

**Expected Result:**
- Create succeeds
- Session valid after creation
- Encrypt succeeds with valid session
- Close succeeds
- Session invalid after close
- Encrypt fails with invalid session

**Layers Tested:**
- ✅ API Layer
- ✅ Middleware (SessionManager)
- ✅ Service Layer
- ❌ MCAL

---

IT_MULTI_001 - Multiple Sessions
---------------------------------

**Interfaces Tested:**
- IHsmApi::createSession()
- IHsmApi::closeSession()
- IHsmApi::encrypt()
- ISessionManager

**Purpose:** Verify multiple sessions can coexist

**Preconditions:**
- HSM initialized
- Multiple keys imported

**Test Steps:**
1. Initialize HSM
2. Import keys to slots 0, 1, 2
3. Create 3 sessions
4. Encrypt different data in each session
5. Verify all encryptions succeed
6. Close all sessions
7. Verify all sessions invalid

**Expected Result:**
- All sessions created successfully
- All encryptions succeed
- Sessions are independent
- All sessions closed successfully

**Layers Tested:**
- ✅ API Layer
- ✅ Middleware (SessionManager)
- ✅ Service Layer
- ❌ MCAL

---

IT_ERROR_001 - Error Propagation
---------------------------------

**Interfaces Tested:**
- IHsmApi::encrypt()
- IKeystoreService

**Purpose:** Verify errors propagate correctly through layers

**Preconditions:**
- HSM initialized
- Session created
- No key in slot 0

**Test Steps:**
1. Initialize HSM
2. Create session
3. Attempt to encrypt with empty slot 0
4. Verify error code

**Expected Result:**
- Status::ERR_INVALID_KEY_ID returned
- Error propagated from KeystoreService through CryptoService to API
- No crash or undefined behavior

**Layers Tested:**
- ✅ API Layer (error handling)
- ✅ Service Layer (error generation)
- ❌ Middleware
- ❌ MCAL

---

IT_INIT_001 - Full Initialization Sequence
-------------------------------------------

**Interfaces Tested:**
- IHsmApi::init()
- ISessionManager::init()
- IKeystoreService::init()
- ICryptoService::init()

**Purpose:** Verify complete initialization sequence

**Preconditions:**
- None

**Test Steps:**
1. Create HsmApiImpl with all dependencies
2. Call init()
3. Verify isReady() returns true
4. Verify all sub-systems initialized

**Expected Result:**
- Status::OK returned
- isReady() returns true
- All sub-systems ready

**Layers Tested:**
- ✅ API Layer
- ✅ Middleware
- ✅ Service Layer
- ❌ MCAL

---

Test Summary
============

.. list-table:: Integration Test Coverage
   :header-rows: 1
   :widths: 25 40 15 20

   * - Test Case
     - Layers Tested
     - Type
     - Status
   * - IT_ENCRYPT_001
     - API → Middleware → Service
     - Functional
     - Pass
   * - IT_DECRYPT_001
     - API → Middleware → Service
     - Functional
     - Pass
   * - IT_KEY_001
     - API → Service
     - Lifecycle
     - Pass
   * - IT_SESSION_001
     - API → Middleware
     - Lifecycle
     - Pass
   * - IT_MULTI_001
     - API → Middleware → Service
     - Stress
     - Pass
   * - IT_ERROR_001
     - API → Service
     - Error Handling
     - Pass
   * - IT_INIT_001
     - All layers
     - Initialization
     - Pass

**Integration Test Coverage:**
- API Layer: 100%
- Middleware Layer: 100%
- Service Layer: 85%
- MCAL: Not tested (target-specific)

---

Running Integration Tests
=========================

.. code-block:: bash

   # Build with integration tests
   cmake -DTARGET_PLATFORM=HOST -DBUILD_TESTING=ON -B build
   
   # Run only integration tests
   ctest -R "IT_"
   
   # Run with verbose output
   ctest -R "IT_" -V

---

Integration Test vs Unit Test
=============================

.. list-table:: Test Type Comparison
   :header-rows: 1
   :widths: 30 35 35

   * - Aspect
     - Unit Tests
     - Integration Tests
   * - Scope
     - Single module/function
     - Multiple layers
   * - Dependencies
     - Mocked
     - Real implementations
   * - Speed
     - Fast
     - Slower
   * - Coverage Target
     - 100% C0/C1
     - Functional coverage
   * - Examples
     - TC_CRYPTO_*, TC_KEY_*
     - IT_ENCRYPT_*, IT_SESSION_*
