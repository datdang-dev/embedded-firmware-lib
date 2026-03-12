# Embedded HSM Firmware - Architecture Design

**Document ID:** `HSM-ARCH-001`
**Version:** 1.0.0
**Last Updated:** 2026-03-12
**Applicable To:** Embedded HSM Firmware C++ Implementation
**Owner:** Embedded Engineering Team

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Layer Architecture](#2-layer-architecture)
3. [Component Diagram](#3-component-diagram)
4. [Class Diagrams](#4-class-diagrams)
5. [Sequence Diagrams](#5-sequence-diagrams)
6. [Dependency Graph](#6-dependency-graph)
7. [Design Patterns](#7-design-patterns)
8. [MCAL Abstraction](#8-mcal-abstraction)
9. [Extension Points](#9-extension-points)

---

## 1. Architecture Overview

### 1.1 Design Goals

| Goal | Description |
|------|-------------|
| **Portability** | Any MCU can be supported by implementing MCAL interfaces |
| **Testability** | All layers can be tested in isolation with mocks |
| **Maintainability** | Clear separation of concerns, SOLID principles |
| **Security** | Key material never exposed, secure erasure |
| **Performance** | Zero-copy where possible, deterministic execution |

### 1.2 Architectural Principles

```
┌─────────────────────────────────────────────────────────────┐
│  PRINCIPLES:                                                │
│  1. Dependency Inversion: High-level doesn't depend on      │
│     low-level. Both depend on abstractions.                 │
│  2. Interface Segregation: Many specific interfaces > one   │
│     general interface.                                      │
│  3. Single Responsibility: Each class has one reason to     │
│     change.                                                 │
│  4. Open/Closed: Open for extension, closed for            │
│     modification.                                           │
│  5. Liskov Substitution: Derived classes must be            │
│     substitutable for base classes.                         │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Layer Architecture

### 2.1 Layer Breakdown

```
┌─────────────────────────────────────────────────────────────┐
│  Layer 1: API Layer (Facade)                                │
│  ─────────────────────────────────────────────────────────  │
│  Purpose: Simple, unified interface for application code    │
│  Components: IHsmApi, HsmApiImpl                            │
│  Dependencies: Middleware layer interfaces                  │
├─────────────────────────────────────────────────────────────┤
│  Layer 2: Middleware Layer (Router)                         │
│  ─────────────────────────────────────────────────────────  │
│  Purpose: Session management, job queuing, routing          │
│  Components: SessionManager, JobQueue                       │
│  Dependencies: Service layer interfaces                     │
├─────────────────────────────────────────────────────────────┤
│  Layer 3: Service Layer (Business Logic)                    │
│  ─────────────────────────────────────────────────────────  │
│  Purpose: Core business logic, crypto operations            │
│  Components: CryptoService, KeystoreService, StorageService │
│  Dependencies: Service interfaces (can use each other)      │
├─────────────────────────────────────────────────────────────┤
│  Layer 4: Driver Layer (Hardware Bridge)                    │
│  ─────────────────────────────────────────────────────────  │
│  Purpose: Bridge between services and hardware              │
│  Components: CryptoDriver, KeystoreDriver, StorageDriver    │
│  Dependencies: MCAL interfaces                              │
├─────────────────────────────────────────────────────────────┤
│  Layer 5: MCAL Layer (Hardware Abstraction)                 │
│  ─────────────────────────────────────────────────────────  │
│  Purpose: Hardware-specific code (C language)               │
│  Components: IMcalUart, IMcalStorage, IMcalCrypto           │
│  Dependencies: None (hardware registers)                    │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Component Diagram

```plantuml
@startuml Component Diagram - Embedded HSM Firmware
skinparam componentStyle rectangle
skinparam packageStyle rectangle

package "Application Layer" {
  [User Application] as App
}

package "API Layer (Facade)" {
  component "IHsmApi\n(interface)" as IHsmApi
  component "HsmApiImpl\n(facade)" as HsmApi
}

package "Middleware Layer" {
  component "SessionManager" as SessionMgr
  component "JobQueue" as JobQ
}

package "Service Layer" {
  package "Crypto Services" {
    component "ICryptoService\n(interface)" as ICrypto
    component "CryptoService\n(implementation)" as CryptoSvc
    component "ICryptoAlgorithm\n(strategy)" as ICryptoAlgo
  }
  
  package "Keystore Services" {
    component "IKeystoreService\n(interface)" as IKey
    component "KeystoreService\n(implementation)" as KeySvc
  }
  
  package "Storage Services" {
    component "IStorageService\n(interface)" as IStorage
    component "StorageService\n(implementation)" as StorageSvc
  }
}

package "Driver Layer" {
  component "CryptoDriver" as CryptoDrv
  component "KeystoreDriver" as KeyDrv
  component "StorageDriver" as StorageDrv
}

package "MCAL Layer (C Interfaces)" {
  component "IMcalUart\n(C interface)" as MUart
  component "IMcalStorage\n(C interface)" as MStorage
  component "IMcalCrypto\n(C interface)" as MCrypto
}

package "Platform Implementations" {
  component "HOST: UartStub" as UartStub
  component "EMBEDDED: UartReal" as UartReal
  component "HOST: StorageStub" as StorageStub
  component "EMBEDDED: FlashDriver" as Flash
}

' Application to API
App --> IHsmApi : uses
IHsmApi <|-- HsmApi : implements

' API to Middleware
HsmApi --> SessionMgr : uses
HsmApi --> JobQ : uses

' Middleware to Services
SessionMgr --> ICrypto : uses
SessionMgr --> IKeystore : uses

' Service relationships
CryptoSvc ..> IKey : depends on (DI)
CryptoSvc o-- ICryptoAlgo : strategy

' Services to Drivers
ICrypto <|-- CryptoSvc : implements
IKey <|-- KeySvc : implements
IStorage <|-- StorageSvc : implements

CryptoSvc --> CryptoDrv : uses
KeySvc --> KeyDrv : uses
StorageSvc --> StorageDrv : uses

' Drivers to MCAL
CryptoDrv --> MCrypto : uses
KeyDrv --> MStorage : uses
StorageDrv --> MStorage : uses

' MCAL to Platform
MCrypto <|-- UartStub : HOST
MCrypto <|-- UartReal : EMBEDDED
MStorage <|-- StorageStub : HOST
MStorage <|-- Flash : EMBEDDED

note right of ICryptoAlgo
  Strategy Pattern
  - Aes128Algorithm
  - Aes256Algorithm
  - RsaAlgorithm
end note

note right of SessionMgr
  State Pattern
  - Session states
  - Job queue management
end note

@enduml
```

---

## 4. Class Diagrams

### 4.1 API Layer Class Diagram

```plantuml
@startuml Class Diagram - API Layer
skinparam classAttributeIconSize 0

package "ehsm::api" {
  abstract class "IHsmApi" {
    +virtual ~IHsmApi() = 0
    +virtual Status init() = 0
    +virtual Status deinit() = 0
    +virtual Status createSession(SessionId& id) = 0
    +virtual Status closeSession(SessionId id) = 0
    +virtual Status encrypt(SessionId id, uint8_t keySlotId, Algorithm algo, span<const uint8_t> input, span<uint8_t> output) = 0
    +virtual Status decrypt(SessionId id, uint8_t keySlotId, Algorithm algo, span<const uint8_t> input, span<uint8_t> output) = 0
    +virtual Status importKey(uint8_t keySlotId, Algorithm algo, span<const uint8_t> keyData, uint8_t permissions) = 0
    +virtual Status deleteKey(uint8_t keySlotId) = 0
  }
  
  class "HsmApiImpl" {
    -sessionManager_: std::shared_ptr<mw::ISessionManager>
    -cryptoService_: std::shared_ptr<services::ICryptoService>
    -keystoreService_: std::shared_ptr<services::IKeystoreService>
    -isInitialized_: bool
    +HsmApiImpl(sessionMgr, cryptoSvc, keystoreSvc)
    +init() Status override
    +deinit() Status override
    +createSession(id) Status override
    +closeSession(id) Status override
    +encrypt(...) Status override
    +decrypt(...) Status override
    +importKey(...) Status override
    +deleteKey(slotId) Status override
    -validateSession(id) Status
  }
}

package "ehsm::types" {
  class "Status" {
    -code_: StatusCode
    +Status()
    +Status(code)
    +isOk() bool
    +message() const char*
  }
  
  enum "StatusCode" {
    OK
    ERR_INVALID_PARAM
    ERR_INSUFFICIENT_BUFFER
    ERR_CRYPTO_FAILED
    ERR_SESSION_INVALID
    ERR_NOT_INITIALIZED
  }
  
  enum "Algorithm" {
    AES_128
    AES_256
    RSA_2048
    ECC_P256
    SHA_256
  }
}

IHsmApi <|-- HsmApiImpl : implements
HsmApiImpl --> Status : returns
HsmApiImpl --> Algorithm : uses

@enduml
```

### 4.2 Service Layer Class Diagram

```plantuml
@startuml Class Diagram - Service Layer
skinparam classAttributeIconSize 0

package "ehsm::services" {
  abstract class "ICryptoService" {
    +virtual ~ICryptoService() = 0
    +virtual Status encrypt(span<const uint8_t> input, span<uint8_t> output, uint8_t keySlotId) = 0
    +virtual Status decrypt(span<const uint8_t> input, span<uint8_t> output, uint8_t keySlotId) = 0
  }
  
  class "CryptoService" {
    -keystore_: std::shared_ptr<IKeystoreService>
    -algorithm_: std::unique_ptr<ICryptoAlgorithm>
    +CryptoService(keystore, algorithm)
    +encrypt(input, output, keySlotId) Status override
    +decrypt(input, output, keySlotId) Status override
    +getAlgorithmType() AlgorithmType
  }
  
  abstract class "IKeystoreService" {
    +virtual ~IKeystoreService() = 0
    +virtual Status importKey(slotId, algo, keyData, permissions) = 0
    +virtual Status deleteKey(slotId) = 0
    +virtual Status getKey(slotId, Key& key) = 0
    +virtual Status getSlotInfo(slotId, info) = 0
    +virtual Status clearAll() = 0
  }
  
  class "KeystoreService" {
    -keySlots_: std::array<KeySlot, kMaxKeySlots>
    -isInitialized_: bool
    +importKey(slotId, algo, keyData, permissions) Status override
    +deleteKey(slotId) Status override
    +getKey(slotId, key) Status override
    +getSlotInfo(slotId, info) Status override
    +clearAll() Status override
    -secureClear(slotId) void
    -isValidSlotId(slotId) bool
  }
  
  abstract class "ICryptoAlgorithm" {
    +virtual ~ICryptoAlgorithm() = 0
    +virtual Status encrypt(input, output) = 0
    +virtual Status decrypt(input, output) = 0
    +virtual getKeySize() size_t = 0
  }
  
  class "Aes128Algorithm" {
    +encrypt(input, output) Status override
    +decrypt(input, output) Status override
    +getKeySize() size_t override
    -encryptBlock(input, output, key) void
  }
  
  class "Aes256Algorithm" {
    +encrypt(input, output) Status override
    +decrypt(input, output) Status override
    +getKeySize() size_t override
  }
}

package "ehsm::drivers" {
  class "CryptoDriver" {
    +encrypt(algo, key, input, output) Status
    +decrypt(algo, key, input, output) Status
  }
}

ICryptoService <|-- CryptoService : implements
IKeystoreService <|-- KeystoreService : implements
ICryptoAlgorithm <|-- Aes128Algorithm : implements
ICryptoAlgorithm <|-- Aes256Algorithm : implements

CryptoService --> IKeystoreService : depends on (DI)
CryptoService o-- ICryptoAlgorithm : strategy pattern
CryptoService --> CryptoDriver : uses
KeystoreService --> CryptoDriver : uses (for secure RNG)

note right of CryptoService
  Uses Strategy Pattern
  Algorithm injected at runtime
end note

note right of KeystoreService
  Singleton Pattern (optional)
  Secure key storage
end note

@enduml
```

### 4.3 MCAL Layer Class Diagram

```plantuml
@startuml Class Diagram - MCAL Layer
skinparam classAttributeIconSize 0

package "ehsm::mcal (C Interfaces)" {
  artifact "imcal_uart.h" {
    [ehsm_status_t ehsm_mcal_uartInit(void)]
    [ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data)]
    [ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t*, size_t)]
  }
  
  artifact "imcal_storage.h" {
    [ehsm_status_t ehsm_mcal_storageInit(void)]
    [ehsm_status_t ehsm_mcal_storageRead(addr, data, len)]
    [ehsm_status_t ehsm_mcal_storageWrite(addr, data, len)]
    [ehsm_status_t ehsm_mcal_storageEraseSector(addr)]
  }
}

package "ehsm::mcal::cpp (C++ Wrappers)" {
  abstract class "IUart" {
    +virtual ~IUart() = 0
    +virtual Status init() = 0
    +virtual Status txByte(data) = 0
    +virtual Status txBuffer(span) = 0
  }
  
  abstract class "IStorage" {
    +virtual ~IStorage() = 0
    +virtual Status init() = 0
    +virtual Status read(addr, data, len) = 0
    +virtual Status write(addr, data, len) = 0
    +virtual Status eraseSector(addr) = 0
  }
}

package "HOST Implementations" {
  class "UartStub" {
    -isInitialized_: bool
    +init() Status override
    +txByte(data) Status override
    +txBuffer(data) Status override
    -logToConsole(data) void
  }
  
  class "StorageStub" {
    -buffer_: std::array<uint8_t, Capacity>
    +init() Status override
    +read(addr, data, len) Status override
    +write(addr, data, len) Status override
    +eraseSector(addr) Status override
  }
}

package "EMBEDDED Implementations" {
  class "UartReal" {
    -uartInstance_: UART_TypeDef*
    -isInitialized_: bool
    +init() Status override
    +txByte(data) Status override
    +txBuffer(data) Status override
    -waitUntilTxEmpty() void
  }
  
  class "FlashDriver" {
    -flashConfig_: FLASH_Config_TypeDef
    +init() Status override
    +read(addr, data, len) Status override
    +write(addr, data, len) Status override
    +eraseSector(addr) Status override
    -waitForOperation() void
  }
}

IUart <|-- UartStub : HOST implements
IUart <|-- UartReal : EMBEDDED implements
IStorage <|-- StorageStub : HOST implements
IStorage <|-- FlashDriver : EMBEDDED implements

note bottom of "imcal_uart.h"
  Pure C interface
  Compiled as C code
  extern "C" linkage
end note

note bottom of UartStub
  Maps UART to printf
  For GDB debugging
end note

note bottom of UartReal
  Direct register access
  MCU-specific (STM32, NXP, etc.)
end note

@enduml
```

---

## 5. Sequence Diagrams

### 5.1 Encryption Flow Sequence

```plantuml
@startuml Sequence Diagram - Encryption Flow
autonumber

participant "User App" as App
participant "HsmApiImpl" as Api
participant "SessionManager" as Session
participant "CryptoService" as Crypto
participant "KeystoreService" as Key
participant "CryptoDriver" as Driver
participant "MCAL UART" as Uart

App -> Api: encrypt(sessionId, keySlot, algo, plaintext)
Api -> Api: validateSession(sessionId)
Api -> Api: validateParameters()

Api -> Session: validateSession(sessionId)
Session --> Api: true

Api -> Crypto: encrypt(plaintext, ciphertext, keySlot)
Crypto -> Key: getKey(keySlot)
Key --> Crypto: key

Crypto -> Driver: encrypt(algo, key, plaintext)
Driver -> Driver: performEncryption()

Driver -> Uart: uartLog("[CRYPTO] Encrypting...")
Uart --> Driver: OK

Driver --> Crypto: ciphertext
Crypto --> Api: Status::OK

Api --> App: Status::OK, ciphertext

note right of App
  Application calls high-level API
  All complexity hidden below
end note

note right of Crypto
  CryptoService uses Strategy pattern
  Algorithm injected at runtime
end note

note right of Driver
  Driver bridges service to hardware
  Implements service interface
end note

@enduml
```

### 5.2 Initialization Sequence

```plantuml
@startuml Sequence Diagram - Initialization Flow
autonumber

participant "main()" as Main
participant "HsmApiImpl" as Api
participant "SessionManager" as Session
participant "CryptoService" as Crypto
participant "KeystoreService" as Key
participant "CryptoDriver" as CryptoDrv
participant "MCAL UART" as Uart

Main -> Api: init()
Api -> Uart: uartInit()
Uart --> Api: OK

Api -> Key: init()
Key -> Key: initializeKeySlots()
Key --> Api: OK

Api -> Crypto: init()
Crypto -> CryptoDrv: init()
CryptoDrv --> Crypto: OK
Crypto --> Api: OK

Api -> Session: init()
Session -> Session: allocateSessionPool()
Session --> Api: OK

Api --> Main: Status::OK

note right of Main
  Composition Root
  All dependencies wired here
end note

note right of Api
  Facade pattern
  Simplifies client code
end note

@enduml
```

### 5.3 Key Import Sequence

```plantuml
@startuml Sequence Diagram - Key Import Flow
autonumber

participant "User App" as App
participant "HsmApiImpl" as Api
participant "KeystoreService" as Key
participant "CryptoDriver" as Driver
participant "MCAL UART" as Uart

App -> Api: importKey(slotId, algo, keyData, perms)
Api -> Api: validateSlotId(slotId)
Api -> Api: validateKeySize(keyData, algo)

Api -> Key: importKey(slotId, algo, keyData, perms)
Key -> Key: isSlotOccupied(slotId)?
Key -> Key: copyKeyToSecureStorage()

Key -> Driver: generateRandomMask()
Driver -> Driver: useHardwareRNG()
Driver --> Key: randomMask

Key -> Key: xorKeyWithMask()

Key -> Uart: uartLog("[KEYSTORE] Key imported")
Uart --> Key: OK

Key --> Api: Status::OK
Api --> App: Status::OK

note right of Key
  Keys stored in secure memory
  XOR with random mask for protection
end note

note right of Driver
  Hardware RNG if available
  Falls back to mock for HOST
end note

@enduml
```

---

## 6. Dependency Graph

```plantuml
@startuml Dependency Graph
skinparam nodesep 40
skinparam ranksep 40

package "API Layer" {
  [HsmApiImpl] as Api
}

package "Middleware Layer" {
  [SessionManager] as Session
  [JobQueue] as JobQ
}

package "Service Layer" {
  [CryptoService] as Crypto
  [KeystoreService] as Key
  [StorageService] as Storage
}

package "Driver Layer" {
  [CryptoDriver] as CryptoDrv
  [KeystoreDriver] as KeyDrv
  [StorageDriver] as StorageDrv
}

package "MCAL Layer" {
  [IMcalUart] as Uart
  [IMcalStorage] as MStorage
  [IMcalCrypto] as MCrypto
}

package "Platform" {
  [HOST Stubs] as Host
  [EMBEDDED Drivers] as Embedded
}

Api --> Session
Api --> JobQ
Session --> Crypto
Session --> Key

Crypto --> Key : uses
Crypto --> CryptoDrv
Key --> KeyDrv
Storage --> StorageDrv

CryptoDrv --> MCrypto
KeyDrv --> MStorage
StorageDrv --> MStorage

MCrypto ..> Host
MCrypto ..> Embedded
MStorage ..> Host
MStorage ..> Embedded

note bottom
  Dependencies flow downward
  Services can use each other via interfaces
  MCAL is pure abstraction (C interfaces)
end note

@enduml
```

---

## 7. Design Patterns

### 7.1 Pattern Summary

| Pattern | Location | Purpose |
|---------|----------|---------|
| **Facade** | API Layer | Simplify client interface |
| **Strategy** | Crypto Service | Algorithm selection |
| **Factory** | Algorithm creation | Object creation |
| **Dependency Injection** | All layers | Decouple dependencies |
| **Singleton** | Keystore Service | Controlled global access |
| **Observer** | Event system | Async notifications |
| **State** | Session Manager | Session state machine |
| **Adapter** | MCAL wrappers | C to C++ interface |

### 7.2 Strategy Pattern Detail

```plantuml
@startuml Strategy Pattern - Crypto Algorithm
skinparam classAttributeIconSize 0

abstract class "ICryptoAlgorithm" {
  +virtual ~ICryptoAlgorithm() = 0
  +virtual Status encrypt(input, output) = 0
  +virtual Status decrypt(input, output) = 0
  +virtual getKeySize() size_t = 0
}

class "Aes128Algorithm" {
  +encrypt(input, output) Status
  +decrypt(input, output) Status
  +getKeySize() size_t
  -keyExpansion(key) void
  -encryptBlock(block, key) void
}

class "Aes256Algorithm" {
  +encrypt(input, output) Status
  +decrypt(input, output) Status
  +getKeySize() size_t
  -keyExpansion(key) void
}

class "RsaAlgorithm" {
  +encrypt(input, output) Status
  +decrypt(input, output) Status
  +getKeySize() size_t
  -modularExponent(base, exp, mod) uint32_t
}

class "CryptoService" {
  -algorithm_: std::unique_ptr<ICryptoAlgorithm>
  +CryptoService(algorithm)
  +encrypt(input, output) Status
  +decrypt(input, output) Status
}

ICryptoAlgorithm <|-- Aes128Algorithm
ICryptoAlgorithm <|-- Aes256Algorithm
ICryptoAlgorithm <|-- RsaAlgorithm

CryptoService o-- ICryptoAlgorithm : strategy

note right of CryptoService
  Algorithm can be swapped
  at runtime without
  modifying CryptoService
end note

@enduml
```

### 7.3 Dependency Injection Detail

```plantuml
@startuml Dependency Injection
skinparam classAttributeIconSize 0

package "Interfaces" {
  abstract class "ICryptoService" {
    +encrypt(...) = 0
    +decrypt(...) = 0
  }
  
  abstract class "IKeystoreService" {
    +getKey(...) = 0
    +importKey(...) = 0
  }
}

package "Implementations" {
  class "CryptoService" {
    -keystore_: std::shared_ptr<IKeystoreService>
    -algorithm_: std::unique_ptr<ICryptoAlgorithm>
    +CryptoService(keystore, algorithm)
  }
  
  class "KeystoreService" {
    +getKey(...) override
    +importKey(...) override
  }
}

package "Composition Root" {
  class "main()" {
    +main() int
  }
}

ICryptoService <|-- CryptoService
IKeystoreService <|-- KeystoreService

main() --> KeystoreService : creates
main() --> CryptoService : creates with DI
CryptoService --> IKeystoreService : injected

note right of main()
  Composition Root
  All dependencies wired here
  Services never create dependencies
end note

note left of CryptoService
  Constructor Injection
  Dependencies passed in
  Not created internally
end note

@enduml
```

---

## 8. MCAL Abstraction

### 8.1 MCAL Interface Design

```cpp
// C Interface (imcal_uart.h)
#pragma once

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
```

### 8.2 C++ Wrapper

```cpp
// C++ Wrapper (uart_wrapper.hpp)
#pragma once

#include "imcal_uart.h"
#include "iuart.hpp"

namespace ehsm::mcal {

class UartWrapper : public IUart {
public:
    Status init() override {
        return convertStatus(ehsm_mcal_uartInit());
    }
    
    Status txByte(uint8_t data) override {
        return convertStatus(ehsm_mcal_uartTxByte(data));
    }
    
    Status txBuffer(std::span<const uint8_t> data) override {
        return convertStatus(
            ehsm_mcal_uartTxBuffer(data.data(), data.size())
        );
    }

private:
    static Status convertStatus(ehsm_status_t cStatus) {
        // Convert C status to C++ Status
    }
};

} // namespace ehsm::mcal
```

### 8.3 Platform Abstraction

```plantuml
@startuml MCAL Platform Abstraction
skinparam rectangle {
  BackgroundColor<<C Interface>> LightCyan
  BackgroundColor<<C++ Wrapper>> LightGreen
  BackgroundColor<<Implementation>> LightYellow
}

rectangle "IMcalUart (C Interface)" <<C Interface>> {
  [ehsm_mcal_uartInit()\nehsm_mcal_uartTxByte()\nehsm_mcal_uartTxBuffer()]
}

rectangle "UartWrapper (C++ Adapter)" <<C++ Wrapper>> {
  [class UartWrapper : IUart\n- convertStatus()\n- init()\n- txByte()\n- txBuffer()]
}

rectangle "HOST: UartStub" <<Implementation>> {
  [class UartStub\n- printf mapping\n- console output]
}

rectangle "EMBEDDED: UartReal" <<Implementation>> {
  [class UartReal\n- UART register access\n- GPIO configuration]
}

[IMcalUart (C Interface)] --> [UartWrapper (C++ Adapter)] : wrapped by
[UartWrapper (C++ Adapter)] --> [HOST: UartStub] : implements for HOST
[UartWrapper (C++ Adapter)] --> [EMBEDDED: UartReal] : implements for EMBEDDED

note bottom
  C code for hardware access
  C++ wrapper for OOP code
  Same interface for both platforms
end note

@enduml
```

---

## 9. Extension Points

### 9.1 Adding New Algorithm

```
1. Create new class inheriting from ICryptoAlgorithm
2. Implement encrypt(), decrypt(), getKeySize()
3. Register with CryptoAlgorithmFactory
4. Inject into CryptoService via constructor

Example:
class ChaCha20Algorithm : public ICryptoAlgorithm {
    Status encrypt(...) override { /* ... */ }
    Status decrypt(...) override { /* ... */ }
    size_t getKeySize() const override { return 32; }
};
```

### 9.2 Adding New Storage Backend

```
1. Implement IStorage interface
2. Create driver that uses new storage
3. Wire in composition root

Example:
class EepromStorage : public IStorage {
    Status read(...) override { /* EEPROM read */ }
    Status write(...) override { /* EEPROM write */ }
    Status erase(...) override { /* EEPROM erase */ }
};
```

### 9.3 Adding New MCU Target

```
1. Create new MCAL implementation in src/mcal/
2. Implement C interfaces for UART, Storage, Crypto
3. Update CMakeLists.txt for new target
4. No changes to upper layers needed!

Example:
src/mcal/stm32f4/
├── mcal_uart.c
├── mcal_storage.c
└── mcal_crypto.c
```

---

## Appendix A: File Organization

```
embedded_hsm/
├── include/ehsm/              # Public headers
│   ├── api/
│   │   ├── ihsm_api.hpp
│   │   └── hsm_api.hpp
│   └── types/
│       ├── status.hpp
│       └── algorithm.hpp
│
├── src/
│   ├── api/
│   │   └── hsm_api_impl.cpp
│   ├── middleware/
│   │   └── session_manager.cpp
│   ├── services/
│   │   ├── crypto/
│   │   │   ├── crypto_service.cpp
│   │   │   └── algorithms/
│   │   │       ├── aes128_algorithm.cpp
│   │   │       └── aes256_algorithm.cpp
│   │   └── keystore/
│   │       └── keystore_service.cpp
│   ├── drivers/
│   │   └── crypto_driver.cpp
│   └── mcal/
│       ├── uart/
│       │   ├── mcal_uart_stub.c      # HOST
│       │   └── mcal_uart_real.c      # EMBEDDED
│       └── storage/
│           ├── mcal_storage_stub.c   # HOST
│           └── mcal_storage_flash.c  # EMBEDDED
│
└── tests/
    ├── test_crypto_service.cpp
    └── mocks/
        └── mock_keystore.hpp
```

---

**Document End**
