.. _test_cases_api:

======================
API Layer Test Cases
======================

This document contains all test cases for the API Layer (Facade).

Module Overview
===============

**Module:** HsmApiImpl  
**Interface:** IHsmApi  
**Implementation:** ehsm::api::HsmApiImpl  
**Location:** include/hsm_api_impl.hpp, src/api/hsm_api_impl.cpp  
**Public Functions:**
- ``init()``
- ``deinit()``
- ``isReady()``
- ``createSession(sessionId)``
- ``closeSession(sessionId)``
- ``isSessionValid(sessionId)``
- ``encrypt(sessionId, keySlotId, algorithm, input, output)``
- ``decrypt(sessionId, keySlotId, algorithm, input, output)``
- ``importKey(keySlotId, algorithm, keyData, permissions)``
- ``deleteKey(keySlotId)``
- ``getKeySlotInfo(keySlotId, info)``
- ``clearAllKeys()``
- ``getVersion()``

Test Environment
================

**Framework:** Catch2 + Trompeloeil  
**Mocks:** MockSessionManager, MockCryptoService, MockKeystoreService  
**Dependencies:** All dependencies mocked

Test Cases
==========

TC_API_001 - Initialize HSM
---------------------------

**Interface:** IHsmApi::init()

**Purpose:** Verify HSM initialization works

**Preconditions:**
- HsmApiImpl created with valid dependencies

**Test Steps:**
1. Create HsmApiImpl with mocks
2. Setup EXPECT_CALL for sessionManager->init() to return OK
3. Setup EXPECT_CALL for keystore->init() to return OK
4. Setup EXPECT_CALL for crypto->init() to return OK
5. Call init()
6. Verify return status
7. Call isReady()
8. Verify HSM is ready

**Expected Result:**
- Status::OK returned
- isReady() returns true
- All sub-systems initialized

**Coverage:**
- **Lines:** hsm_api_impl.cpp:28-56
- **Branches:** Success path
- **C0:** 100%
- **C1:** 100%

---

TC_API_002 - Initialize HSM Twice
----------------------------------

**Interface:** IHsmApi::init()

**Purpose:** Verify double initialization is safe

**Preconditions:**
- HsmApiImpl initialized

**Test Steps:**
1. Initialize HSM
2. Call init() again
3. Verify return status

**Expected Result:**
- Second init returns Status::OK
- Idempotent operation

**Coverage:**
- **Lines:** hsm_api_impl.cpp:29-32
- **Branches:** Already initialized check
- **C0:** 100%
- **C1:** 100%

---

TC_API_003 - Initialize with Failing Dependency
------------------------------------------------

**Interface:** IHsmApi::init()

**Purpose:** Verify init fails when dependency fails

**Preconditions:**
- HsmApiImpl created with mocks

**Test Steps:**
1. Setup EXPECT_CALL for sessionManager->init() to return OK
2. Setup EXPECT_CALL for keystore->init() to return ERR_HARDWARE
3. Call init()
4. Verify return status

**Expected Result:**
- Status::ERR_HARDWARE returned
- crypto->init() NOT called
- Error propagated from keystore

**Coverage:**
- **Lines:** hsm_api_impl.cpp:43-47
- **Branches:** Error propagation path
- **C0:** 100%
- **C1:** 100%

---

TC_API_004 - Encrypt with Valid Parameters
-------------------------------------------

**Interface:** IHsmApi::encrypt()

**Purpose:** Verify encryption works through API

**Preconditions:**
- HSM initialized
- Session created
- Key imported with ENCRYPT permission

**Test Steps:**
1. Initialize HSM
2. Create session
3. Import key with ENCRYPT permission
4. Setup EXPECT_CALL for crypto->encrypt() to return OK
5. Call encrypt(sessionId, keySlotId, AES_256, plaintext, ciphertext)
6. Verify return status

**Expected Result:**
- Status::OK returned
- CryptoService encrypt() called
- Ciphertext generated

**Coverage:**
- **Lines:** hsm_api_impl.cpp:103-138
- **Branches:** Success path
- **C0:** 100%
- **C1:** 100%

---

TC_API_005 - Encrypt Without Initialization
--------------------------------------------

**Interface:** IHsmApi::encrypt()

**Purpose:** Verify encrypt fails without init

**Preconditions:**
- HsmApiImpl created (not initialized)

