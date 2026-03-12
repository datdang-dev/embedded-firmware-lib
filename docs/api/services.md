# Service Layer Documentation

## Overview

The service layer provides the core business logic for cryptographic operations and key management. It follows the **Strategy pattern** for algorithm flexibility and **Dependency Injection** for loose coupling.

**Files:**
- `icrypto_service.hpp` - Crypto service interface
- `crypto_service.hpp` - Crypto service implementation
- `ikeystore_service.hpp` - Keystore service interface
- `keystore_service.hpp` - Keystore implementation
- `icrypto_algorithm.hpp` - Crypto algorithm strategy interface
- `aes128_algorithm.hpp` - AES-128 implementation
- `aes256_algorithm.hpp` - AES-256 implementation

---

## File: icrypto_service.hpp

### Overview

Abstract interface for cryptographic operations. All crypto service implementations must inherit from this interface, enabling dependency injection and testing with mocks.

### Class: ICryptoService

**Purpose:** Interface for cryptographic operations.

**Pattern:** Strategy Interface

#### Lifecycle Methods

##### init()

**Purpose:** Initialize the crypto service.

**Signature:**
```cpp
[[nodiscard]] virtual Status init() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Initialization successful |
| `ERR_NOT_INITIALIZED` | Dependencies not ready |
| `ERR_HARDWARE` | Hardware crypto engine failed |

##### deinit()

**Purpose:** Deinitialize the crypto service.

**Signature:**
```cpp
[[nodiscard]] virtual Status deinit() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Deinitialization successful |
| `ERR_NOT_INITIALIZED` | Service not initialized |

---

#### Cryptographic Operations

##### encrypt()

**Purpose:** Encrypt data using the configured algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual Status encrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output,
    uint8_t keySlotId) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `input` | [in] | Plaintext input data |
| `output` | [out] | Ciphertext output buffer |
| `keySlotId` | [in] | Key slot containing encryption key |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Encryption successful |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Encryption operation failed |
| `ERR_NOT_INITIALIZED` | Service not initialized |

##### decrypt()

**Purpose:** Decrypt data using the configured algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual Status decrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output,
    uint8_t keySlotId) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `input` | [in] | Ciphertext input data |
| `output` | [out] | Plaintext output buffer |
| `keySlotId` | [in] | Key slot containing decryption key |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Decryption successful |
| `ERR_INVALID_KEY_ID` | Invalid key slot ID |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Decryption operation failed |
| `ERR_NOT_INITIALIZED` | Service not initialized |

##### hash()

**Purpose:** Compute hash of input data.

**Signature:**
```cpp
[[nodiscard]] virtual Status hash(
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `algorithm` | [in] | Hash algorithm to use |
| `input` | [in] | Input data |
| `output` | [out] | Hash digest output |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Hash computation successful |
| `ERR_INVALID_PARAM` | Invalid hash algorithm |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Hash operation failed |

### Usage Example

```cpp
#include "icrypto_service.hpp"

void useCryptoService(std::shared_ptr<services::ICryptoService> cryptoService) {
    cryptoService->init();
    
    std::array<uint8_t, 16> plaintext = { /* data */ };
    std::array<uint8_t, 16> ciphertext;
    
    Status status = cryptoService->encrypt(plaintext, ciphertext, 0);
    if (status.isOk()) {
        // Encryption successful
    }
    
    cryptoService->deinit();
}
```

---

## File: crypto_service.hpp

### Overview

Concrete implementation of `ICryptoService` that uses the Strategy pattern to support multiple encryption algorithms.

### Class: CryptoService

**Purpose:** Implements symmetric encryption operations.

**Pattern:** Strategy (Context)

**Inheritance:** `public ICryptoService`

#### Constructor

```cpp
explicit CryptoService(
    std::shared_ptr<IKeystoreService> keystore,
    std::unique_ptr<ICryptoAlgorithm> algorithm);
