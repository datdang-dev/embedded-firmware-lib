# API Layer Documentation

## Overview

The API layer provides the public interface for HSM operations. It follows the **Facade pattern**, presenting a unified interface to the underlying subsystem components.

**Files:**
- `ihsm_api.hpp` - Public HSM API interface
- `hsm_api_impl.hpp` - API implementation

---

## File: ihsm_api.hpp

### Overview

Defines the abstract interface `IHsmApi` that applications use to interact with the HSM subsystem. This interface-based design enables dependency injection and testing with mock implementations.

### Class: IHsmApi

**Purpose:** Abstract interface for all HSM operations.

**Pattern:** Facade Interface

#### Lifecycle Methods

##### init()

**Purpose:** Initialize the HSM subsystem.

**Signature:**
```cpp
[[nodiscard]] virtual Status init() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Initialization successful |
| `ERR_NOT_INITIALIZED` | Dependencies not ready |
| `ERR_HARDWARE` | Hardware initialization failed |
| `ERR_INTERNAL` | Internal initialization error |

**Thread Safety:** No - Call once during system initialization.

##### deinit()

**Purpose:** Deinitialize the HSM subsystem and release resources.

**Signature:**
```cpp
[[nodiscard]] virtual Status deinit() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Deinitialization successful |
| `ERR_NOT_INITIALIZED` | HSM not initialized |
| `ERR_BUSY` | Active sessions still open |

**Thread Safety:** No - Call once during system shutdown.

##### isReady()

**Purpose:** Check if HSM is initialized and ready for operations.

**Signature:**
```cpp
[[nodiscard]] virtual bool isReady() const = 0;
```

**Return Value:**
- `true` - HSM is initialized and ready
- `false` - HSM is not ready

**Thread Safety:** Yes - Read-only operation.

---

#### Session Management Methods

##### createSession()

**Purpose:** Create a new HSM session for cryptographic operations.

**Signature:**
```cpp
[[nodiscard]] virtual Status createSession(types::SessionId& sessionId) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `sessionId` | [out] | Reference to store the new session ID |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Session created successfully |
| `ERR_SESSION_INVALID` | Cannot create session (limit reached) |
| `ERR_NOT_INITIALIZED` | HSM not initialized |
| `ERR_QUEUE_FULL` | Session queue is full |

**Usage Example:**
```cpp
types::SessionId sessionId;
Status result = hsmApi->createSession(sessionId);
if (result.isOk()) {
    // Use sessionId for subsequent operations
}
```

**Thread Safety:** No - External synchronization required for concurrent calls.

##### closeSession()

**Purpose:** Close an existing HSM session.

**Signature:**
```cpp
[[nodiscard]] virtual Status closeSession(types::SessionId sessionId) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `sessionId` | [in] | Session ID to close |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Session closed successfully |
| `ERR_SESSION_INVALID` | Session ID not found |
| `ERR_NOT_INITIALIZED` | HSM not initialized |

**Usage Example:**
```cpp
Status result = hsmApi->closeSession(sessionId);
if (result.isOk()) {
    // Session closed
}
```

**Thread Safety:** No - External synchronization required.

##### isSessionValid()

**Purpose:** Check if a session ID is valid and active.

**Signature:**
```cpp
[[nodiscard]] virtual bool isSessionValid(types::SessionId sessionId) const = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `sessionId` | [in] | Session ID to validate |

**Return Value:**
- `true` - Session is valid and active
- `false` - Session is invalid or expired

**Thread Safety:** Yes - Read-only operation.

---

#### Cryptographic Operations

##### encrypt()

**Purpose:** Encrypt data using the specified key and algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual Status encrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `sessionId` | [in] | Valid session ID |
| `keySlotId` | [in] | ID of the key slot containing encryption key |
| `algorithm` | [in] | Encryption algorithm to use |
| `input` | [in] | Input plaintext data |
| `output` | [out] | Output ciphertext buffer |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Encryption successful |
| `ERR_SESSION_INVALID` | Invalid session ID |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_INVALID_PARAM` | Invalid algorithm or parameters |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Encryption operation failed |
| `ERR_AUTH_FAILED` | Key permission check failed |

**Usage Example:**
```cpp
std::array<uint8_t, 32> plaintext = { /* data */ };
std::array<uint8_t, 32> ciphertext;

Status result = hsmApi->encrypt(
    sessionId,
    0,  // key slot 0
    types::Algorithm::AES_128,
    plaintext,
    ciphertext);

if (result.isOk()) {
    // Encryption successful
}
```

**Thread Safety:** No - External synchronization required for concurrent operations on same session.

##### decrypt()

