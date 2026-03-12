# Embedded HSM Architecture

## Overview

The Embedded HSM (Hardware Security Module) firmware is a layered architecture designed for embedded systems. It provides secure cryptographic operations with a clean separation of concerns.

**Version:** 1.0.0  
**Last Updated:** 2024

---

## System Architecture

### Layer Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           APPLICATION LAYER                              │
│                    (User Application / Test Code)                        │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                            API LAYER (Facade)                            │
│  ┌─────────────────────────┐         ┌─────────────────────────────┐   │
│  │     IHsmApi             │         │      HsmApiImpl             │   │
│  │   (Interface)           │◄────────│   (Implementation)          │   │
│  │   - init()              │         │   - init()                  │   │
│  │   - createSession()     │         │   - createSession()         │   │
│  │   - encrypt()           │         │   - encrypt()               │   │
│  │   - decrypt()           │         │   - decrypt()               │   │
│  │   - importKey()         │         │   - importKey()             │   │
│  └─────────────────────────┘         └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    ▼               ▼               ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         SERVICE LAYER                                    │
│  ┌─────────────────────────────────────────────────────────────────┐    │
│  │                    ICryptoService                                │    │
│  │                   (Crypto Interface)                             │    │
│  └────────────────────────────┬────────────────────────────────────┘    │
│                               │                                          │
│                  ┌────────────▼────────────┐                            │
│                  │    CryptoService        │                            │
│                  │   (Strategy Context)    │                            │
│                  └────────────┬────────────┘                            │
│                               │                                          │
│         ┌─────────────────────┴─────────────────────┐                   │
│         │                                           │                   │
│ ┌───────▼──────────┐                      ┌────────▼────────┐          │
│ │ ICryptoAlgorithm │                      │ IKeystoreService│          │
│ │ (Strategy)       │                      │ (Repository)    │          │
│ └───────┬──────────┘                      └────────┬────────┘          │
│         │                                          │                   │
│ ┌───────▼──────────┐                      ┌────────▼────────┐          │
│ │ Aes128Algorithm  │                      │ KeystoreService │          │
│ │ Aes256Algorithm  │                      │                 │          │
│ └──────────────────┘                      └─────────────────┘          │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        MIDDLEWARE LAYER                                  │
│  ┌─────────────────────────┐         ┌─────────────────────────────┐   │
│  │   ISessionManager       │         │    SessionManager           │   │
│  │   (Interface)           │◄────────│    (Implementation)         │   │
│  │   - createSession()     │         │    - Session storage        │   │
│  │   - closeSession()      │         │    - Validation             │   │
│  └─────────────────────────┘         └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         MCAL LAYER                                       │
│  ┌─────────────────────────┐         ┌─────────────────────────────┐   │
│  │      IUart              │         │   UartStub / PlatformImpl   │   │
│  │   (Hardware Interface)  │         │   (Hardware Implementation) │   │
│  └─────────────────────────┘         └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         HARDWARE LAYER                                   │
│              (STM32 / ESP32 / HOST / Other Platform)                     │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Design Patterns

### 1. Facade Pattern

**Location:** API Layer  
**Purpose:** Provide a unified interface to the subsystem

```cpp
// HsmApiImpl is the Facade
class HsmApiImpl : public IHsmApi {
    // Delegates to:
    // - SessionManager (middleware)
    // - CryptoService (service)
    // - KeystoreService (service)
};
```

**Benefits:**
- Simplified interface for clients
- Loose coupling between layers
- Easy to swap implementations

---

### 2. Strategy Pattern

**Location:** Service Layer (Crypto)  
**Purpose:** Enable interchangeable algorithms

```cpp
// Strategy Interface
class ICryptoAlgorithm {
    virtual Status encrypt(...) = 0;
    virtual Status decrypt(...) = 0;
};

// Concrete Strategies
class Aes128Algorithm : public ICryptoAlgorithm { /* ... */ };
class Aes256Algorithm : public ICryptoAlgorithm { /* ... */ };

// Context
class CryptoService {
    std::unique_ptr<ICryptoAlgorithm> algorithm_;
};
```

**Benefits:**
- Easy to add new algorithms
- Runtime algorithm selection
- Open/Closed Principle

