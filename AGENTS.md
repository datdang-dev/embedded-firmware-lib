# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Build Commands

**HOST platform (PC debugging with GDB):**

```bash
cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build
./build/hsm_host
```

**Run tests (HOST only):**

```bash
cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build --target test
# Or: ctest --test-dir build
```

**Run single test:**

```bash
./build/test_crypto_service    # Run crypto service test
./build/test_keystore          # Run keystore test
```

**EMBEDDED platform:**

```bash
cmake -DTARGET_PLATFORM=EMBEDDED -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -B build && cmake --build build
```

## Project Structure (5-Layer Architecture)

```
include/hsm/     # Public API headers
src/api/         # API layer (facade)
src/middleware/  # Job router & session management
src/services/    # Crypto & keystore
src/drivers/    # Hardware abstraction
src/mcal/       # UART driver (or stub for HOST)
```

## Key Conventions

- **Include style**: Use `#include "hsm/hsm_api.h"` (not relative paths)
- **No dynamic memory**: Project intentionally avoids `malloc`/`free` for security
- **Status handling**: All functions return `Hsm_Status_t` (0 = success, negative = error)
- **Width types**: Use `uint8_t`, `uint32_t`, `size_t` from `<stdint.h>`
- **Constants**: Suffix with `U` for unsigned (`0xFFU`), `L` for long (`1UL`)

## Non-Obvious Patterns

- Out-of-source builds required: never build in source directory
- `TARGET_PLATFORM` must be `HOST` or `EMBEDDED` (defaults to HOST)
- `main.c` is only compiled for HOST target (hsm_host executable)
- Static library `hsm_firmware.a` contains all layers for both platforms
- MCAL UART is stubbed on HOST (maps to printf), real driver on EMBEDDED
