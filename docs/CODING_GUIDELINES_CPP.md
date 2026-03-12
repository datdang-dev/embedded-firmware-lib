# Embedded HSM Firmware - C++ Coding Guidelines

**Document ID:** `HSM-CG-CPP-001`
**Version:** 1.0.0
**Last Updated:** 2026-03-12
**Applicable To:** All C++ source code in Embedded HSM Firmware
**Owner:** Embedded Engineering Team
**Compliance:** MISRA-C++:2008 Mindset, Automotive Security Standards

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [C++ Feature Restrictions](#2-c-feature-restrictions)
3. [Architecture Overview](#3-architecture-overview)
4. [Design Patterns](#4-design-patterns)
5. [Naming Conventions](#5-naming-conventions)
6. [Class Design Guidelines](#6-class-design-guidelines)
7. [Memory Management](#7-memory-management)
8. [Error Handling](#8-error-handling)
9. [Interface Design](#9-interface-design)
10. [Dependency Injection](#10-dependency-injection)
11. [Testing Guidelines](#11-testing-guidelines)
12. [Folder Structure](#12-folder-structure)

---

## 1. Introduction

### 1.1 Purpose

This document establishes C++ coding standards for the Embedded HSM Firmware project, combining:
- **Embedded safety** (no dynamic allocation in critical paths)
- **Modern C++ best practices** (RAII, type safety, interfaces)
- **Design patterns** (Strategy, Factory, Dependency Injection)
- **Hardware abstraction** (MCAL interfaces for portability)

### 1.2 Compliance Levels

| Level | Description | Enforcement |
|-------|-------------|-------------|
| **MUST** | Mandatory requirement | Enforced by static analysis (clang-tidy) |
| **SHOULD** | Strong recommendation | Enforced by code review |
| **MAY** | Optional guidance | Developer discretion |

---

## 2. C++ Feature Restrictions

### 2.1 Allowed Features ✅

| Feature | Usage | Example |
|---------|-------|---------|
| **Classes & Inheritance** | For interfaces and polymorphism | `class ICryptoService { virtual ... }` |
| **Virtual Functions** | For runtime polymorphism | `virtual Status encrypt() = 0;` |
| **Templates** | For type-safe containers | `std::array<T, N>` |
| **constexpr** | For compile-time computation | `constexpr size_t KeySize = 32;` |
| **enum class** | For type-safe enums | `enum class Algorithm { AES128, AES256 };` |
| **smart pointers (limited)** | For ownership in non-critical code | `std::unique_ptr<IService>` |
| **std::array** | For fixed-size arrays | `std::array<uint8_t, 32>` |
| **std::span** | For non-owning views | `std::span<const uint8_t> input` |
| **nullptr** | For null pointers | `ptr == nullptr` |

### 2.2 Prohibited Features ❌

| Feature | Reason | Alternative |
|---------|--------|-------------|
| **Exceptions** | Non-deterministic, code bloat | `Status` return type |
| **RTTI** (`dynamic_cast`, `typeid`) | Code bloat, runtime overhead | Virtual functions, Strategy pattern |
| **std::vector, std::string** | Dynamic allocation | `std::array`, `std::span` |
| **new/delete** | Fragmentation, non-deterministic | Static allocation, custom allocators |
| **try/catch** | See exceptions | Status codes |
| **Multiple inheritance** | Complexity | Single inheritance + interfaces |

### 2.3 C Compatibility for MCAL

**Rule:** MCAL layer MUST be written in C (extern "C") for hardware portability.

```cpp
// ehsm_mcal_uart.hpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef enum {
    EHSM_STATUS_OK = 0,
    EHSM_STATUS_ERR_INVALID_PARAM = -1
} ehsm_status_t;

ehsm_status_t ehsm_mcal_uartInit(void);
ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data);

#ifdef __cplusplus
}
#endif
```

---

## 3. Architecture Overview

### 3.1 Layer Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    API Layer (Facade)                       │
│              ehsm::api::IHsmApi (interface)                 │
│              ehsm::api::HsmApiImpl (implementation)         │
├─────────────────────────────────────────────────────────────┤
│                 Middleware Layer (Router)                   │
│         ehsm::mw::SessionManager, JobQueue                  │
├─────────────────────────────────────────────────────────────┤
│                  Service Layer (Business Logic)             │
│   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│   │ICryptoService│──▶│IKeystoreService│  │IStorageService│  │
│   │(uses other) │    │(shared)     │    │(shared)     │    │
│   └─────────────┘    └─────────────┘    └─────────────┘    │
├─────────────────────────────────────────────────────────────┤
│                  Driver Layer (Hardware Bridge)             │
│   ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│   │CryptoDriver │    │KeystoreDriver│   │StorageDriver│    │
│   │implements   │    │implements   │    │implements   │    │
│   │Service IF   │    │Service IF   │    │Service IF   │    │
│   └──────┬──────┘    └──────┬──────┘    └──────┬──────┘    │
├──────────┼──────────────────┼──────────────────┼───────────┤
│          │                  │                  │           │
│          ▼                  ▼                  ▼           │
│                MCAL Layer (C Interfaces)                    │
│          ┌──────────────────────────────────────┐          │
│          │  IMcalUart, IMcalStorage, IMcalCrypto │          │
│          └──────────────────────────────────────┘          │
└─────────────────────────────────────────────────────────────┘
           ▲
           │  Target implements MCAL interfaces
           │  (HOST stub or EMBEDDED hardware)
```

### 3.2 Dependency Rules

```
API → Middleware → Service → Driver → MCAL
       ↑              ↑         ↑
       └──────────────┴─────────┘
         Services can use each other
         (via interfaces, not concrete classes)
```

**Rule:** Dependencies always point downward. Services at the same layer depend on interfaces, not implementations.

---

## 4. Design Patterns

### 4.1 Strategy Pattern (Algorithm Selection)

**Use Case:** Different encryption algorithms (AES-128, AES-256, RSA)

```cpp
// Interface
class ICryptoAlgorithm {
public:
    virtual ~ICryptoAlgorithm() = default;
    virtual Status encrypt(std::span<const uint8_t> input,
                          std::span<uint8_t> output) = 0;
    virtual Status decrypt(std::span<const uint8_t> input,
                          std::span<uint8_t> output) = 0;
    virtual size_t getKeySize() const = 0;
};

// Concrete strategies
class Aes128Algorithm : public ICryptoAlgorithm {
    // ... implementation
};

class Aes256Algorithm : public ICryptoAlgorithm {
    // ... implementation
};

// Context (uses strategy)
class CryptoService {
public:
    explicit CryptoService(std::unique_ptr<ICryptoAlgorithm> algorithm)
        : algorithm_(std::move(algorithm)) {}
    
    Status encrypt(std::span<const uint8_t> input,
                  std::span<uint8_t> output) {
        return algorithm_->encrypt(input, output);
    }

private:
    std::unique_ptr<ICryptoAlgorithm> algorithm_;
};
```

### 4.2 Factory Pattern (Object Creation)

**Use Case:** Creating algorithm instances based on configuration

```cpp
class CryptoAlgorithmFactory {
public:
    static std::unique_ptr<ICryptoAlgorithm> create(AlgorithmType type);
};

// Usage
auto algo = CryptoAlgorithmFactory::create(AlgorithmType::AES256);
auto service = std::make_unique<CryptoService>(std::move(algo));
```

### 4.3 Dependency Injection (Constructor Injection)

**Use Case:** Injecting service dependencies

```cpp
// BAD: Hard-coded dependency
class CryptoService {
    KeystoreService keystore_;  // ❌ Concrete class
};

// GOOD: Injected interface
class CryptoService {
public:
    CryptoService(std::shared_ptr<IKeystoreService> keystore,
                  std::unique_ptr<ICryptoAlgorithm> algorithm)
        : keystore_(std::move(keystore))
        , algorithm_(std::move(algorithm)) {}

private:
    std::shared_ptr<IKeystoreService> keystore_;
    std::unique_ptr<ICryptoAlgorithm> algorithm_;
};
```

### 4.4 Singleton Pattern (Controlled Global Access)

**Use Case:** Single instance services (Keystore)

```cpp
class KeystoreService {
public:
    static KeystoreService& getInstance() {
        static KeystoreService instance;
        return instance;
    }
    
    // Delete copy/move
    KeystoreService(const KeystoreService&) = delete;
    KeystoreService& operator=(const KeystoreService&) = delete;

private:
    KeystoreService() = default;
};
```

### 4.5 Observer Pattern (Event System)

**Use Case:** Notifying listeners of HSM events

```cpp
class IHsmEventListener {
public:
    virtual ~IHsmEventListener() = default;
    virtual void onEncryptionComplete(Status status) = 0;
    virtual void onKeyImported(uint8_t keySlotId) = 0;
};

class HsmEventDispatcher {
public:
    void addListener(std::weak_ptr<IHsmEventListener> listener);
    void removeListener(std::weak_ptr<IHsmEventListener> listener);
    void notifyEncryptionComplete(Status status);
};
```

---

## 5. Naming Conventions

### 5.1 General Naming

| Entity | Convention | Example |
|--------|------------|---------|
| Namespaces | `snake_case`, hierarchical | `ehsm::api`, `ehsm::services` |
| Classes | `PascalCase` | `CryptoService`, `HsmApi` |
| Interfaces | `I` prefix + `PascalCase` | `ICryptoService`, `IKeystore` |
| Functions | `camelCase` | `encryptData()`, `getKeySlot()` |
| Variables | `snake_case` with descriptive name | `key_slot_id`, `ciphertext_len` |
| Member variables | `snake_case_` with trailing underscore | `key_slot_id_`, `buffer_` |
| Constants | `kPascalCase` | `kMaxKeySlots`, `kBlockSize` |
| Enum values | `kPascalCase` | `kAes128`, `kAes256` |
| Type aliases | `snake_case_t` | `status_t`, `key_id_t` |

### 5.2 File Naming

```
include/ehsm/
├── api/
│   ├── ihsm_api.hpp           # Interface
│   └── hsm_api_impl.hpp       # Implementation
├── services/
│   ├── icrypto_service.hpp
│   ├── crypto_service.hpp
│   └── ikeystore_service.hpp
└── mcal/
    ├── imcal_uart.hpp         # C interface wrapper
    └── mcal_uart_stub.hpp     # HOST stub

src/
├── api/
│   └── hsm_api_impl.cpp
├── services/
│   ├── crypto_service.cpp
│   └── keystore_service.cpp
└── mcal/
    └── mcal_uart_stub.cpp
```

### 5.3 Namespace Structure

```cpp
namespace ehsm {

namespace api {
    class IHsmApi;
    class HsmApiImpl;
}

namespace mw {
    class SessionManager;
    class JobQueue;
}

namespace services {
    class ICryptoService;
    class CryptoService;
    class IKeystoreService;
    class KeystoreService;
}

namespace drivers {
    class CryptoDriver;
    class KeystoreDriver;
}

namespace mcal {
    // C interfaces wrapped in extern "C"
}

namespace types {
    enum class Status;
    enum class Algorithm;
    struct KeySlot;
}

} // namespace ehsm
```

---

## 6. Class Design Guidelines

### 6.1 Class Structure

```cpp
// ehsm_crypto_service.hpp
#pragma once

#include "ehsm/services/icrypto_service.hpp"
#include "ehsm/types/status.hpp"
#include <memory>
#include <span>

namespace ehsm::services {

/**
 * @brief CryptoService implements symmetric encryption operations.
 * 
 * This class uses the Strategy pattern to support multiple algorithms.
 * Dependencies are injected via constructor.
 */
class CryptoService final : public ICryptoService {
public:
    // === Constructors ===
    explicit CryptoService(
        std::shared_ptr<IKeystoreService> keystore,
        std::unique_ptr<ICryptoAlgorithm> algorithm);
    
    // === Rule of Five (delete if non-copyable) ===
    CryptoService(const CryptoService&) = delete;
    CryptoService& operator=(const CryptoService&) = delete;
    CryptoService(CryptoService&&) noexcept = default;
    CryptoService& operator=(CryptoService&&) noexcept = default;
    ~CryptoService() override = default;

    // === Interface Implementation ===
    [[nodiscard]] Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) override;
    
    [[nodiscard]] Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) override;

    // === Public API ===
    [[nodiscard]] AlgorithmType getAlgorithmType() const;

private:
    // === Member Variables (alphabetical, trailing _) ===
    std::unique_ptr<ICryptoAlgorithm> algorithm_;
    std::shared_ptr<IKeystoreService> keystore_;
};

} // namespace ehsm::services
```

### 6.2 Interface Design

**Rule:** Interfaces must be pure abstract classes (no data members, no implementation).

```cpp
// icrypto_service.hpp
#pragma once

#include "ehsm/types/status.hpp"
#include <span>

namespace ehsm::services {

/**
 * @brief Interface for cryptographic operations.
 * 
 * All crypto service implementations must inherit from this interface.
 * This enables dependency injection and testing with mocks.
 */
class ICryptoService {
public:
    virtual ~ICryptoService() = default;

    /**
     * @brief Encrypt data using the configured algorithm.
     * @param input Plaintext input
     * @param output Ciphertext output (must be >= input.size())
     * @param keySlotId Key slot containing encryption key
     * @return Status::OK on success
     */
    [[nodiscard]] virtual Status encrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) = 0;

    /**
     * @brief Decrypt data using the configured algorithm.
     * @param input Ciphertext input
     * @param output Plaintext output (must be >= input.size())
     * @param keySlotId Key slot containing decryption key
     * @return Status::OK on success
     */
    [[nodiscard]] virtual Status decrypt(
        std::span<const uint8_t> input,
        std::span<uint8_t> output,
        uint8_t keySlotId) = 0;
};

} // namespace ehsm::services
```

### 6.3 Member Initialization

**Rule:** Use member initializer lists, not assignment in constructor body.

```cpp
// ✅ GOOD
CryptoService::CryptoService(
    std::shared_ptr<IKeystoreService> keystore,
    std::unique_ptr<ICryptoAlgorithm> algorithm)
    : algorithm_(std::move(algorithm))
    , keystore_(std::move(keystore)) {}

// ❌ BAD
CryptoService::CryptoService(...) {
    algorithm_ = std::move(algorithm);  // Assignment, not initialization
    keystore_ = std::move(keystore);
}
```

---

## 7. Memory Management

### 7.1 Allocation Strategy

| Context | Allocation Method |
|---------|-------------------|
| **Static/Global** | Static arrays, `std::array` |
| **Stack** | Local variables, `std::array` |
| **Heap (limited)** | `std::unique_ptr`, `std::shared_ptr` |
| **ISR/Real-time** | Static only, no heap |

### 7.2 RAII Pattern

**Rule:** Resources must be managed via RAII (Resource Acquisition Is Initialization).

```cpp
// ✅ GOOD: RAII with unique_ptr
class SessionManager {
public:
    SessionManager() 
        : sessionPool_(std::make_unique<Session[]>(kMaxSessions)) {}
    
    // No manual delete needed

private:
    std::unique_ptr<Session[]> sessionPool_;
};

// ✅ GOOD: Custom deleter for C resources
struct CBufferDeleter {
    void operator()(uint8_t* ptr) const {
        secureZero(ptr, kBufferSize);  // Secure erase before free
        free(ptr);
    }
};

using SecureBuffer = std::unique_ptr<uint8_t[], CBufferDeleter>;
```

### 7.3 std::span for Views

**Rule:** Use `std::span` for non-owning buffer views.

```cpp
// ✅ GOOD: span for buffer parameters
Status encrypt(std::span<const uint8_t> input,
               std::span<uint8_t> output);

// ❌ BAD: raw pointers
Status encrypt(const uint8_t* input, size_t inputLen,
               uint8_t* output, size_t outputSize);
```

---

## 8. Error Handling

### 8.1 Status Class

**Rule:** Use `Status` class instead of exceptions.

```cpp
// status.hpp
#pragma once

#include <system_error>

namespace ehsm::types {

enum class StatusCode {
    OK = 0,
    ERR_INVALID_PARAM,
    ERR_INSUFFICIENT_BUFFER,
    ERR_CRYPTO_FAILED,
    // ...
};

/**
 * @brief Status class for error handling (no exceptions).
 */
class Status {
public:
    Status() noexcept : code_(StatusCode::OK) {}
    explicit Status(StatusCode code) noexcept : code_(code) {}
    
    [[nodiscard]] bool isOk() const noexcept { return code_ == StatusCode::OK; }
    [[nodiscard]] explicit operator bool() const noexcept { return isOk(); }
    
    [[nodiscard]] const char* message() const;

private:
    StatusCode code_;
};

// Usage
Status encrypt(...) {
    if (input.empty()) {
        return Status(StatusCode::ERR_INVALID_PARAM);
    }
    return Status(StatusCode::OK);
}

// Check
Status s = encrypt(...);
if (!s.isOk()) {
    handleError(s);
}
```

### 8.2 Error Propagation

**Rule:** Propagate errors up the call stack. Handle only where recovery is possible.

```cpp
Status CryptoService::encrypt(...) {
    // Validate
    if (input.empty()) {
        return Status(StatusCode::ERR_INVALID_PARAM);
    }
    
    // Get key (propagate error)
    auto keyResult = keystore_->getKey(keySlotId);
    if (!keyResult.isOk()) {
        return keyResult;  // Propagate
    }
    
    // Encrypt
    return algorithm_->encrypt(input, output);
}
```

---

## 9. Interface Design

### 9.1 MCAL Interface Abstraction

**Rule:** MCAL interfaces must be pure C for hardware portability.

```cpp
// imcal_uart.hpp
#pragma once

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EHSM_STATUS_OK = 0,
    EHSM_STATUS_ERR_INVALID_PARAM = -1
} ehsm_status_t;

ehsm_status_t ehsm_mcal_uartInit(void);
ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data);
ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif

// C++ wrapper (optional, for OOP code)
namespace ehsm::mcal {

class IUart {
public:
    virtual ~IUart() = default;
    virtual Status init() = 0;
    virtual Status txByte(uint8_t data) = 0;
    virtual Status txBuffer(std::span<const uint8_t> data) = 0;
};

} // namespace ehsm::mcal
```

### 9.2 Service Interface Composition

**Rule:** Services depend on interfaces, not concrete classes.

```cpp
// ❌ BAD: Depends on concrete class
class CryptoService {
    KeystoreService keystore_;  // Hard dependency
};

// ✅ GOOD: Depends on interface
class CryptoService {
public:
    CryptoService(std::shared_ptr<IKeystoreService> keystore)
        : keystore_(std::move(keystore)) {}

private:
    std::shared_ptr<IKeystoreService> keystore_;
};
```

---

## 10. Dependency Injection

### 10.1 Composition Root

**Rule:** Wire dependencies at application entry point (composition root).

```cpp
// main.cpp
int main() {
    // Create MCAL (lowest layer)
    auto uart = std::make_unique<mcal::UartStub>();
    
    // Create Drivers
    auto storageDriver = std::make_unique<drivers::StorageDriver>();
    
    // Create Services (with dependencies)
    auto keystoreService = std::make_shared<services::KeystoreService>();
    auto cryptoAlgorithm = std::make_unique<services::Aes256Algorithm>();
    auto cryptoService = std::make_unique<services::CryptoService>(
        keystoreService, std::move(cryptoAlgorithm));
    
    // Create Middleware
    auto sessionManager = std::make_unique<mw::SessionManager>();
    
    // Create API (Facade)
    auto hsmApi = std::make_unique<api::HsmApiImpl>(
        std::move(sessionManager),
        std::move(cryptoService),
        std::move(keystoreService));
    
    // Use API
    hsmApi->init();
    // ...
    
    return 0;
}
```

### 10.2 Service Locator (Optional)

**Rule:** Use Service Locator sparingly (prefer constructor injection).

```cpp
class ServiceLocator {
public:
    static ServiceLocator& getInstance() {
        static ServiceLocator instance;
        return instance;
    }
    
    template<typename T>
    void registerService(std::shared_ptr<T> service) {
        services_[typeid(T).name()] = service;
    }
    
    template<typename T>
    std::shared_ptr<T> getService() {
        return std::static_pointer_cast<T>(services_[typeid(T).name()]);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<void>> services_;
};
```

---

## 11. Testing Guidelines

### 11.1 Testing Framework

**Rule:** Use Tromploe (trompeloeil) for mocking with Catch2.

```cpp
// test_crypto_service.cpp
#include <catch2/catch.hpp>
#include <trompeloeil.hpp>
#include "ehsm/services/crypto_service.hpp"

// Mock interface
class MockKeystoreService : public ehsm::services::IKeystoreService {
    MAKE_MOCK2(getKey, ehsm::Status(uint8_t, ehsm::types::Key&), override);
    MAKE_MOCK1(deleteKey, ehsm::Status(uint8_t), override);
};

TEST_CASE("CryptoService encrypts data", "[crypto]") {
    // Arrange
    auto mockKeystore = std::make_shared<MockKeystoreService>();
    auto algorithm = std::make_unique<ehsm::services::Aes256Algorithm>();
    
    REQUIRE_CALL(*mockKeystore, getKey(0, _))
        .LR_RETURN(ehsm::Status(ehsm::StatusCode::OK));
    
    auto cryptoService = std::make_unique<ehsm::services::CryptoService>(
        mockKeystore, std::move(algorithm));
    
    // Act
    std::array<uint8_t, 16> plaintext = {0};
    std::array<uint8_t, 16> ciphertext = {0};
    auto status = cryptoService->encrypt(plaintext, ciphertext, 0);
    
    // Assert
    REQUIRE(status.isOk());
}
```

### 11.2 Test Organization

```
tests/
├── CMakeLists.txt
├── main.cpp                  # Catch2 main
├── test_crypto_service.cpp
├── test_keystore_service.cpp
├── test_session_manager.cpp
└── mocks/
    ├── mock_keystore.hpp
    ├── mock_crypto.hpp
    └── mock_mcal.hpp
```

---

## 12. Folder Structure

### 12.1 Proposed Structure

```
embedded_hsm/
├── CMakeLists.txt
├── main.cpp
│
├── include/ehsm/             # Public headers
│   ├── api/
│   │   ├── ihsm_api.hpp
│   │   └── hsm_api.hpp
│   └── types/
│       ├── status.hpp
│       ├── algorithm.hpp
│       └── key_slot.hpp
│
├── src/
│   ├── api/
│   │   ├── CMakeLists.txt
│   │   ├── hsm_api_impl.hpp
│   │   └── hsm_api_impl.cpp
│   │
│   ├── middleware/
│   │   ├── CMakeLists.txt
│   │   ├── session_manager.hpp
│   │   ├── session_manager.cpp
│   │   └── job_queue.hpp
│   │
│   ├── services/
│   │   ├── CMakeLists.txt
│   │   ├── crypto/
│   │   │   ├── icrypto_service.hpp
│   │   │   ├── crypto_service.hpp
│   │   │   ├── crypto_service.cpp
│   │   │   └── algorithms/
│   │   │       ├── icrypto_algorithm.hpp
│   │   │       ├── aes128_algorithm.hpp
│   │   │       └── aes256_algorithm.hpp
│   │   └── keystore/
│   │       ├── ikeystore_service.hpp
│   │       ├── keystore_service.hpp
│   │       └── keystore_service.cpp
│   │
│   ├── drivers/
│   │   ├── CMakeLists.txt
│   │   ├── crypto_driver.hpp
│   │   ├── crypto_driver.cpp
│   │   └── storage_driver.hpp
│   │
│   └── mcal/
│       ├── CMakeLists.txt
│       ├── imcal_uart.hpp
│       ├── mcal_uart_stub.hpp    # HOST
│       ├── mcal_uart_stub.cpp
│       └── mcal_uart_real.c      # EMBEDDED (C code)
│
├── tests/
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── test_crypto_service.cpp
│   ├── test_keystore_service.cpp
│   └── mocks/
│       ├── mock_keystore.hpp
│       └── mock_crypto.hpp
│
├── cmake/
│   ├── CompilerWarnings.cmake
│   └── PlatformConfig.cmake
│
└── docs/
    ├── CODING_GUIDELINES_CPP.md
    ├── ARCHITECTURE_DESIGN.md
    └── folder-structure-proposal.md
```

---

## Appendix A: Quick Reference

```cpp
// Namespace
namespace ehsm::services { }

// Interface
class IService {
    virtual Status doWork() = 0;
};

// Implementation
class ServiceImpl final : public IService {
public:
    explicit ServiceImpl(std::shared_ptr<IDependency> dep);
    Status doWork() override;
private:
    std::shared_ptr<IDependency> dep_;
};

// Status handling
Status result = service.doWork();
if (!result.isOk()) { handleError(result); }

// Span for buffers
void process(std::span<const uint8_t> input,
             std::span<uint8_t> output);

// Smart pointers
std::unique_ptr<IService> service;        // Exclusive ownership
std::shared_ptr<IDependency> dependency;  // Shared ownership
std::weak_ptr<IListener> listener;        // Non-owning reference
```

---

**Document End**