---

### 3. Dependency Injection

**Location:** All layers  
**Purpose:** Invert control, enable testing

```cpp
// Constructor injection
HsmApiImpl::HsmApiImpl(
    std::shared_ptr<mw::ISessionManager> sessionManager,
    std::shared_ptr<services::ICryptoService> cryptoService,
    std::shared_ptr<services::IKeystoreService> keystoreService);

CryptoService::CryptoService(
    std::shared_ptr<IKeystoreService> keystore,
    std::unique_ptr<ICryptoAlgorithm> algorithm);
```

**Benefits:**
- Loose coupling
- Easy unit testing with mocks
- Flexible configuration

---

### 4. Repository Pattern

**Location:** Service Layer (Keystore)  
**Purpose:** Abstract data access

```cpp
class IKeystoreService {
    virtual Status importKey(...) = 0;
    virtual Status getKey(...) = 0;
    virtual Status deleteKey(...) = 0;
};
```

**Benefits:**
- Separation of concerns
- Easy to swap storage backends
- Testable interface

---

## Component Relationships

### Dependency Graph

```
┌──────────────────────────────────────────────────────────────────┐
│                         HsmApiImpl                                │
│                                                                   │
│  Dependencies:                                                    │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ ISessionManager │  │ ICryptoService  │  │IKeystoreService │  │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘  │
│           │                    │                    │            │
│           │                    │                    │            │
│           ▼                    ▼                    │            │
│  ┌─────────────────┐  ┌─────────────────┐          │            │
│  │ SessionManager  │  │ CryptoService   │◄─────────┘            │
│  └─────────────────┘  └────────┬────────┘                       │
│                                │                                 │
│                                ▼                                 │
│                       ┌─────────────────┐                       │
│                       │ICryptoAlgorithm │                       │
│                       └────────┬────────┘                       │
│                                │                                 │
│           ┌────────────────────┼────────────────────┐            │
│           ▼                    ▼                    ▼            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │Aes128Algorithm  │  │Aes256Algorithm  │  │  (More...)      │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

---

## Data Flow

### Encryption Flow

```
Application
    │
    │ 1. encrypt(sessionId, keySlotId, algorithm, input, output)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                      HsmApiImpl                              │
│  1. Validate session (sessionId)                            │
│  2. Check session validity                                   │
└─────────────────────────────────────────────────────────────┘
    │
    │ 2. encrypt(input, output, keySlotId)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                     CryptoService                            │
│  1. Get key from keystore (keySlotId)                       │
│  2. Validate key size                                        │
│  3. Delegate to algorithm                                    │
└─────────────────────────────────────────────────────────────┘
    │
    │ 3. getKey(keySlotId, key)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                    KeystoreService                           │
│  1. Validate slot ID                                         │
│  2. Check slot occupied                                      │
│  3. Return key copy                                          │
└─────────────────────────────────────────────────────────────┘
    │
    │ 4. encrypt(input, output)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                   Aes128Algorithm                            │
│  1. Perform AES-128 encryption                               │
│  2. Return status                                            │
└─────────────────────────────────────────────────────────────┘
    │
    │ Status propagates back up
    ▼
Application receives Status
```

---

## Key Management Flow

### Key Import Flow

```
Application
    │
    │ importKey(keySlotId, algorithm, keyData, permissions)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                      HsmApiImpl                              │
│  1. Delegate to keystore service                            │
└─────────────────────────────────────────────────────────────┘
    │
    │ importKey(keySlotId, algorithm, keyData, permissions)
    ▼
┌─────────────────────────────────────────────────────────────┐
│                    KeystoreService                           │
│  1. Validate slot ID (0 to kMaxKeySlots-1)                  │
│  2. Check if slot is empty                                   │
│  3. Validate key size matches algorithm                      │
│  4. Store key in secure memory                               │
│  5. Set slot info (algorithm, permissions, size)            │
└─────────────────────────────────────────────────────────────┘
    │
    │ Status::OK or error code
    ▼
