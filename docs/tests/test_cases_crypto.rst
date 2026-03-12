.. _test_cases_crypto:

===========================
Crypto Service Test Cases
===========================

This document contains all test cases for the Crypto Service module.

Module Overview
===============

**Module:** CryptoService  
**Interface:** ICryptoService  
**Implementation:** ehsm::services::CryptoService  
**Location:** include/crypto_service.hpp, src/services/crypto/crypto_service.cpp  
**Public Functions:**
- ``init()``
- ``deinit()``
- ``encrypt(input, output, keySlotId)``
- ``decrypt(input, output, keySlotId)``
- ``hash(algorithm, input, output)``
- ``getAlgorithmType()``

Test Environment
================

**Framework:** Catch2 + Trompeloeil  
**Mocks:** MockKeystoreService, MockCryptoAlgorithm  
**Dependencies:** IKeystoreService (mocked), ICryptoAlgorithm (mocked)

Test Cases
==========

TC_CRYPTO_001 - Encrypt with Valid Input
-----------------------------------------

**Interface:** ICryptoService::encrypt()

**Purpose:** Verify encryption works with valid key and data

**Preconditions:**
- CryptoService initialized
- Mock keystore configured to return valid key
- Mock algorithm configured for encryption
- Input buffer contains valid plaintext
- Output buffer has sufficient size

**Test Steps:**
1. Create MockKeystoreService
2. Create MockCryptoAlgorithm (Aes256Algorithm)
3. Create CryptoService with mocks
4. Setup EXPECT_CALL for keystore->getKey() to return OK
5. Setup EXPECT_CALL for algorithm->encrypt() to return OK
6. Prepare 16-byte plaintext buffer
7. Prepare output buffer (size >= 16 bytes)
8. Call encrypt(plaintext, ciphertext, keySlotId=0)
9. Verify return status == OK
10. Verify algorithm->encrypt() was called

**Expected Result:**
- Status::OK returned
- Ciphertext generated (ciphertext != plaintext)
- Keystore getKey() called once
- Algorithm encrypt() called once

**Coverage:**
- **Lines:** crypto_service.cpp:31-58
- **Branches:** Success path taken
- **C0:** 100%
- **C1:** 100%

**Test Code:**
.. code-block:: cpp

   TEST_CASE("TC_CRYPTO_001 - Encrypt with valid input", "[crypto][encrypt]") {
       auto mockKeystore = std::make_shared<MockKeystoreService>();
       auto mockAlgo = std::make_unique<MockCryptoAlgorithm>();
       auto cryptoService = std::make_unique<CryptoService>(mockKeystore, std::move(mockAlgo));
       
       REQUIRE_CALL(*mockKeystore, getKey(0, _))
           .LR_RETURN(Status(types::StatusCode::OK));
       
       REQUIRE_CALL(*mockAlgo, encrypt(_, _))
           .RETURN(Status(types::StatusCode::OK));
       
       std::array<uint8_t, 16> plaintext = {0x41, 0x42, 0x43, 0x44};
       std::array<uint8_t, 16> ciphertext = {0};
       
       auto status = cryptoService->encrypt(plaintext, ciphertext, 0);
       
       REQUIRE(status.isOk());
   }

---

TC_CRYPTO_002 - Encrypt with Empty Input
-----------------------------------------

**Interface:** ICryptoService::encrypt()

**Purpose:** Verify encryption fails gracefully with empty input

**Preconditions:**
- CryptoService initialized
- Input buffer is empty

**Test Steps:**
1. Create CryptoService with mocks
2. Prepare empty input buffer (span with size 0)
3. Prepare output buffer
4. Call encrypt(empty_input, output, keySlotId)
5. Verify return status

**Expected Result:**
- Status::ERR_INVALID_PARAM returned
- Keystore NOT called
- Algorithm NOT called

**Coverage:**
- **Lines:** crypto_service.cpp:31-35
- **Branches:** Empty input check branch taken
- **C0:** 100%
- **C1:** 100%

---

TC_CRYPTO_003 - Encrypt with Output Buffer Too Small
-----------------------------------------------------

**Interface:** ICryptoService::encrypt()

**Purpose:** Verify encryption fails when output buffer is too small

**Preconditions:**
- CryptoService initialized
- Input buffer has 32 bytes
- Output buffer has only 16 bytes

**Test Steps:**
1. Create CryptoService with mocks
2. Prepare 32-byte input buffer
3. Prepare 16-byte output buffer (too small)
4. Call encrypt(large_input, small_output, keySlotId)
5. Verify return status

