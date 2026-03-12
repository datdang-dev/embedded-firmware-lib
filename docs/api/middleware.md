# Middleware Layer Documentation

## Overview

The middleware layer provides session management functionality, enabling secure multi-user access to HSM resources. Sessions track user context and ensure proper authorization for cryptographic operations.

**Files:**
- `isession_manager.hpp` - Session manager interface
- `session_manager.hpp` - Session manager implementation

---

## File: isession_manager.hpp

### Overview

Abstract interface for session management. This interface enables dependency injection and testing with mock implementations.

### Class: ISessionManager

**Purpose:** Interface for session management.

**Pattern:** Manager Interface

**Namespace:** `ehsm::mw`

#### Lifecycle Methods

##### init()

**Purpose:** Initialize the session manager.

**Signature:**
```cpp
[[nodiscard]] virtual Status init() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Initialization successful |
| `ERR_NOT_INITIALIZED` | Dependencies not ready |
| `ERR_INTERNAL` | Internal initialization error |

**Thread Safety:** No - Call once during system initialization.

##### deinit()

**Purpose:** Deinitialize the session manager.

**Signature:**
```cpp
[[nodiscard]] virtual Status deinit() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Deinitialization successful |
| `ERR_NOT_INITIALIZED` | Manager not initialized |
| `ERR_BUSY` | Active sessions still exist |

**Thread Safety:** No - Call once during system shutdown.

---

#### Session Management Methods

##### createSession()

**Purpose:** Create a new HSM session.

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
| `ERR_SESSION_INVALID` | Maximum sessions reached |
| `ERR_NOT_INITIALIZED` | Manager not initialized |
| `ERR_QUEUE_FULL` | Session queue is full |