Application receives Status
```

---

## Session Management Flow

### Session Lifecycle

```
                    ┌──────────────┐
                    │    NONE      │
                    │ (Initial)    │
                    └──────┬───────┘
                           │
                           │ init()
                           ▼
                    ┌──────────────┐
                    │   READY      │
                    │ (Initialized)│
                    └──────┬───────┘
                           │
                           │ createSession()
                           ▼
                    ┌──────────────┐
          ┌────────│   ACTIVE     │◄───────┐
          │        │ (Session OK) │        │
          │        └──────┬───────┘        │
          │               │                │
          │               │ closeSession() │
          │               ▼                │
          │        ┌──────────────┐        │
          │        │   CLOSED     │        │
          │        │ (Released)   │        │
          │        └──────────────┘        │
          │                                │
          └────────────────────────────────┘
                   Multiple sessions
```

---

## Error Handling Strategy

### Status Propagation

```
┌─────────────────────────────────────────────────────────────┐
│                    Error Handling Flow                       │
└─────────────────────────────────────────────────────────────┘

Algorithm Layer:
  Aes128Algorithm::encrypt()
    │
    │ Returns: Status(ERR_CRYPTO_FAILED)
    ▼
Service Layer:
  CryptoService::encrypt()
    │
    │ Propagates: return algorithm_->encrypt(...)
    ▼
API Layer:
  HsmApiImpl::encrypt()
    │
    │ Propagates: return cryptoService_->encrypt(...)
    ▼
Application:
  Status status = hsmApi->encrypt(...);
  if (!status.isOk()) {
      handleError(status.code(), status.message());
  }
```

### Error Code Categories

| Category | Codes | Description |
|----------|-------|-------------|
| Success | `OK` | Operation completed successfully |
| Parameter Errors | `ERR_INVALID_PARAM`, `ERR_INSUFFICIENT_BUFFER` | Invalid input |
| State Errors | `ERR_NOT_INITIALIZED`, `ERR_SESSION_INVALID` | Invalid state |
| Resource Errors | `ERR_KEY_SLOT_FULL`, `ERR_QUEUE_FULL`, `ERR_BUSY` | Resource exhausted |
| Operation Errors | `ERR_CRYPTO_FAILED`, `ERR_AUTH_FAILED`, `ERR_HARDWARE` | Operation failed |
| System Errors | `ERR_UNSUPPORTED`, `ERR_TIMEOUT`, `ERR_INTERNAL` | System-level errors |

---

## Thread Safety Model

### Current State

| Component | Thread Safe | Notes |
|-----------|-------------|-------|
| Status | Yes | Immutable |
| Types (Algorithm, KeyPermission) | Yes | Immutable |
| Key / KeySlotInfo | No | Mutable structs |
| IHsmApi / HsmApiImpl | No | External sync required |
| ICryptoService / CryptoService | No | External sync required |
| IKeystoreService / KeystoreService | No | External sync required |
| ISessionManager / SessionManager | No | External sync required |
| ICryptoAlgorithm implementations | Yes | Stateless |
| IUart implementations | Depends | Hardware-dependent |

### Recommendation for Thread Safety

```cpp
class ThreadSafeHsm {
public:
    ThreadSafeHsm(std::unique_ptr<api::IHsmApi> hsmApi)
        : hsmApi_(std::move(hsmApi)) {}
    
    Status encrypt(...) {
        std::lock_guard<std::mutex> lock(mutex_);
        return hsmApi_->encrypt(...);
    }
    
    Status decrypt(...) {
        std::lock_guard<std::mutex> lock(mutex_);
        return hsmApi_->decrypt(...);
    }
    
    // ... other methods
    
private:
    std::unique_ptr<api::IHsmApi> hsmApi_;
    mutable std::mutex mutex_;
};
```

---

## Memory Model

### Key Storage

```
┌─────────────────────────────────────────────────────────────┐
│                    Keystore Memory Layout                    │
└─────────────────────────────────────────────────────────────┘

