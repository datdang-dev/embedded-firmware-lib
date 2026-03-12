# Core Types Documentation

## Overview

This document covers the core type definitions used throughout the Embedded HSM firmware. These types provide the foundation for all HSM operations.

**Files:**
- `common.hpp` - Common includes and type aliases
- `status.hpp` - Status class for error handling
- `algorithm.hpp` - Algorithm type definitions
- `key_slot.hpp` - Key slot and key types

---

## File: common.hpp

### Overview

Central include file that aggregates all common type definitions and standard library headers. This file should be included by most HSM components.

### Includes

```cpp
#include "status.hpp"
#include "algorithm.hpp"
#include "key_slot.hpp"

#include <cstdint>
#include <cstddef>
#include <cstdarg>
#include <span>
#include <memory>
#include <array>
```

### Type Aliases

| Alias | Type | Description |
|-------|------|-------------|
| `Status` | `ehsm::types::Status` | Status class for error handling |
| `StatusCode` | `ehsm::types::StatusCode` | Status code enumeration |

### Usage Example

```cpp
#include "common.hpp"

// Status and StatusCode are now available
Status result = someOperation();
if (result.code() == StatusCode::OK) {
    // Success
}
```

---

## File: status.hpp

### Overview

Provides exception-free error handling through the `Status` class and `StatusCode` enumeration. This approach is essential for embedded systems where exceptions may be disabled.

### StatusCode Enumeration

```cpp
enum class StatusCode : int32_t {
    OK = 0,                    ///< Success
    ERR_INVALID_PARAM = -1,    ///< Invalid parameter
    ERR_INSUFFICIENT_BUFFER = -2,  ///< Buffer too small
    ERR_CRYPTO_FAILED = -3,    ///< Cryptographic operation failed
    ERR_SESSION_INVALID = -4,  ///< Invalid session ID
    ERR_NOT_INITIALIZED = -5,  ///< Component not initialized
    ERR_KEY_SLOT_FULL = -6,    ///< No available key slots
    ERR_INVALID_KEY_ID = -7,   ///< Invalid key slot ID
    ERR_AUTH_FAILED = -8,      ///< Authentication failed
    ERR_QUEUE_FULL = -9,       ///< Command queue full
    ERR_HARDWARE = -10,        ///< Hardware error
    ERR_UNSUPPORTED = -11,     ///< Unsupported operation
    ERR_TIMEOUT = -12,         ///< Operation timed out
    ERR_INTERNAL = -13,        ///< Internal error
    ERR_BUSY = -14             ///< Resource busy
};
```

### Status Class

**Purpose:** Type-safe wrapper for error handling without exceptions.

#### Methods

| Method | Description |
|--------|-------------|
| `Status()` | Default constructor (OK status) |
| `explicit Status(StatusCode)` | Construct from status code |
| `bool isOk() const` | Check if status is OK |
| `StatusCode code() const` | Get the status code |
| `const char* message() const` | Get human-readable error message |
| `explicit operator bool() const` | Bool conversion (true if OK) |
| `operator==(const Status&) const` | Equality comparison |
| `operator!=(const Status&) const` | Inequality comparison |

#### Utility Functions

| Function | Description |
|----------|-------------|
| `const char* toString(StatusCode)` | Convert status code to string |
| `const char* toString(const Status&)` | Convert status to string |

### Usage Example

```cpp
#include "status.hpp"

using namespace ehsm::types;

// Method returning Status
Status performOperation() {
    if (!isValidParameter()) {
        return Status(StatusCode::ERR_INVALID_PARAM);
    }
    // ... operation ...
    return Status(StatusCode::OK);
}

// Checking status
Status result = performOperation();

// Method 1: Using isOk()
if (!result.isOk()) {
    handleError(result.code(), result.message());
}

// Method 2: Using bool conversion
if (result) {
    // Success
} else {
    // Error
}

// Method 3: Comparing with specific code
if (result == Status(StatusCode::ERR_TIMEOUT)) {
    // Handle timeout
}

// Method 4: Switch on code
switch (result.code()) {
    case StatusCode::OK:
        // Success
        break;
    case StatusCode::ERR_INVALID_PARAM:
        // Invalid parameter
        break;
    default:
        // Other error
        break;
}
```