**Usage Example:**
```cpp
types::SessionId sessionId;
Status result = sessionManager->createSession(sessionId);
if (result.isOk()) {
    // sessionId now contains a valid session ID
    // Use sessionId for subsequent HSM operations
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
| `ERR_NOT_INITIALIZED` | Manager not initialized |

**Usage Example:**
```cpp
Status result = sessionManager->closeSession(sessionId);
if (result.isOk()) {
    // Session closed, sessionId is now invalid
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
- `false` - Session is invalid, expired, or closed

**Usage Example:**
```cpp
if (sessionManager->isSessionValid(sessionId)) {
    // Session is valid, proceed with operation
} else {
    // Session invalid, create new session or reject operation
}
```

**Thread Safety:** Yes - Read-only operation.

---

## File: session_manager.hpp

### Overview

Concrete implementation of `ISessionManager` that manages session state and lifecycle.

### Class: SessionManager

**Purpose:** Session manager implementation.

**Pattern:** Manager (concrete implementation)

**Inheritance:** `public ISessionManager`

**Namespace:** `ehsm::mw`

#### Constructor

```cpp
SessionManager() = default;
```

**Design Note:** Default constructor - no dependencies required. Uses internal storage for session state.

#### Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `sessions_` | `std::array<Session, kMaxSessions>` | Session storage array |
| `isInitialized_` | `bool` | Initialization flag |
| `nextSessionId_` | `types::SessionId` | Next session ID to assign |

#### Internal Session Struct

```cpp
struct Session {
    types::SessionId id{kInvalidSessionId};
    bool isActive{false};
    uint32_t jobCount{0U};
};
```

**Fields:**
| Field | Type | Description |
|-------|------|-------------|
| `id` | `types::SessionId` | Session identifier |
| `isActive` | `bool` | Session active flag |
| `jobCount` | `uint32_t` | Number of jobs processed |

---

### Implementation Details

##### init()

```cpp
Status SessionManager::init() {
    if (isInitialized_) {
        return Status(types::StatusCode::ERR_BUSY);
    }
    
    // Reset all sessions
    for (auto& session : sessions_) {
        session.id = types::kInvalidSessionId;
        session.isActive = false;
        session.jobCount = 0;
    }
    
    nextSessionId_ = 1;
    isInitialized_ = true;
    
    return Status(types::StatusCode::OK);
}
```

##### createSession()

```cpp
Status SessionManager::createSession(types::SessionId& sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    
    // Find available slot
    for (auto& session : sessions_) {
        if (!session.isActive) {
            session.id = nextSessionId_;
            session.isActive = true;
            session.jobCount = 0;
            sessionId = nextSessionId_;
            
            // Increment for next session (wrap around)
            nextSessionId_ = (nextSessionId_ + 1) & 0x7FFFFFFF;
            
            return Status(types::StatusCode::OK);
        }
    }
    
    // No available slots
    return Status(types::StatusCode::ERR_SESSION_INVALID);
}
```

##### closeSession()

```cpp
Status SessionManager::closeSession(types::SessionId sessionId) {
    if (!isInitialized_) {
        return Status(types::StatusCode::ERR_NOT_INITIALIZED);
    }
    
    for (auto& session : sessions_) {
        if (session.isActive && session.id == sessionId) {
            session.isActive = false;
            session.id = types::kInvalidSessionId;
            session.jobCount = 0;
            return Status(types::StatusCode::OK);
        }
    }
    
    return Status(types::StatusCode::ERR_SESSION_INVALID);
}
```

##### isSessionValid()

```cpp
bool SessionManager::isSessionValid(types::SessionId sessionId) const {
    if (!isInitialized_ || sessionId == types::kInvalidSessionId) {
        return false;
    }
    
    for (const auto& session : sessions_) {
        if (session.isActive && session.id == sessionId) {
            return true;
        }
    }
    
    return false;
}
```

---

### Usage Example

```cpp
#include "session_manager.hpp"

using namespace ehsm::mw;

class SessionExample {
public:
    Status run() {
        // Create session manager
        auto sessionManager = std::make_shared<SessionManager>();
        
        // Initialize
        Status status = sessionManager->init();
        if (!status.isOk()) {
            return status;
        }
        
        // Create session 1
        types::SessionId session1;
        status = sessionManager->createSession(session1);
        // session1 == 1
        
        // Create session 2
        types::SessionId session2;
        status = sessionManager->createSession(session2);
        // session2 == 2
        
        // Validate sessions
        if (sessionManager->isSessionValid(session1)) {
            // Session 1 is valid
        }
        
        // Close session 1
        status = sessionManager->closeSession(session1);
        
        // Session 1 is now invalid
        if (!sessionManager->isSessionValid(session1)) {
            // Session 1 is closed
        }
        
        // Session 2 is still valid
        if (sessionManager->isSessionValid(session2)) {
            // Session 2 is still active
        }
        
        // Deinitialize (should fail if sessions are still active)
        sessionManager->closeSession(session2);
        status = sessionManager->deinit();
        
        return Status(types::StatusCode::OK);
    }
};
```

---

### Session Lifecycle

```
┌─────────────────────────────────────────────────────────────┐
│                    Session Lifecycle                         │
│                                                              │
│    ┌──────────┐     createSession()     ┌──────────┐        │
│    │  NONE    │ ──────────────────────► │  ACTIVE  │        │
│    └──────────┘                         └────┬─────┘        │
│          ▲                                   │              │
│          │                                   │              │
│          │              closeSession()       │              │
│          │ ──────────────────────────────────┘              │
│          │                                                  │
│          │              deinit()                            │
│          └────────────────────────────────────────          │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

### Thread Safety

**No** - The `SessionManager` is not thread-safe. External synchronization is required for:
- Concurrent session creation
- Concurrent session closure
- Concurrent session validation during modification

**Recommendation:** Use a mutex or similar synchronization primitive when accessing from multiple threads:

```cpp
#include <mutex>

class ThreadSafeSessionManager {
public:
    Status createSession(types::SessionId& sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        return sessionManager_->createSession(sessionId);
    }
    
    Status closeSession(types::SessionId sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        return sessionManager_->closeSession(sessionId);
    }
    
    bool isSessionValid(types::SessionId sessionId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sessionManager_->isSessionValid(sessionId);
    }
    
private:
    std::shared_ptr<mw::ISessionManager> sessionManager_;
    mutable std::mutex mutex_;
};
```

---

### Resource Limits

| Resource | Limit | Description |
|----------|-------|-------------|
| Maximum Sessions | 4 | Defined by `kMaxSessions` |
| Session ID Range | 1 to 0x7FFFFFFF | Wraps around on overflow |

---

### Error Handling

The session manager uses the standard `Status` error handling pattern:

```cpp
Status result = sessionManager->createSession(sessionId);
switch (result.code()) {
    case types::StatusCode::OK:
        // Success
        break;
    case types::StatusCode::ERR_NOT_INITIALIZED:
        // Call init() first
        break;
    case types::StatusCode::ERR_SESSION_INVALID:
        // Maximum sessions reached
        break;
    default:
        // Handle other errors
        break;
}
```

---

### Integration with API Layer

The session manager is used by the API layer to validate sessions before cryptographic operations:

```cpp
Status HsmApiImpl::encrypt(
    types::SessionId sessionId,
    uint8_t keySlotId,
    types::Algorithm algorithm,
    std::span<const uint8_t> input,
    std::span<uint8_t> output)
{
    // Validate session first
    if (!sessionManager_->isSessionValid(sessionId)) {
        return Status(types::StatusCode::ERR_SESSION_INVALID);
    }
    
    // Proceed with encryption
    return cryptoService_->encrypt(input, output, keySlotId);
}
```

---

## Complete Middleware Example

```cpp
#include "session_manager.hpp"
#include "crypto_service.hpp"
#include "keystore_service.hpp"
#include "aes128_algorithm.hpp"
#include "hsm_api_impl.hpp"

using namespace ehsm;

class MiddlewareExample {
public:
    Status run() {
        // 1. Create all components
        auto sessionManager = std::make_shared<mw::SessionManager>();
        auto keystore = std::make_shared<services::KeystoreService>();
        auto algorithm = std::make_unique<services::crypto::Aes128Algorithm>();
        auto cryptoService = std::make_shared<services::CryptoService>(
            keystore, std::move(algorithm));
        auto hsmApi = std::make_unique<api::HsmApiImpl>(
            sessionManager, cryptoService, keystore);
        
        // 2. Initialize all components
        sessionManager->init();
        keystore->init();
        cryptoService->init();
        hsmApi->init();
        
        // 3. Create a session
        types::SessionId sessionId;
        Status status = hsmApi->createSession(sessionId);
        if (!status.isOk()) {
            return status;
        }
        
        // 4. Import a key
        std::array<uint8_t, 16> keyData = { /* 128-bit key */ };
        status = hsmApi->importKey(
            0,
            types::Algorithm::AES_128,
            keyData,
            types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
        if (!status.isOk()) {
            return status;
        }
        
        // 5. Perform encryption
        std::array<uint8_t, 16> plaintext = { /* data */ };
        std::array<uint8_t, 16> ciphertext;
        status = hsmApi->encrypt(
            sessionId, 0, types::Algorithm::AES_128,
            plaintext, ciphertext);
        if (!status.isOk()) {
            return status;
        }
        
        // 6. Perform decryption
        std::array<uint8_t, 16> decrypted;
        status = hsmApi->decrypt(
            sessionId, 0, types::Algorithm::AES_128,
            ciphertext, decrypted);
        if (!status.isOk()) {
            return status;
        }
        
        // 7. Cleanup
        hsmApi->deleteKey(0);
        hsmApi->closeSession(sessionId);
        hsmApi->deinit();
        
        return Status(types::StatusCode::OK);
    }
};
```

---

## Related Files

| File | Relationship |
|------|--------------|
| [api_layer.md](api_layer.md) | API layer uses session manager |
| [services.md](services.md) | Services coordinate with session manager |
| [types.md](types.md) | Uses SessionId type |

---

## Cross-References

- `ihsm_api.hpp` - Uses `ISessionManager` for session validation
- `hsm_api_impl.hpp` - Contains `sessionManager_` member
- `key_slot.hpp` - Defines `SessionId` type and `kMaxSessions` constant