KeySlot Storage (8 slots × sizeof(KeySlotStorage)):
┌──────────────────────────────────────────────────────────────┐
│ Slot 0                                                       │
│ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐ │
│ │ KeySlotInfo     │ │ Key (64 bytes)  │ │ accessCount     │ │
│ │ (24 bytes)      │ │ (72 bytes)      │ │ (4 bytes)       │ │
│ └─────────────────┘ └─────────────────┘ └─────────────────┘ │
├──────────────────────────────────────────────────────────────┤
│ Slot 1                                                       │
│ ...                                                          │
├──────────────────────────────────────────────────────────────┤
│ ...                                                          │
├──────────────────────────────────────────────────────────────┤
│ Slot 7                                                       │
│ ...                                                          │
└──────────────────────────────────────────────────────────────┘

Total: 8 × (24 + 72 + 4) = 800 bytes
```

### Session Storage

```
┌─────────────────────────────────────────────────────────────┐
│                   Session Memory Layout                      │
└─────────────────────────────────────────────────────────────┘

Session Array (4 sessions × sizeof(Session)):
┌──────────────────────────────────────────────────────────────┐
│ Session 0 │ Session 1 │ Session 2 │ Session 3               │
│ (12 bytes)│ (12 bytes)│ (12 bytes)│ (12 bytes)              │
└──────────────────────────────────────────────────────────────┘

Session struct:
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│ SessionId (4B)  │ │ isActive (1B)   │ │ jobCount (4B)   │   │
└─────────────────┘ └─────────────────┘ └─────────────────┘   │

Total: 4 × 12 = 48 bytes
```

---

## Configuration Constants

### From key_slot.hpp

```cpp
inline constexpr size_t kMaxKeySize = 64U;       // 512-bit max key
inline constexpr uint8_t kMaxKeySlots = 8U;      // 8 key slots
inline constexpr uint8_t kMaxSessions = 4U;      // 4 concurrent sessions
inline constexpr uint8_t kInvalidKeySlotId = 0xFFU;
inline constexpr SessionId kInvalidSessionId = 0xFFFFFFFFU;
inline constexpr JobId kInvalidJobId = 0xFFFFFFFFU;
```

### Memory Summary

| Component | Size | Notes |
|-----------|------|-------|
| Keystore | ~800 bytes | 8 key slots |
| Session Manager | ~48 bytes | 4 sessions |
| Total Static | ~850 bytes | Excludes code/stack |

---

## Extension Points

### Adding a New Algorithm

1. Create new class inheriting from `ICryptoAlgorithm`:
   ```cpp
   class ChaCha20Algorithm : public ICryptoAlgorithm {
       types::Algorithm getAlgorithmType() const override;
       size_t getKeySize() const override;
       size_t getBlockSize() const override;
       Status encrypt(...) override;
       Status decrypt(...) override;
   };
   ```

2. Implement the methods

3. Use with existing `CryptoService`:
   ```cpp
   auto algo = std::make_unique<ChaCha20Algorithm>();
   auto cryptoService = std::make_shared<CryptoService>(keystore, std::move(algo));
   ```

### Adding a New Service

1. Define interface:
   ```cpp
   class INewService {
       virtual Status init() = 0;
       virtual Status doSomething() = 0;
   };
   ```

2. Implement:
   ```cpp
   class NewService : public INewService { /* ... */ };
   ```

3. Inject into `HsmApiImpl`:
   ```cpp
   HsmApiImpl::HsmApiImpl(
       std::shared_ptr<mw::ISessionManager> sessionManager,
       std::shared_ptr<services::ICryptoService> cryptoService,
       std::shared_ptr<services::IKeystoreService> keystoreService,
       std::shared_ptr<services::INewService> newService);  // New
   ```

### Adding a New Platform

1. Create MCAL implementation:
   ```cpp
   class PlatformUart : public IUart { /* ... */ };
   ```

2. Update factory:
   ```cpp
   std::shared_ptr<mcal::IUart> createUart() {
       #ifdef PLATFORM_NEW
           return std::make_shared<PlatformUart>();
       // ...
   }
   ```

---

## Related Documentation

| Document | Description |
|----------|-------------|
| [README.md](README.md) | API overview |
| [types.md](types.md) | Core types |
| [api_layer.md](api_layer.md) | API layer details |
| [services.md](services.md) | Service layer details |
| [middleware.md](middleware.md) | Middleware details |
| [mcal.md](mcal.md) | MCAL details |

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024 | Initial architecture |