### Thread Safety

**Yes** - The `Status` class is immutable and thread-safe once constructed.

### Error Codes Reference

| Code | Value | Description |
|------|-------|-------------|
| `OK` | 0 | Operation completed successfully |
| `ERR_INVALID_PARAM` | -1 | One or more parameters are invalid |
| `ERR_INSUFFICIENT_BUFFER` | -2 | Output buffer is too small |
| `ERR_CRYPTO_FAILED` | -3 | Cryptographic operation failed |
| `ERR_SESSION_INVALID` | -4 | Session ID is not valid |
| `ERR_NOT_INITIALIZED` | -5 | Component not initialized |
| `ERR_KEY_SLOT_FULL` | -6 | All key slots are occupied |
| `ERR_INVALID_KEY_ID` | -7 | Key slot ID is out of range |
| `ERR_AUTH_FAILED` | -8 | Authentication/authorization failed |
| `ERR_QUEUE_FULL` | -9 | Command queue is full |
| `ERR_HARDWARE` | -10 | Hardware error occurred |
| `ERR_UNSUPPORTED` | -11 | Operation not supported |
| `ERR_TIMEOUT` | -12 | Operation timed out |
| `ERR_INTERNAL` | -13 | Internal system error |
| `ERR_BUSY` | -14 | Resource is currently busy |

---

## File: algorithm.hpp

### Overview

Defines supported encryption algorithms, key permissions, and utility functions for algorithm-related operations.

### Algorithm Enumeration

```cpp
enum class Algorithm : uint8_t {
    NONE = 0,       ///< No algorithm
    AES_128 = 1,    ///< AES with 128-bit key
    AES_256 = 2,    ///< AES with 256-bit key
    RSA_2048 = 3,   ///< RSA with 2048-bit key
    RSA_4096 = 4,   ///< RSA with 4096-bit key
    ECC_P256 = 5,   ///< Elliptic Curve P-256
    SHA_256 = 6,    ///< SHA-256 hash
    CHACHA20 = 7    ///< ChaCha20 stream cipher
};
```

### KeyPermission Enumeration

Bit flags for controlling key access permissions:

```cpp
enum class KeyPermission : uint8_t {
    NONE = 0x00,    ///< No permissions
    ENCRYPT = 0x01, ///< Can encrypt data
    DECRYPT = 0x02, ///< Can decrypt data
    SIGN = 0x04,    ///< Can sign data
    VERIFY = 0x08,  ///< Can verify signatures
    EXPORT = 0x10,  ///< Can export key (use with caution)
    ALL = 0xFF      ///< All permissions
};
```

### Operators

| Operator | Description |
|----------|-------------|
| `operator|(KeyPermission, KeyPermission)` | Bitwise OR for combining permissions |
| `operator&(KeyPermission, KeyPermission)` | Bitwise AND for checking permissions |

### Utility Functions

#### getKeySizeForAlgorithm()

**Purpose:** Get the key size in bytes for a given algorithm.

**Signature:**
```cpp
[[nodiscard]] constexpr size_t getKeySizeForAlgorithm(Algorithm algorithm);
```

**Return Values:**

| Algorithm | Key Size |
|-----------|----------|
| AES_128 | 16 bytes |
| AES_256 | 32 bytes |
| CHACHA20 | 32 bytes |
| RSA_2048 | 256 bytes |
| RSA_4096 | 512 bytes |
| ECC_P256 | 32 bytes |
| Others | 0 (unknown/variable) |

### Usage Example

```cpp
#include "algorithm.hpp"

using namespace ehsm::types;

// Using Algorithm enum
Algorithm algo = Algorithm::AES_128;

// Combining permissions
KeyPermission perms = KeyPermission::ENCRYPT | KeyPermission::DECRYPT;

// Checking permissions
if (perms & KeyPermission::ENCRYPT) {
    // Can encrypt
}

// Getting key size
size_t keySize = getKeySizeForAlgorithm(Algorithm::AES_256);
// keySize == 32

// Switch on algorithm
switch (algo) {
    case Algorithm::AES_128:
    case Algorithm::AES_256:
        // AES operations
        break;
    case Algorithm::RSA_2048:
        // RSA operations
        break;
    default:
        // Unsupported
        break;
}
```

