# Embedded HSM Firmware Architecture

## Overview

This document describes the architecture of the Embedded Hardware Security Module (HSM) firmware. The design follows a strict 5-layer architecture for maximum decoupling, testability, and maintainability.

## Layer Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    API Layer (hsm_api.h)                        │
│  Public interfaces: Hsm_Encrypt, Hsm_Decrypt, Hsm_Init, etc.   │
├─────────────────────────────────────────────────────────────────┤
│              Middleware Layer (hsm_middleware.h)                │
│  Job Router: Session management, job queuing, dispatch logic   │
├─────────────────────────────────────────────────────────────────┤
│                Service Layer (hsm_*.h)                          │
│  Core Logic: Crypto primitives, Keystore management            │
├─────────────────────────────────────────────────────────────────┤
│               Driver Layer (hsm_storage_driver.h)               │
│  Hardware Abstraction: Storage drivers, crypto accelerators    │
├─────────────────────────────────────────────────────────────────┤
│                 MCAL Layer (mcal_uart.h)                        │
│  MCU Abstraction: UART driver (or stub for HOST)               │
└─────────────────────────────────────────────────────────────────┘
```

## Layer Descriptions

### 1. API Layer (Highest)
- **Purpose**: Public interface for user applications
- **Files**: `include/hsm/hsm_api.h`, `src/api/hsm_api.c`
- **Key Functions**:
  - `Hsm_Init()` / `Hsm_Deinit()`
  - `Hsm_CreateSession()` / `Hsm_CloseSession()`
  - `Hsm_Encrypt()` / `Hsm_Decrypt()`
  - `Hsm_ImportKey()` / `Hsm_DeleteKey()`

### 2. Middleware Layer
- **Purpose**: Crypto Job Router and Session Manager
- **Files**: `include/hsm/hsm_middleware.h`, `src/middleware/hsm_middleware.c`
- **Responsibilities**:
  - Session lifecycle management
  - Job queue management
  - Job routing to appropriate services
  - Access control enforcement

### 3. Service Layer
- **Purpose**: Core business logic and cryptographic operations
- **Files**:
  - `hsm_crypto_service.h/c` - Crypto primitives
  - `hsm_keystore.h/c` - Key slot management
- **Features**:
  - Mock crypto implementations (for PoC)
  - Secure key storage
  - Permission-based access control

### 4. Driver Layer
- **Purpose**: Hardware abstraction for peripherals
- **Files**: `hsm_storage_driver.h/c`
- **Responsibilities**:
  - Non-volatile storage interface
  - Hardware crypto accelerator interface (future)

### 5. MCAL Layer (Lowest)
- **Purpose**: Microcontroller Abstraction Layer
- **Files**: `mcal_uart.h`, `mcal_uart.c` (EMBEDDED), `mcal_uart_stub.c` (HOST)
- **Features**:
  - UART communication interface
  - Platform-specific implementations
  - Debug logging via UART

## Platform Support

### HOST Platform (PC Debugging)
```bash
cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build
./build/hsm_host
```

- Uses UART stub that maps to `printf`
- Includes GDB debug symbols (`-g -O0`)
- Code coverage enabled (`-fprofile-arcs -ftest-coverage`)
- Full traceability via console output

### EMBEDDED Platform
```bash
cmake -DTARGET_PLATFORM=EMBEDDED -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -B build && cmake --build build
```

- Uses real UART driver
- Optimized for size (`-Os`)
- Hardware-specific implementations

## Data Flow Example: Encryption

```
Application
    │
    ▼
Hsm_Encrypt()  [API Layer]
    │
    ├─► Validate parameters
    ├─► Check session validity
    └─► Verify key permissions
    │
    ▼
Hsm_Middleware_DispatchJob()  [Middleware Layer]
    │
    ├─► Assign job ID
    ├─► Queue job
    └─► Route to crypto service
    │
    ▼
Hsm_CryptoService_Encrypt()  [Service Layer]
    │
    ├─► Retrieve key from keystore
    ├─► Perform encryption
    └─► Log via UART
    │
    ▼
Hsm_Keystore_GetKey()  [Service Layer]
    │
    └─► Return key data
    │
    ▼
Mcal_Uart_Log()  [MCAL Layer]
    │
    └─► Output to console (HOST) or UART (EMBEDDED)
```

## Build System Features

### CMake Targets
| Target | Description |
|--------|-------------|
| `hsm_firmware` | Static library with all HSM code |
| `hsm_host` | Host executable for testing |
| `docs` | Generate Doxygen documentation |
| `test` | Run unit tests (CTest) |

### Compiler Flags
- **Strict warnings**: `-Wall -Wextra -Wpedantic -Werror`
- **MISRA-C mindset**: `-Wconversion -Wsign-conversion`
- **Debug (HOST)**: `-g -O0 -fprofile-arcs -ftest-coverage`
- **Release (EMBEDDED)**: `-Os -ffunction-sections -fdata-sections`

## Testing Strategy

### Unit Testing
- Framework: Unity/CTest
- Location: `tests/unit/`
- Coverage: Crypto service, keystore

### Integration Testing
- Full stack testing via `hsm_host`
- GDB debugging with UART trace

### Static Analysis
- `clang-tidy` ready
- MISRA-C compliance checking (future)

## Security Considerations

1. **No dynamic memory**: No `malloc`/`free` to prevent heap vulnerabilities
2. **Secure key erasure**: Keys are zeroed before slot reuse
3. **Permission-based access**: Keys have explicit permission flags
4. **Session isolation**: Each session has independent context
5. **Constant-time operations**: (Future) Prevent timing attacks

## Future Enhancements

- [ ] Hardware crypto accelerator support
- [ ] True random number generator (TRNG)
- [ ] Secure boot integration
- [ ] Firmware update mechanism
- [ ] Side-channel attack countermeasures
- [ ] MISRA-C 2012 compliance verification
- [ ] Common Criteria preparation