**Purpose:** Decrypt data using the specified key and algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual Status decrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `sessionId` | [in] | Valid session ID |
| `keySlotId` | [in] | ID of the key slot containing decryption key |
| `algorithm` | [in] | Decryption algorithm to use |
| `input` | [in] | Input ciphertext data |
| `output` | [out] | Output plaintext buffer |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Decryption successful |
| `ERR_SESSION_INVALID` | Invalid session ID |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_INVALID_PARAM` | Invalid algorithm or parameters |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Decryption operation failed |
| `ERR_AUTH_FAILED` | Key permission check failed |

**Usage Example:**
```cpp
std::array<uint8_t, 32> ciphertext = { /* encrypted data */ };
std::array<uint8_t, 32> plaintext;

Status result = hsmApi->decrypt(
    sessionId,
    0,  // key slot 0
    types::Algorithm::AES_128,
    ciphertext,
    plaintext);

if (result.isOk()) {
    // Decryption successful
}
```

**Thread Safety:** No - External synchronization required.

---

#### Key Management Methods

##### importKey()

**Purpose:** Import a key into a key slot.

**Signature:**
```cpp
[[nodiscard]] virtual Status importKey(
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> keyData,
    uint8_t permissions) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `keySlotId` | [in] | Target key slot ID (0 to kMaxKeySlots-1) |
| `algorithm` | [in] | Algorithm associated with the key |
| `keyData` | [in] | Key data span |
| `permissions` | [in] | Access permission flags |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Key imported successfully |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_KEY_SLOT_FULL` | Key slot already occupied |
| `ERR_INVALID_PARAM` | Invalid algorithm or key size mismatch |
| `ERR_NOT_INITIALIZED` | HSM not initialized |

**Usage Example:**
```cpp
std::array<uint8_t, 16> aesKey = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

uint8_t permissions = types::KeyPermission::ENCRYPT | 
                      types::KeyPermission::DECRYPT;

Status result = hsmApi->importKey(
    0,  // slot 0
    types::Algorithm::AES_128,
    aesKey,
    permissions);

if (result.isOk()) {
    // Key imported successfully
}
```

**Thread Safety:** No - External synchronization required.

##### deleteKey()

**Purpose:** Delete a key from a key slot.

**Signature:**
```cpp
[[nodiscard]] virtual Status deleteKey(uint8_t keySlotId) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `keySlotId` | [in] | Key slot ID to clear |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Key deleted successfully |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_NOT_INITIALIZED` | HSM not initialized |

**Usage Example:**
```cpp
Status result = hsmApi->deleteKey(0);
if (result.isOk()) {
    // Key deleted
}
```

**Thread Safety:** No - External synchronization required.

##### getKeySlotInfo()

**Purpose:** Get information about a key slot.

**Signature:**
```cpp
[[nodiscard]] virtual Status getKeySlotInfo(
    uint8_t keySlotId,
    types::KeySlotInfo& info) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `keySlotId` | [in] | Key slot ID to query |
| `info` | [out] | Reference to store key slot info |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Info retrieved successfully |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_NOT_INITIALIZED` | HSM not initialized |

**Usage Example:**
```cpp
types::KeySlotInfo info;
Status result = hsmApi->getKeySlotInfo(0, info);
if (result.isOk()) {
    if (info.isOccupied) {
        // Key slot is in use
        // info.algorithm, info.keySize, info.permissions available
    }
}
```

**Thread Safety:** Yes - Read-only operation.

##### clearAllKeys()

**Purpose:** Clear all keys from the keystore.

**Signature:**
```cpp
[[nodiscard]] virtual Status clearAllKeys() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | All keys cleared |
| `ERR_NOT_INITIALIZED` | HSM not initialized |

**Usage Example:**
```cpp
Status result = hsmApi->clearAllKeys();
if (result.isOk()) {
    // All keys securely cleared
}
```

**Thread Safety:** No - External synchronization required.

---

#### Utility Methods

##### getVersion()

**Purpose:** Get HSM firmware version string.

**Signature:**
```cpp
[[nodiscard]] virtual const char* getVersion() const = 0;
```

**Return Value:** Static version string (do not free).

**Usage Example:**
```cpp
const char* version = hsmApi->getVersion();
printf("HSM Firmware Version: %s\n", version);
```

**Thread Safety:** Yes - Returns static string.

---

## File: hsm_api_impl.hpp

### Overview

Concrete implementation of the `IHsmApi` interface. This class acts as a facade, delegating work to the middleware and service layers.

### Class: HsmApiImpl

**Purpose:** HSM API implementation using the Facade pattern.

**Pattern:** Facade (concrete implementation)

**Inheritance:** `public IHsmApi`

#### Constructor

