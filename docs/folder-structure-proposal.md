# Folder Structure Proposal

## Current Structure Analysis

### Issues with Current Structure:

1. **Mixed naming conventions**: `src/stubs/` vs `src/mcal/`
2. **Inconsistent layer organization**: MCAL is in `src/mcal/` but stubs are separate
3. **Public headers mixed with internal**: `include/hsm/` contains both public API and internal headers
4. **Tests not co-located with modules**: Tests are in `tests/` separate from source
5. **No platform-specific separation**: HOST/EMBEDDED code mixed in same directories

---

## Proposed Clean Structure

```
embedded_hsm/
├── CMakeLists.txt                    # Root build config
├── toolchain.cmake                   # Embedded toolchain (if needed)
│
├── include/                          # Public headers (for external use)
│   └── hsm/
│       ├── hsm_api.h                 # Public API
│       └── hsm_types.h               # Public types
│
├── src/                              # Implementation
│   ├── api/                          # API Layer (facade)
│   │   ├── CMakeLists.txt
│   │   ├── hsm_api.c
│   │   └── hsm_api.h                 # Internal header
│   │
│   ├── middleware/                   # Middleware Layer
│   │   ├── CMakeLists.txt
│   │   ├── hsm_middleware.c
│   │   └── hsm_middleware.h
│   │
│   ├── services/                     # Service Layer
│   │   ├── crypto/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── hsm_crypto_service.c
│   │   │   └── hsm_crypto_service.h
│   │   └── keystore/
│   │       ├── CMakeLists.txt
│   │       ├── hsm_keystore.c
│   │       └── hsm_keystore.h
│   │
│   ├── drivers/                      # Driver Layer
│   │   ├── CMakeLists.txt
│   │   ├── storage/
│   │   │   ├── hsm_storage_driver.c
│   │   │   └── hsm_storage_driver.h
│   │   └── crypto_hw/                # Future hardware crypto
│   │
│   ├── mcal/                         # MCAL Layer
│   │   ├── CMakeLists.txt
│   │   ├── uart/
│   │   │   ├── mcal_uart.c           # Embedded UART
│   │   │   └── mcal_uart.h
│   │   └── platform/                 # Platform-specific
│   │       ├── platform_host.c
│   │       └── platform_embedded.c
│   │
│   └── main.c                        # HOST entry point only
│
├── tests/                           # Test suite
│   ├── CMakeLists.txt
│   ├── test_crypto_service.c        # Replaces tests/unit/
│   ├── test_keystore.c
│   └── fixtures/
│       └── test_data.h
│
├── cmake/                           # CMake modules
│   ├── CompilerWarnings.cmake
│   ├── PlatformConfig.cmake
│   └── Doxygen.cmake
│
└── docs/                            # Documentation
    ├── architecture.md
    └── api/
        └── doxygen/                  # Generated docs
```

---

## Key Improvements

### 1. Module-based Organization

```
src/services/
├── crypto/        # Crypto service module
└── keystore/      # Keystore module
```

- Each module has its own directory
- CMakeLists.txt at each level for fine-grained builds

### 2. Platform Separation

```
src/mcal/
├── uart/
│   ├── mcal_uart.c        # Common interface
│   └── platform/
│       ├── host/          # HOST-specific
│       │   └── uart_stub.c
│       └── embedded/      # EMBEDDED-specific
│           └── uart_real.c
```

### 3. Clear Header Visibility

| Location       | Contents         | Visibility |
| -------------- | ---------------- | ---------- |
| `include/hsm/` | Public API only  | External   |
| `src/*/*.h`    | Internal headers | Internal   |

### 4. Flat Test Structure

```
tests/
├── test_crypto_service.c   # Simple, flat structure
├── test_keystore.c
└── fixtures/
    └── test_data.h
```

- Removes unnecessary `unit/` and `integration/` nesting
- Tests can include headers directly from `src/`

---

## Migration Plan

### Phase 1: Create New Structure

```bash
# Create directories
mkdir -p src/services/crypto
mkdir -p src/services/keystore
mkdir -p src/drivers/storage
mkdir -p src/mcal/uart/platform/host
mkdir -p src/mcal/uart/platform/embedded
mkdir -p tests/fixtures

# Move files
mv src/services/hsm_crypto_service.c src/services/crypto/
mv src/services/hsm_keystore.c src/services/keystore/
mv src/drivers/hsm_storage_driver.c src/drivers/storage/
mv src/mcal/mcal_uart.c src/mcal/uart/
mv src/stubs/mcal_uart_stub.c src/mcal/uart/platform/host/
```

### Phase 2: Update CMakeLists.txt

- Update all `add_subdirectory()` calls
- Update include paths
- Update target linking

### Phase 3: Update Source References

- Update `#include` directives
- Update namespace/module references

---

## Summary

| Aspect   | Current                             | Proposed                                      |
| -------- | ----------------------------------- | --------------------------------------------- |
| Services | Flat in `src/services/`             | `src/services/{crypto,keystore}/`             |
| Drivers  | Flat in `src/drivers/`              | `src/drivers/{storage,crypto_hw}/`            |
| MCAL     | Mixed                               | `src/mcal/{module}/platform/{host,embedded}/` |
| Tests    | `tests/unit/`, `tests/integration/` | Flat `tests/`                                 |
| Headers  | Mixed in `include/hsm/`             | Public in `include/`, internal in `src/`      |

This structure follows embedded industry standards and improves:

- **Maintainability**: Clear module boundaries
- **Testability**: Simpler test structure
- **Portability**: Platform code clearly separated
- **Scalability**: Easy to add new modules/drivers