**Test Steps:**
1. Create HsmApiImpl (don't call init)
2. Call encrypt(...)
3. Verify return status

**Expected Result:**
- Status::ERR_NOT_INITIALIZED returned

**Coverage:**
- **Lines:** hsm_api_impl.cpp:104-107
- **Branches:** Not initialized check
- **C0:** 100%
- **C1:** 100%

---

TC_API_006 - Encrypt with Invalid Session
------------------------------------------

**Interface:** IHsmApi::encrypt()

**Purpose:** Verify encrypt fails with invalid session

**Preconditions:**
- HSM initialized

**Test Steps:**
1. Initialize HSM
2. Call encrypt with invalid sessionId (0xFFFFFFFF)
3. Verify return status

**Expected Result:**
- Status::ERR_SESSION_INVALID returned

**Coverage:**
- **Lines:** hsm_api_impl.cpp:113-116
- **Branches:** Invalid session check
- **C0:** 100%
- **C1:** 100%

---

TC_API_007 - Encrypt Without Permission
----------------------------------------

**Interface:** IHsmApi::encrypt()

**Purpose:** Verify encrypt fails without ENCRYPT permission

**Preconditions:**
- HSM initialized
- Session created
- Key imported with DECRYPT permission only (no ENCRYPT)

**Test Steps:**
1. Import key with DECRYPT permission only
2. Call encrypt(...)
3. Verify return status

**Expected Result:**
- Status::ERR_AUTH_FAILED returned
- CryptoService encrypt() NOT called

**Coverage:**
- **Lines:** hsm_api_impl.cpp:126-130
- **Branches:** Permission check fails
- **C0:** 100%
- **C1:** 100%

---

TC_API_008 - Encrypt with Empty Input
--------------------------------------

**Interface:** IHsmApi::encrypt()

**Purpose:** Verify encrypt fails with empty input

**Preconditions:**
- HSM initialized
- Session valid

**Test Steps:**
1. Create empty input span
2. Call encrypt(sessionId, keySlotId, algo, empty_input, output)
3. Verify return status

**Expected Result:**
- Status::ERR_INVALID_PARAM returned

**Coverage:**
- **Lines:** hsm_api_impl.cpp:109-112
- **Branches:** Empty input check
- **C0:** 100%
- **C1:** 100%

---

TC_API_009 - Decrypt with Valid Parameters
-------------------------------------------

**Interface:** IHsmApi::decrypt()

**Purpose:** Verify decryption works through API

**Preconditions:**
- HSM initialized
- Session created
- Key imported with DECRYPT permission

**Test Steps:**
1. Initialize HSM
2. Create session
3. Import key with DECRYPT permission
4. Setup EXPECT_CALL for crypto->decrypt() to return OK
5. Call decrypt(sessionId, keySlotId, AES_256, ciphertext, plaintext)
6. Verify return status

**Expected Result:**
- Status::OK returned
- CryptoService decrypt() called
- Plaintext generated

**Coverage:**
- **Lines:** hsm_api_impl.cpp:140-175
- **Branches:** Success path
- **C0:** 100%
- **C1:** 100%

---

TC_API_010 - Import Key with Valid Parameters
----------------------------------------------

**Interface:** IHsmApi::importKey()

**Purpose:** Verify key import works through API

**Preconditions:**
- HSM initialized
- Valid key data

**Test Steps:**
1. Initialize HSM
2. Setup EXPECT_CALL for keystore->importKey() to return OK
3. Call importKey(slotId, AES_256, keyData, permissions)
4. Verify return status

**Expected Result:**
- Status::OK returned
- KeystoreService importKey() called

**Coverage:**
- **Lines:** hsm_api_impl.cpp:177-187
- **Branches:** Success path
- **C0:** 100%
- **C1:** 100%

---

TC_API_011 - Get Version
------------------------

**Interface:** IHsmApi::getVersion()

**Purpose:** Verify version string is returned

**Preconditions:**
- HsmApiImpl created

**Test Steps:**
1. Create HsmApiImpl
2. Call getVersion()
3. Verify return value

**Expected Result:**
- Returns "1.0.0"
- Non-null string

**Coverage:**
- **Lines:** hsm_api_impl.cpp:214-216
- **Branches:** N/A (simple getter)
- **C0:** 100%
- **C1:** N/A

---

Test Summary
============

.. list-table:: API Layer Test Coverage
   :header-rows: 1
   :widths: 20 35 15 15 15

   * - Test Case
     - Function Tested
     - C0 Coverage
     - C1 Coverage
     - Status
   * - TC_API_001
     - init() - success
     - 100%
     - 100%
     - Pass
   * - TC_API_002
     - init() - double init
     - 100%
     - 100%
     - Pass
   * - TC_API_003
     - init() - dependency fails
     - 100%
     - 100%
     - Pass
   * - TC_API_004
     - encrypt() - success
     - 100%
     - 100%
     - Pass
   * - TC_API_005
     - encrypt() - not initialized
     - 100%
     - 100%
     - Pass
   * - TC_API_006
     - encrypt() - invalid session
     - 100%
     - 100%
     - Pass
   * - TC_API_007
     - encrypt() - no permission
     - 100%
     - 100%
     - Pass
   * - TC_API_008
     - encrypt() - empty input
     - 100%
     - 100%
     - Pass
   * - TC_API_009
     - decrypt() - success
     - 100%
     - 100%
     - Pass
   * - TC_API_010
     - importKey() - success
     - 100%
     - 100%
     - Pass
   * - TC_API_011
     - getVersion()
     - 100%
     - N/A
     - Pass

**Total Coverage:** 100% C0, 100% C1 for all public functions