```cpp
explicit HsmApiImpl(
    std::shared_ptr<mw::ISessionManager> sessionManager,
    std::shared_ptr<services::ICryptoService> cryptoService,
    std::shared_ptr<services::IKeystoreService> keystoreService);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `sessionManager` | `std::shared_ptr<mw::ISessionManager>` | Session manager for session handling |
| `cryptoService` | `std::shared_ptr<services::ICryptoService>` | Crypto service for encryption/decryption |
| `keystoreService` | `std::shared_ptr<services::IKeystoreService>` | Keystore service for key management |

**Design Note:** Dependencies are injected via constructor, enabling:
- Easy testing with mock implementations
- Flexible configuration
- Loose coupling between layers

#### Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `sessionManager_` | `std::shared_ptr<mw::ISessionManager>` | Session manager dependency |
| `cryptoService_` | `std::shared_ptr<services::ICryptoService>` | Crypto service dependency |
| `keystoreService_` | `std::shared_ptr<services::IKeystoreService>` | Keystore service dependency |
| `isInitialized_` | `bool` | Initialization state flag |

#### Implementation Details

All public methods delegate to the underlying services:

```cpp
Status HsmApiImpl::encrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output) 
{
    // 1. Validate session
    if (!sessionManager_->isSessionValid(sessionId)) {
        return Status(types::StatusCode::ERR_SESSION_INVALID);
    }
    
    // 2. Delegate to crypto service
    return cryptoService_->encrypt(input, output, keySlotId);
}
```

### Usage Example

```cpp
#include "hsm_api_impl.hpp"
#include "session_manager.hpp"
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes128_algorithm.hpp"

using namespace ehsm;

int main() {
    // Create dependencies
    auto sessionManager = std::make_shared<mw::SessionManager>();
    auto keystore = std::make_shared<services::KeystoreService>();
    auto algorithm = std::make_unique<services::crypto::Aes128Algorithm>();
    auto cryptoService = std::make_shared<services::CryptoService>(
        keystore, std::move(algorithm));
    
    // Create HSM API implementation
    auto hsmApi = std::make_unique<api::HsmApiImpl>(
        sessionManager, cryptoService, keystore);
    
    // Initialize
    hsmApi->init();
    
    // Use the API
    types::SessionId sessionId;
    hsmApi->createSession(sessionId);
    
    // ... perform operations ...
    
    // Cleanup
    hsmApi->deinit();
    
    return 0;
}
```

### Thread Safety

**Partial** - The implementation itself does not provide internal synchronization. External synchronization is required for:
- Concurrent session creation/closure
- Concurrent cryptographic operations on the same session
- Concurrent key management operations

### Error Handling

All methods return `Status` objects. Errors propagate from the underlying services:

```
HsmApiImpl
    ├── SessionManager errors
    ├── CryptoService errors
    └── KeystoreService errors
```

### Related Files

| File | Relationship |
|------|--------------|
| [ihsm_api.hpp](#file-ihsm_apihpp) | Interface implemented by this class |
| [services.md](services.md) | Service layer dependencies |
| [middleware.md](middleware.md) | Middleware dependencies |

---

## Complete Usage Example

```cpp
#include "common.hpp"
#include "ihsm_api.hpp"
#include "hsm_api_impl.hpp"
#include "session_manager.hpp"
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes128_algorithm.hpp"

using namespace ehsm;

class HsmExample {
public:
    Status run() {
        // 1. Setup dependencies
        auto sessionManager = std::make_shared<mw::SessionManager>();
        auto keystore = std::make_shared<services::KeystoreService>();
        auto algorithm = std::make_unique<services::crypto::Aes128Algorithm>();
        auto cryptoService = std::make_shared<services::CryptoService>(
            keystore, std::move(algorithm));
        
        // 2. Create API
        auto hsmApi = std::make_unique<api::HsmApiImpl>(
            sessionManager, cryptoService, keystore);
        
        // 3. Initialize
        Status status = hsmApi->init();
        if (!status.isOk()) {
            return status;
        }
        
        // 4. Create session
        types::SessionId sessionId;
        status = hsmApi->createSession(sessionId);
        if (!status.isOk()) {
            return status;
        }
        
        // 5. Import key
        std::array<uint8_t, 16> keyData = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
        };
        status = hsmApi->importKey(
            0,
            types::Algorithm::AES_128,
            keyData,
            types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
        if (!status.isOk()) {
            return status;
        }
        
        // 6. Encrypt data
        std::array<uint8_t, 16> plaintext = {
            'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
            'o', 'r', 'l', 'd', '!', '!', '!', '!'
        };
        std::array<uint8_t, 16> ciphertext;
        status = hsmApi->encrypt(
            sessionId, 0, types::Algorithm::AES_128,
            plaintext, ciphertext);
        if (!status.isOk()) {
            return status;
        }
        
        // 7. Decrypt data
        std::array<uint8_t, 16> decrypted;
        status = hsmApi->decrypt(
            sessionId, 0, types::Algorithm::AES_128,
            ciphertext, decrypted);
        if (!status.isOk()) {
            return status;
        }
        
        // 8. Cleanup
        hsmApi->deleteKey(0);
        hsmApi->closeSession(sessionId);
        hsmApi->deinit();
        
        return Status(types::StatusCode::OK);
    }
};
```
