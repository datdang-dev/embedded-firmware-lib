# Embedded HSM C++ API Reference

## Overview

The Embedded HSM (Hardware Security Module) firmware provides a secure cryptographic subsystem for embedded devices. This documentation covers the public API headers and their usage.

**Version:** 1.0.0  
**Copyright:** Copyright (c) 2024 Embedded HSM Project  
**License:** MIT License

## Architecture Layers

The HSM firmware is organized into the following layers:

```
┌─────────────────────────────────────────────────────────────┐
│                      API Layer                               │
│  ┌─────────────────┐  ┌─────────────────────────────────┐   │
│  │  ihsm_api.hpp   │  │         hsm_api_impl.hpp        │   │
│  │  (Interface)    │  │         (Implementation)        │   │
│  └─────────────────┘  └─────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    Service Layer                             │
│  ┌────────────┐ ┌──────────────┐ ┌───────────────────────┐  │
│  │ ICrypto    │ │ IKeystore    │ │ ICryptoAlgorithm      │  │
│  │ Service    │ │ Service      │ │ (Strategy Interface)  │  │
│  └────────────┘ └──────────────┘ └───────────────────────┘  │
│  ┌────────────┐ ┌──────────────┐ ┌───────────────────────┐  │
│  │ Crypto     │ │ Keystore     │ │ AES128/AES256         │  │
│  │ Service    │ │ Service      │ │ (Concrete Strategies) │  │
│  └────────────┘ └──────────────┘ └───────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                   Middleware Layer                           │
│  ┌─────────────────────┐  ┌─────────────────────────────┐   │
│  │ ISessionManager     │  │ SessionManager              │   │
│  │ (Interface)         │  │ (Implementation)            │   │
│  └─────────────────────┘  └─────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                      MCAL Layer                              │
│  ┌─────────────────────┐  ┌─────────────────────────────┐   │
│  │ IUart               │  │ UartStub                    │   │
│  │ (Interface)         │  │ (HOST Implementation)       │   │
│  └─────────────────────┘  └─────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Documentation Sections

| Section | Description |
|---------|-------------|
| [Architecture](architecture.md) | System architecture and design patterns |
| [Core Types](types.md) | Common types, status codes, algorithms, key slots |
| [API Layer](api_layer.md) | Public HSM API interface and implementation |
| [Services](services.md) | Crypto and keystore services |
| [Middleware](middleware.md) | Session management |
| [MCAL](mcal.md) | Hardware abstraction layer |

## Quick Start

```cpp
#include "ihsm_api.hpp"
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
    
    // Create HSM API
    auto hsmApi = std::make_unique<api::HsmApiImpl>(
        sessionManager, cryptoService, keystore);
    
    // Initialize
    hsmApi->init();
    
    // Create session
    types::SessionId sessionId;
    hsmApi->createSession(sessionId);
    
    // Import key
    std::array<uint8_t, 16> keyData = { /* 128-bit key */ };
    hsmApi->importKey(0, types::Algorithm::AES_128, keyData, 
                      types::KeyPermission::ENCRYPT | types::KeyPermission::DECRYPT);
    
    // Encrypt data
    std::array<uint8_t, 32> plaintext = { /* data */ };
    std::array<uint8_t, 32> ciphertext;
    hsmApi->encrypt(sessionId, 0, types::Algorithm::AES_128, 
                    plaintext, ciphertext);
    
    return 0;
}
```

## Design Patterns

The HSM firmware utilizes several design patterns:

| Pattern | Usage |
|---------|-------|
| **Facade** | `HsmApiImpl` provides unified interface to subsystem |
| **Strategy** | `ICryptoAlgorithm` allows interchangeable encryption algorithms |
| **Dependency Injection** | All services receive dependencies via constructor |
| **Interface Segregation** | Separate interfaces for each service layer |

## Thread Safety

- **API Layer:** Not thread-safe by default. External synchronization required.
- **Services:** Individual services are not thread-safe.
- **Session Manager:** Maintains session state; external synchronization required for concurrent access.

## Error Handling

All API methods return a `Status` object instead of throwing exceptions. This approach:

- Avoids C++ exceptions (not suitable for embedded systems)
- Provides type-safe error handling
- Enables explicit error checking

```cpp
Status result = hsmApi->encrypt(sessionId, keySlotId, algorithm, input, output);
if (!result.isOk()) {
    // Handle error
    handleError(result.code(), result.message());
}
```

## Supported Algorithms

| Algorithm | Key Size | Block Size | Status |
|-----------|----------|------------|--------|
| AES-128 | 16 bytes | 16 bytes | Implemented |
| AES-256 | 32 bytes | 16 bytes | Implemented |
| RSA-2048 | 256 bytes | N/A | Defined |
| RSA-4096 | 512 bytes | N/A | Defined |
| ECC-P256 | 32 bytes | N/A | Defined |
| SHA-256 | N/A | 32 bytes | Defined |
| ChaCha20 | 32 bytes | Stream | Defined |

## Key Management

- **Maximum Key Slots:** 8
- **Maximum Sessions:** 4
- **Maximum Key Size:** 64 bytes (512 bits)

Keys are stored securely and never exposed outside the keystore boundary. Each key slot supports permission flags for fine-grained access control.

## Related Documentation

- [Architecture Design](../architecture.md) - Detailed system design
- [Coding Guidelines](../CODING_GUIDELINES_CPP.md) - C++ coding standards
- [Doxygen Configuration](../Doxyfile.in) - Documentation generation settings
