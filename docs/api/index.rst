.. _api_index:

=================
API Documentation
=================

This section contains the API reference documentation for all public interfaces.

Modules
=======

.. toctree::
   :maxdepth: 2

   Core Types <types>
   API Layer <api_layer>
   Services <services>
   Middleware <middleware>
   MCAL <mcal>

Quick Reference
===============

**Initialization:**

.. code-block:: cpp

   #include "ihsm_api.hpp"
   
   auto hsm = std::make_unique<HsmApiImpl>(...);
   hsm->init();

**Encryption:**

.. code-block:: cpp

   hsm->encrypt(sessionId, keySlotId, Algorithm::AES_256,
                plaintext, ciphertext);

**Key Management:**

.. code-block:: cpp

   hsm->importKey(slotId, Algorithm::AES_256, keyData, permissions);
   hsm->deleteKey(slotId);

Common Types
============

.. list-table:: Common Types
   :header-rows: 1
   :widths: 30 70

   * - Type
     - Description
   * - Status
     - Error handling (no exceptions)
   * - Algorithm
     - Encryption algorithm enum
   * - KeyPermission
     - Access permission flags
   * - SessionId
     - Session identifier (uint32_t)
   * - KeySlotInfo
     - Key slot metadata

Error Codes
===========

.. list-table:: Status Codes
   :header-rows: 1
   :widths: 30 70

   * - Code
     - Meaning
   * - Status::OK
     - Success
   * - Status::ERR_INVALID_PARAM
     - Invalid parameter
   * - Status::ERR_INSUFFICIENT_BUFFER
     - Output buffer too small
   * - Status::ERR_CRYPTO_FAILED
     - Crypto operation failed
   * - Status::ERR_SESSION_INVALID
     - Invalid session ID
   * - Status::ERR_KEY_SLOT_FULL
     - Key slot already occupied
   * - Status::ERR_INVALID_KEY_ID
     - Invalid key slot ID
   * - Status::ERR_AUTH_FAILED
     - Permission check failed
   * - Status::ERR_NOT_INITIALIZED
     - HSM not initialized

Thread Safety
=============

.. list-table:: Thread Safety by Module
   :header-rows: 1
   :widths: 30 70

   * - Module
     - Thread Safety
   * - HsmApiImpl
     - Not thread-safe (external sync required)
   * - SessionManager
     - Not thread-safe
   * - CryptoService
     - Not thread-safe
   * - KeystoreService
     - Not thread-safe

**Recommendation:** Use external mutex for multi-threaded access.

Memory Safety
=============

**Key Points:**
- No dynamic allocation in critical paths
- Keys securely cleared after use
- std::span for buffer views (no copies)
- std::array for fixed-size buffers
- volatile for secure clear

Example Usage
=============

Complete example:

.. code-block:: cpp

   #include "ihsm_api.hpp"
   #include "crypto_service.hpp"
   #include "keystore_service.hpp"
   #include "session_manager.hpp"
   
   int main() {
       // Create services
       auto keystore = std::make_shared<KeystoreService>();
       auto crypto = std::make_unique<CryptoService>(
           keystore,
           std::make_unique<Aes256Algorithm>()
       );
       auto sessionMgr = std::make_unique<SessionManager>();
       
       // Create API
       auto hsm = std::make_unique<HsmApiImpl>(
           std::move(sessionMgr),
           std::move(crypto),
           keystore
       );
       
       // Initialize
       hsm->init();
       
       // Create session
       SessionId sessionId;
       hsm->createSession(sessionId);
       
       // Import key
       std::array<uint8_t, 32> keyData = {0};
       hsm->importKey(0, Algorithm::AES_256, keyData,
                      static_cast<uint8_t>(KeyPermission::ENCRYPT));
       
       // Encrypt
       std::string plaintext = "Hello!";
       std::array<uint8_t, 256> ciphertext = {0};
       hsm->encrypt(sessionId, 0, Algorithm::AES_256,
                    std::as_bytes(std::span(plaintext)),
                    ciphertext);
       
       // Decrypt
       std::array<uint8_t, 256> decrypted = {0};
       hsm->decrypt(sessionId, 0, Algorithm::AES_256,
                    ciphertext,
                    decrypted);
       
       // Cleanup
       hsm->deleteKey(0);
       hsm->closeSession(sessionId);
       hsm->deinit();
       
       return 0;
   }

Related Documentation
=====================

* :doc:`../architecture/index` - Architecture overview
* :doc:`../tests/index` - Test documentation
* :doc:`../diagrams/index` - Architecture diagrams