```

**Parameters:**
| Parameter | Type | Description |
|-----------|------|-------------|
| `keystore` | `std::shared_ptr<IKeystoreService>` | Keystore for key retrieval |
| `algorithm` | `std::unique_ptr<ICryptoAlgorithm>` | Crypto algorithm strategy |

**Design Notes:**
- Takes ownership of the algorithm via `unique_ptr`
- Shares keystore via `shared_ptr`
- Dependencies are injected, not created internally

#### Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `keystore_` | `std::shared_ptr<IKeystoreService>` | Keystore dependency |
| `algorithm_` | `std::unique_ptr<ICryptoAlgorithm>` | Algorithm strategy |

#### Implementation Details

##### encrypt()

```cpp
Status CryptoService::encrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output,
    uint8_t keySlotId)
{
    // 1. Get key from keystore
    types::Key key;
    Status status = keystore_->getKey(keySlotId, key);
    if (!status.isOk()) {
        return status;
    }
    
    // 2. Validate key size
    if (key.size != algorithm_->getKeySize()) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }
    
    // 3. Delegate to algorithm
    return algorithm_->encrypt(input, output);
}
```

##### getAlgorithmType()

**Purpose:** Get the current algorithm type.

**Signature:**
```cpp
[[nodiscard]] types::Algorithm getAlgorithmType() const;
```

**Return Value:** Current algorithm type (delegates to `algorithm_`).

### Usage Example

```cpp
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes128_algorithm.hpp"

using namespace ehsm::services;

// Create keystore
auto keystore = std::make_shared<KeystoreService>();
keystore->init();

// Import key
std::array<uint8_t, 16> keyData = { /* 128-bit key */ };
keystore->importKey(0, types::Algorithm::AES_128, keyData, 
                    types::KeyPermission::ENCRYPT);

// Create crypto service with AES-128
auto algorithm = std::make_unique<crypto::Aes128Algorithm>();
auto cryptoService = std::make_shared<CryptoService>(keystore, std::move(algorithm));
cryptoService->init();

// Encrypt
std::array<uint8_t, 16> plaintext = { /* data */ };
std::array<uint8_t, 16> ciphertext;
Status status = cryptoService->encrypt(plaintext, ciphertext, 0);

// Check algorithm type
types::Algorithm algo = cryptoService->getAlgorithmType();
// algo == types::Algorithm::AES_128

cryptoService->deinit();
```

### Thread Safety

**No** - The service is not thread-safe. External synchronization is required for concurrent access.

---

## File: ikeystore_service.hpp

### Overview

Interface for secure key storage and retrieval operations.

### Class: IKeystoreService

**Purpose:** Interface for keystore operations.

**Pattern:** Repository Interface

#### Lifecycle Methods

##### init()

**Purpose:** Initialize the keystore.

**Signature:**
```cpp
[[nodiscard]] virtual Status init() = 0;
```

##### deinit()

**Purpose:** Deinitialize the keystore.

**Signature:**
```cpp
[[nodiscard]] virtual Status deinit() = 0;
```

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
| `keySlotId` | [in] | Target slot ID |
| `algorithm` | [in] | Algorithm associated with the key |
| `keyData` | [in] | Key data |
| `permissions` | [in] | Access permission flags |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Key imported successfully |
| `ERR_INVALID_KEY_ID` | Invalid slot ID |
| `ERR_KEY_SLOT_FULL` | Slot already occupied |
| `ERR_INVALID_PARAM` | Invalid algorithm or key size |

##### deleteKey()

**Purpose:** Delete a key from a slot.

**Signature:**
```cpp
[[nodiscard]] virtual Status deleteKey(uint8_t keySlotId) = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Key deleted successfully |
| `ERR_INVALID_KEY_ID` | Invalid slot ID |

##### getKey()

**Purpose:** Get a key from a slot (for crypto operations).