### Thread Safety

**Yes** - All types and functions are immutable and thread-safe.

---

## File: key_slot.hpp

### Overview

Defines key storage structures, session types, and constants for key slot management.

### Constants

| Constant | Type | Value | Description |
|----------|------|-------|-------------|
| `kMaxKeySize` | `size_t` | 64 | Maximum key size in bytes (512-bit) |
| `kMaxKeySlots` | `uint8_t` | 8 | Maximum number of key slots |
| `kMaxSessions` | `uint8_t` | 4 | Maximum number of sessions |
| `kInvalidKeySlotId` | `uint8_t` | 0xFF | Invalid key slot ID marker |
| `kInvalidSessionId` | `SessionId` | 0xFFFFFFFF | Invalid session ID |
| `kInvalidJobId` | `JobId` | 0xFFFFFFFF | Invalid job ID |

### Type Aliases

| Type | Underlying | Description |
|------|------------|-------------|
| `SessionId` | `uint32_t` | Session identifier |
| `JobId` | `uint32_t` | Job identifier |

### Key Struct

**Purpose:** Secure storage for cryptographic keys.

```cpp
struct Key {
    uint8_t data[kMaxKeySize];    ///< Key data buffer
    size_t size;                   ///< Actual key size in bytes
    Algorithm algorithm;           ///< Algorithm associated with key
};
```

#### Methods

| Method | Description |
|--------|-------------|
| `Key()` | Default constructor (zeroes all data) |
| `void secureClear()` | Securely clear key data (volatile write) |

**Security Note:** The `secureClear()` method uses volatile writes to prevent compiler optimization from removing the clear operation.

### KeySlotInfo Struct

**Purpose:** Information about a key slot without exposing the actual key.

```cpp
struct KeySlotInfo {
    uint8_t id;                  ///< Slot ID (0-based)
    bool isOccupied;             ///< Slot is occupied
    Algorithm algorithm;         ///< Algorithm associated with key
    uint8_t permissions;         ///< Access permission flags
    size_t keySize;              ///< Key size in bytes
};
```

### Usage Example

```cpp
#include "key_slot.hpp"

using namespace ehsm::types;

// Creating a key
Key myKey;
myKey.size = 16;
myKey.algorithm = Algorithm::AES_128;
// ... copy key data to myKey.data ...

// Securely clearing a key
myKey.secureClear();

// Key slot info
KeySlotInfo info;
info.id = 0;
info.isOccupied = true;
info.algorithm = Algorithm::AES_128;
info.permissions = KeyPermission::ENCRYPT | KeyPermission::DECRYPT;
info.keySize = 16;

// Checking session validity
SessionId sessionId = kInvalidSessionId;
if (sessionId == kInvalidSessionId) {
    // Invalid session
}

// Validating key slot ID
uint8_t slotId = 5;
if (slotId < kMaxKeySlots) {
    // Valid slot ID
}
```

### Thread Safety

**Partial** - The `Key` struct itself is not thread-safe. External synchronization is required when accessing shared key data. The `secureClear()` method is safe to call from any thread but does not provide atomicity.

### Security Considerations

1. **Key Exposure:** Keys should never be exposed outside the keystore service boundary.
2. **Secure Clear:** Always call `secureClear()` before a `Key` object goes out of scope.
3. **Volatile Writes:** The `secureClear()` method uses volatile writes to prevent optimization.
4. **Memory Safety:** Key data is stored in a fixed-size buffer to prevent overflow.

---

## Related Files

| File | Relationship |
|------|--------------|
| [api_layer.md](api_layer.md) | API layer uses these types |
| [services.md](services.md) | Services operate on these types |
| [middleware.md](middleware.md) | Middleware manages sessions using these types |

## Cross-References

- `ihsm_api.hpp` - Uses `Status`, `Algorithm`, `SessionId`, `KeySlotInfo`
- `icrypto_service.hpp` - Uses `Status`, `Algorithm`, `Key`
- `isession_manager.hpp` - Uses `Status`, `SessionId`