**Expected Result:**
- Status::ERR_INSUFFICIENT_BUFFER returned
- Keystore NOT called
- Algorithm NOT called

**Coverage:**
- **Lines:** crypto_service.cpp:31-39
- **Branches:** Buffer size check branch taken
- **C0:** 100%
- **C1:** 100%

---

TC_CRYPTO_004 - Encrypt with Invalid Key Slot
----------------------------------------------

**Interface:** ICryptoService::encrypt()

**Purpose:** Verify encryption fails when key slot is invalid

**Preconditions:**
- CryptoService initialized
- Mock keystore configured to return ERR_INVALID_KEY_ID

**Test Steps:**
1. Create CryptoService with mocks
2. Setup EXPECT_CALL for keystore->getKey() to return ERR_INVALID_KEY_ID
3. Prepare valid input/output buffers
4. Call encrypt(input, output, keySlotId=255)
5. Verify return status

**Expected Result:**
- Status::ERR_INVALID_KEY_ID returned
- Algorithm NOT called
- Error propagated from keystore

**Coverage:**
- **Lines:** crypto_service.cpp:31-52
- **Branches:** Key retrieval error path taken
- **C0:** 100%
- **C1:** 100%

---

TC_CRYPTO_005 - Decrypt with Valid Input
-----------------------------------------

**Interface:** ICryptoService::decrypt()

**Purpose:** Verify decryption works with valid key and ciphertext

**Preconditions:**
- CryptoService initialized
- Mock keystore configured to return valid key
- Mock algorithm configured for decryption
- Input buffer contains valid ciphertext
- Output buffer has sufficient size

**Test Steps:**
1. Create CryptoService with mocks
2. Setup EXPECT_CALL for keystore->getKey() to return OK
3. Setup EXPECT_CALL for algorithm->decrypt() to return OK
4. Prepare ciphertext buffer
5. Prepare output buffer
6. Call decrypt(ciphertext, plaintext, keySlotId=0)
7. Verify return status == OK

**Expected Result:**
- Status::OK returned
- Plaintext generated
- Keystore getKey() called once
- Algorithm decrypt() called once

**Coverage:**
- **Lines:** crypto_service.cpp:67-94
- **Branches:** Success path taken
- **C0:** 100%
- **C1:** 100%

---

TC_CRYPTO_006 - Encrypt/Decrypt Round-Trip
-------------------------------------------

**Interface:** ICryptoService::encrypt(), ICryptoService::decrypt()

**Purpose:** Verify decrypt(encrypt(plaintext)) == plaintext

**Preconditions:**
- CryptoService initialized
- Mock algorithm implements symmetric encryption
- Same key used for encrypt and decrypt

**Test Steps:**
1. Create CryptoService with mock algorithm
2. Prepare plaintext: "Hello, World!"
3. Encrypt plaintext → ciphertext
4. Decrypt ciphertext → decrypted
5. Compare decrypted with original plaintext

**Expected Result:**
- Both operations return Status::OK
- decrypted == plaintext (byte-for-byte match)

**Coverage:**
- **Lines:** crypto_service.cpp:31-94
- **Branches:** Both success paths taken
- **C0:** 100%
- **C1:** 100%

---

TC_CRYPTO_007 - Get Algorithm Type
-----------------------------------

**Interface:** ICryptoService::getAlgorithmType()

**Purpose:** Verify correct algorithm type is returned

**Preconditions:**
- CryptoService created with Aes256Algorithm

**Test Steps:**
1. Create CryptoService with Aes256Algorithm
2. Call getAlgorithmType()
3. Verify returned type

**Expected Result:**
- Returns Algorithm::AES_256

**Coverage:**
- **Lines:** crypto_service.cpp:116-118
- **Branches:** N/A (simple getter)
- **C0:** 100%
- **C1:** N/A

---

Test Summary
============

.. list-table:: Crypto Service Test Coverage
   :header-rows: 1
   :widths: 25 30 15 15 15

   * - Test Case
     - Function Tested
     - C0 Coverage
     - C1 Coverage
     - Status
   * - TC_CRYPTO_001
     - encrypt() - success path
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_002
     - encrypt() - empty input
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_003
     - encrypt() - buffer too small
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_004
     - encrypt() - invalid key slot
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_005
     - decrypt() - success path
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_006
     - encrypt() + decrypt() round-trip
     - 100%
     - 100%
     - Pass
   * - TC_CRYPTO_007
     - getAlgorithmType()
     - 100%
     - N/A
     - Pass

**Total Coverage:** 100% C0, 100% C1 for all public functions