**Signature:**
```cpp
[[nodiscard]] virtual Status getKey(
    uint8_t keySlotId,
    types::Key& key) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `keySlotId` | [in] | Slot ID to read from |
| `key` | [out] | Reference to store key data |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Key retrieved successfully |
| `ERR_INVALID_KEY_ID` | Invalid slot ID |
| `ERR_NOT_INITIALIZED` | Slot is empty |

**Security Note:** This method is intended for internal crypto service use only. Applications should not have direct access to key data.

##### getSlotInfo()

**Purpose:** Get information about a key slot.

**Signature:**
```cpp
[[nodiscard]] virtual Status getSlotInfo(
    uint8_t keySlotId,
    types::KeySlotInfo& info) = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Info retrieved successfully |
| `ERR_INVALID_KEY_ID` | Invalid slot ID |

##### clearAll()

**Purpose:** Clear all keys from the keystore.

**Signature:**
```cpp
[[nodiscard]] virtual Status clearAll() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | All keys cleared |

##### isSlotOccupied()

**Purpose:** Check if a key slot is occupied.

**Signature:**
```cpp
[[nodiscard]] virtual bool isSlotOccupied(uint8_t keySlotId) const = 0;
```

**Return Value:**
- `true` - Slot contains a key
- `false` - Slot is empty

### Usage Example

```cpp
#include "ikeystore_service.hpp"

void useKeystore(std::shared_ptr<services::IKeystoreService> keystore) {
    keystore->init();
    
    // Import key
    std::array<uint8_t, 16> keyData = { /* key */ };
    Status status = keystore->importKey(
        0,
        types::Algorithm::AES_128,
        keyData,
        types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
    
    // Check if occupied
    if (keystore->isSlotOccupied(0)) {
        // Slot has a key
    }
    
    // Get slot info
    types::KeySlotInfo info;
    keystore->getSlotInfo(0, info);
    
    // Clear all keys
    keystore->clearAll();
    keystore->deinit();
}
```

---

## File: keystore_service.hpp

### Overview

Concrete implementation of `IKeystoreService` that provides secure key storage.

### Class: KeystoreService

**Purpose:** Implements secure key storage.

**Pattern:** Repository (concrete implementation)

**Inheritance:** `public IKeystoreService`

#### Constructor

```cpp
KeystoreService() = default;
```

**Design Note:** Default constructor - no dependencies required.

#### Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `keySlots_` | `std::array<KeySlotStorage, kMaxKeySlots>` | Key slot array |
| `isInitialized_` | `bool` | Initialization flag |

#### KeySlotStorage Struct (Internal)

```cpp
struct KeySlotStorage {
    types::KeySlotInfo info;
    types::Key key;
    uint32_t accessCount;
};
```

#### Private Methods

##### secureClear()

**Purpose:** Securely clear a key slot.

**Signature:**
```cpp
void secureClear(uint8_t slotId);
```

**Implementation:** Uses volatile writes to prevent optimization.

##### isValidSlotId()

**Purpose:** Check if slot ID is valid.

**Signature:**
```cpp
[[nodiscard]] constexpr bool isValidSlotId(uint8_t slotId) const;
```

**Return Value:** `true` if `slotId < kMaxKeySlots`.

### Implementation Details

##### importKey()

```cpp
Status KeystoreService::importKey(
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> keyData,
    uint8_t permissions)
{
    // 1. Validate slot ID
    if (!isValidSlotId(keySlotId)) {
        return Status(types::StatusCode::ERR_INVALID_KEY_ID);
    }
    
    // 2. Check if slot is occupied
    if (keySlots_[keySlotId].info.isOccupied) {
        return Status(types::StatusCode::ERR_KEY_SLOT_FULL);
    }
    
    // 3. Validate key size
    size_t expectedSize = types::getKeySizeForAlgorithm(algorithm);
    if (keyData.size() != expectedSize) {
        return Status(types::StatusCode::ERR_INVALID_PARAM);
    }
    
    // 4. Store key
    keySlots_[keySlotId].info.id = keySlotId;
    keySlots_[keySlotId].info.isOccupied = true;
    keySlots_[keySlotId].info.algorithm = algorithm;
    keySlots_[keySlotId].info.permissions = permissions;
    keySlots_[keySlotId].info.keySize = keyData.size();
    
    std::copy(keyData.begin(), keyData.end(), 
              keySlots_[keySlotId].key.data);
    keySlots_[keySlotId].key.size = keyData.size();
    keySlots_[keySlotId].key.algorithm = algorithm;
    
    return Status(types::StatusCode::OK);
}
```

### Usage Example

```cpp
#include "keystore_service.hpp"

using namespace ehsm::services;

auto keystore = std::make_shared<KeystoreService>();
keystore->init();

// Import AES-128 key
std::array<uint8_t, 16> aes128Key = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

Status status = keystore->importKey(
    0,
    types::Algorithm::AES_128,
    aes128Key,
    types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);

// Import AES-256 key in slot 1
std::array<uint8_t, 32> aes256Key = { /* 256-bit key */ };
keystore->importKey(1, types::Algorithm::AES_256, aes256Key,
                    types::KeyPermission::ALL);

// Check slot status
bool occupied = keystore->isSlotOccupied(0);  // true

// Get slot info
types::KeySlotInfo info;
keystore->getSlotInfo(0, info);
// info.algorithm == types::Algorithm::AES_128
// info.keySize == 16
// info.permissions == (ENCRYPT | DECRYPT)

keystore->deinit();
```

### Thread Safety

**No** - The service is not thread-safe. External synchronization is required for concurrent access.

### Security Considerations

1. **Key Isolation:** Keys are stored in internal storage and never exposed directly.
2. **Secure Clear:** Keys are securely cleared when deleted or on deinit.
3. **Permission Checking:** Permissions should be checked before key operations.
4. **Memory Safety:** Fixed-size arrays prevent buffer overflows.

---

## File: icrypto_algorithm.hpp

### Overview

Strategy interface for cryptographic algorithms. Each algorithm (AES-128, AES-256, etc.) implements this interface.

### Class: ICryptoAlgorithm

**Purpose:** Interface for cryptographic algorithms (Strategy pattern).

**Pattern:** Strategy Interface

#### Algorithm Information Methods

##### getAlgorithmType()

**Purpose:** Get the algorithm type identifier.

**Signature:**
```cpp
[[nodiscard]] virtual types::Algorithm getAlgorithmType() const = 0;
```

**Return Value:** Algorithm type (e.g., `Algorithm::AES_128`).

##### getKeySize()

**Purpose:** Get the key size for this algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual size_t getKeySize() const = 0;
```

**Return Value:** Key size in bytes.

##### getBlockSize()

**Purpose:** Get the block size for this algorithm.

**Signature:**
```cpp
[[nodiscard]] virtual size_t getBlockSize() const = 0;
```

**Return Value:** Block size in bytes (0 for stream ciphers).

---

#### Cryptographic Operations

##### encrypt()

**Purpose:** Encrypt data.

**Signature:**
```cpp
[[nodiscard]] virtual Status encrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output) = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Encryption successful |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Encryption failed |

##### decrypt()

**Purpose:** Decrypt data.

**Signature:**
```cpp
[[nodiscard]] virtual Status decrypt(
    std::span<const uint8_t> input,
    std::span<uint8_t> output) = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Decryption successful |
| `ERR_INSUFFICIENT_BUFFER` | Output buffer too small |
| `ERR_CRYPTO_FAILED` | Decryption failed |

### Usage Example

```cpp
#include "icrypto_algorithm.hpp"

void useAlgorithm(std::unique_ptr<services::ICryptoAlgorithm> algorithm) {
    // Get algorithm info
    types::Algorithm type = algorithm->getAlgorithmType();
    size_t keySize = algorithm->getKeySize();
    size_t blockSize = algorithm->getBlockSize();
    
    // Encrypt
    std::array<uint8_t, 16> plaintext = { /* data */ };
    std::array<uint8_t, 16> ciphertext;
    Status status = algorithm->encrypt(plaintext, ciphertext);
    
    // Decrypt
    std::array<uint8_t, 16> decrypted;
    status = algorithm->decrypt(ciphertext, decrypted);
}
```

---

## File: aes128_algorithm.hpp

### Overview

AES-128 encryption algorithm implementation.

### Class: Aes128Algorithm

**Purpose:** AES-128 encryption algorithm implementation.

**Pattern:** Strategy (concrete implementation)

**Inheritance:** `public ICryptoAlgorithm`

**Namespace:** `ehsm::services::crypto`

#### Implementation Details

| Method | Return Value |
|--------|--------------|
| `getAlgorithmType()` | `types::Algorithm::AES_128` |
| `getKeySize()` | 16 bytes |
| `getBlockSize()` | 16 bytes |

#### Private Methods

##### encryptBlock()

**Purpose:** Encrypt a single AES block.

**Signature:**
```cpp
void encryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* key) const;
```

**Parameters:**
| Parameter | Description |
|-----------|-------------|
| `input` | 16-byte plaintext block |
| `output` | 16-byte ciphertext block |
| `key` | 16-byte AES key |

##### decryptBlock()

**Purpose:** Decrypt a single AES block.

**Signature:**
```cpp
void decryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* key) const;
```

**Parameters:**
| Parameter | Description |
|-----------|-------------|
| `input` | 16-byte ciphertext block |
| `output` | 16-byte plaintext block |
| `key` | 16-byte AES key |

### Usage Example

```cpp
#include "aes128_algorithm.hpp"

using namespace ehsm::services::crypto;

auto algorithm = std::make_unique<Aes128Algorithm>();

// Verify algorithm type
assert(algorithm->getAlgorithmType() == types::Algorithm::AES_128);
assert(algorithm->getKeySize() == 16);
assert(algorithm->getBlockSize() == 16);

// Encrypt data
std::array<uint8_t, 16> plaintext = { /* 16-byte data */ };
std::array<uint8_t, 16> ciphertext;
Status status = algorithm->encrypt(plaintext, ciphertext);

// Decrypt data
std::array<uint8_t, 16> decrypted;
status = algorithm->decrypt(ciphertext, decrypted);
```

### Thread Safety

**Yes** - The algorithm is stateless and thread-safe.

### Note

**Mock Implementation:** This is a mock implementation for demonstration. In production, replace with actual AES-128 implementation (e.g., using hardware crypto engine or a validated crypto library).

---

## File: aes256_algorithm.hpp

### Overview

AES-256 encryption algorithm implementation.

### Class: Aes256Algorithm

**Purpose:** AES-256 encryption algorithm implementation.

**Pattern:** Strategy (concrete implementation)

**Inheritance:** `public ICryptoAlgorithm`

**Namespace:** `ehsm::services::crypto`

#### Implementation Details

| Method | Return Value |
|--------|--------------|
| `getAlgorithmType()` | `types::Algorithm::AES_256` |
| `getKeySize()` | 32 bytes |
| `getBlockSize()` | 16 bytes |

#### Private Methods

##### encryptBlock()

**Purpose:** Encrypt a single AES block.

**Signature:**
```cpp
void encryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* key) const;
```

**Parameters:**
| Parameter | Description |
|-----------|-------------|
| `input` | 16-byte plaintext block |
| `output` | 16-byte ciphertext block |
| `key` | 32-byte AES-256 key |

##### decryptBlock()

**Purpose:** Decrypt a single AES block.

**Signature:**
```cpp
void decryptBlock(
    const uint8_t* input,
    uint8_t* output,
    const uint8_t* key) const;
```

**Parameters:**
| Parameter | Description |
|-----------|-------------|
| `input` | 16-byte ciphertext block |
| `output` | 16-byte plaintext block |
| `key` | 32-byte AES-256 key |

### Usage Example

```cpp
#include "aes256_algorithm.hpp"

using namespace ehsm::services::crypto;

auto algorithm = std::make_unique<Aes256Algorithm>();

// Verify algorithm type
assert(algorithm->getAlgorithmType() == types::Algorithm::AES_256);
assert(algorithm->getKeySize() == 32);
assert(algorithm->getBlockSize() == 16);

// Encrypt data
std::array<uint8_t, 16> plaintext = { /* 16-byte data */ };
std::array<uint8_t, 16> ciphertext;
Status status = algorithm->encrypt(plaintext, ciphertext);

// Decrypt data
std::array<uint8_t, 16> decrypted;
status = algorithm->decrypt(ciphertext, decrypted);
```

### Thread Safety

**Yes** - The algorithm is stateless and thread-safe.

### Note

**Mock Implementation:** This is a mock implementation for demonstration. In production, replace with actual AES-256 implementation.

---

## Service Layer Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    CryptoService                             │
│  ┌─────────────────────────────────────────────────────┐    │
│  │  ICryptoService (Interface)                         │    │
│  │  - init()                                           │    │
│  │  - deinit()                                         │    │
│  │  - encrypt()                                        │    │
│  │  - decrypt()                                        │    │
│  │  - hash()                                           │    │
│  └─────────────────────────────────────────────────────┘    │
│                            │                                 │
│              ┌─────────────┴─────────────┐                  │
│              │                           │                  │
│    ┌─────────▼─────────┐     ┌──────────▼──────────┐       │
│    │ IKeystoreService  │     │ ICryptoAlgorithm    │       │
│    │ (Interface)       │     │ (Strategy Interface)│       │
│    └─────────┬─────────┘     └──────────┬──────────┘       │
│              │                          │                   │
│    ┌─────────▼─────────┐     ┌──────────▼──────────┐       │
│    │ KeystoreService   │     │ Aes128Algorithm     │       │
│    │ (Implementation)  │     │ Aes256Algorithm     │       │
│    └───────────────────┘     └─────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
```

---

## Complete Service Layer Example

```cpp
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes128_algorithm.hpp"
#include "aes256_algorithm.hpp"

using namespace ehsm::services;

class ServiceExample {
public:
    Status run() {
        // 1. Create keystore
        auto keystore = std::make_shared<KeystoreService>();
        keystore->init();
        
        // 2. Import keys
        std::array<uint8_t, 16> aes128Key = { /* key */ };
        std::array<uint8_t, 32> aes256Key = { /* key */ };
        
        keystore->importKey(0, types::Algorithm::AES_128, aes128Key,
                            types::KeyPermission::ENCRYPT);
        keystore->importKey(1, types::Algorithm::AES_256, aes256Key,
                            types::KeyPermission::ENCRYPT);
        
        // 3. Create crypto service with AES-128
        auto aes128Algo = std::make_unique<crypto::Aes128Algorithm>();
        auto cryptoService128 = std::make_shared<CryptoService>(
            keystore, std::move(aes128Algo));
        cryptoService128->init();
        
        // 4. Encrypt with AES-128
        std::array<uint8_t, 16> plaintext = { /* data */ };
        std::array<uint8_t, 16> ciphertext;
        Status status = cryptoService128->encrypt(plaintext, ciphertext, 0);
        
        // 5. Create another crypto service with AES-256
        auto aes256Algo = std::make_unique<crypto::Aes256Algorithm>();
        auto cryptoService256 = std::make_shared<CryptoService>(
            keystore, std::move(aes256Algo));
        cryptoService256->init();
        
        // 6. Encrypt with AES-256
        std::array<uint8_t, 16> ciphertext256;
        status = cryptoService256->encrypt(plaintext, ciphertext256, 1);
        
        // Cleanup
        cryptoService256->deinit();
        cryptoService128->deinit();
        keystore->clearAll();
        keystore->deinit();
        
        return Status(types::StatusCode::OK);
    }
};
```

---

## Related Files

| File | Relationship |
|------|--------------|
| [api_layer.md](api_layer.md) | API layer uses these services |
| [types.md](types.md) | Core types used by services |
| [middleware.md](middleware.md) | Middleware coordinates with services |
