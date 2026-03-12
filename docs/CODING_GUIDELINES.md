# Embedded HSM Firmware - Coding Guidelines

**Document ID:** `HSM-CG-001`
**Version:** 1.1.0
**Last Updated:** 2026-03-12
**Applicable To:** All Embedded HSM Firmware source code
**Owner:** Embedded Engineering Team
**Compliance:** MISRA-C:2012 Mindset, Automotive Security Standards

---

## Naming Convention Update (v1.1.0)

### **New Naming Standard**

All public interfaces MUST follow this naming convention:

```
Format: ehsm_<module>_<functionName>()
        ─┬─  ──┬───  ─────┬────
         │     │          └─ camelCase function name
         │     └─ module name (short form)
         └─ project prefix
```

### **Examples**

| Layer | Old Name (snake_case) | New Name (camelCase) |
|-------|----------------------|---------------------|
| API | `Hsm_Init()` | `ehsm_api_init()` |
| API | `Hsm_Encrypt()` | `ehsm_api_encrypt()` |
| API | `Hsm_Decrypt()` | `ehsm_api_decrypt()` |
| Middleware | `Hsm_Middleware_Init()` | `ehsm_mw_init()` |
| Middleware | `Hsm_Middleware_DispatchJob()` | `ehsm_mw_dispatchJob()` |
| Service | `Hsm_CryptoService_Encrypt()` | `ehsm_crypto_encrypt()` |
| Service | `Hsm_Keystore_Init()` | `ehsm_keystore_init()` |
| Driver | `Hsm_Storage_Init()` | `ehsm_storage_init()` |
| MCAL | `Mcal_Uart_Init()` | `ehsm_mcal_uartInit()` |
| MCAL | `Mcal_Uart_TxByte()` | `ehsm_mcal_uartTxByte()` |

### **Type Naming**

```c
/* Types: PascalCase with _t suffix, ehsm_ prefix */
typedef enum EhsmStatus { ... } ehsm_status_t;
typedef enum EhsmAlgorithm { ... } ehsm_algorithm_t;
typedef struct EhsmKeySlot { ... } ehsm_keySlot_t;
typedef struct EhsmCryptoJob { ... } ehsm_cryptoJob_t;

/* Enum values: PASCAL_CASE with EHSM_ prefix */
#define EHSM_STATUS_OK                  (0)
#define EHSM_STATUS_ERR_INVALID_PARAM   (-1)
#define EHSM_ALGORITHM_AES_128          (1)
```

### **Module Prefixes**

| Module | Prefix | Example |
|--------|--------|---------|
| API | `ehsm_api_` | `ehsm_api_init()` |
| Middleware | `ehsm_mw_` | `ehsm_mw_dispatchJob()` |
| Crypto Service | `ehsm_crypto_` | `ehsm_crypto_encrypt()` |
| Keystore | `ehsm_keystore_` | `ehsm_keystore_importKey()` |
| Storage | `ehsm_storage_` | `ehsm_storage_read()` |
| MCAL | `ehsm_mcal_` | `ehsm_mcal_uartInit()` |

---

## 1. Introduction

### 1.1 Purpose

This document establishes coding standards for the Embedded HSM Firmware project. These guidelines ensure:

- **Safety:** Critical security code with zero tolerance for memory corruption
- **Reliability:** Deterministic behavior in resource-constrained environments
- **Maintainability:** Clear, consistent code for long-term embedded projects
- **Security:** Protection against common embedded vulnerabilities
- **Portability:** Hardware abstraction for multiple MCU targets

### 1.2 Scope

These guidelines apply to:
- All production C code in the HSM firmware stack
- All MCAL (Microcontroller Abstraction Layer) code
- All device drivers and hardware interfaces
- All test code (unit, integration, hardware-in-the-loop)
- Build scripts and linker configurations

### 1.3 Compliance Levels

| Level | Description | Enforcement |
|-------|-------------|-------------|
| **MUST** | Mandatory requirement | Enforced by static analysis (clang-tidy) |
| **SHOULD** | Strong recommendation | Enforced by code review |
| **MAY** | Optional guidance | Developer discretion |

### 1.4 MISRA-C:2012 Alignment

This project follows a **MISRA-C:2012 mindset** with these key rules:

| MISRA Rule | Project Policy |
|------------|----------------|
| 1.1 | No project-specific extensions to C language |
| 2.1 | All source files must compile without errors |
| 4.1 | Use explicit signed/unsigned types (`int32_t`, `uint8_t`) |
| 5.1 | External identifiers must be unique (31 characters) |
| 7.1 | Octal constants prohibited (except `0`) |
| 7.2 | Use `u` suffix for unsigned constants |
| 8.1 | Functions must have prototype declarations |
| 8.2 | Function names must be unique |
| 8.3 | Function declarations and definitions must match |
| 8.4 | Compatible function declaration types |
| 8.7 | External variables must have single definition |
| 10.1 | No mixing of signed/unsigned in expressions |
| 11.1 | No implicit pointer conversions |
| 13.1 | No assignment in conditional expressions |
| 13.4 | Result of assignment not used (except in loops) |
| 14.1 | Loop counters must not be modified inside loop body |
| 14.2 | All non-void functions must return a value |
| 15.1 | `goto` statements prohibited |
| 15.2 | `break`/`continue` only in iteration statements |
| 15.3 | `switch` statements must have `default` case |
| 16.1 | All `switch` labels must be unique |
| 17.1 | Variable argument lists prohibited |
| 17.2 | Recursion prohibited |
| 17.3 | Function-like macros prohibited (except logging) |
| 18.1 | Pointers must not point to overlapping storage |
| 21.1 | `#error` and `#pragma` allowed with justification |

---

## 2. Embedded Design Principles

### 2.1 SOLID for Embedded C

#### 2.1.1 Single Responsibility Principle (SRP)

**Rule:** Each module, function, or layer must have exactly one reason to change.

**Reasoning:** Isolates concerns, reduces coupling, simplifies testing on HOST.

**Example:**

```c
/* ❌ BAD: Multiple responsibilities in one function */
ehsm_status_t ehsm_api_processEncryption(ehsm_cryptoJob_t* job)
{
    /* Validation logic */
    if (job->input.data == NULL) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Key retrieval logic */
    ehsm_key_t key;
    (void)memcpy(key.data, g_keyStore[job->keySlot].data, 32);
    
    /* Crypto logic */
    for (size_t i = 0; i < job->input.length; i++) {
        job->output.data[i] = job->input.data[i] ^ key.data[i % 32];
    }
    
    /* Logging logic */
    ehsm_mcal_uartLog("[HSM] Encrypted %zu bytes\n", job->input.length);
    
    return EHSM_STATUS_OK;
}

/* ✅ GOOD: Separated responsibilities */
static ehsm_status_t validateJob(const ehsm_cryptoJob_t* job);
static ehsm_status_t retrieveKey(uint8_t slotId, ehsm_key_t* key);
static ehsm_status_t performEncryption(ehsm_cryptoJob_t* job, const ehsm_key_t* key);

ehsm_status_t ehsm_api_processEncryption(ehsm_cryptoJob_t* job)
{
    ehsm_status_t status;
    
    status = validateJob(job);
    if (status != EHSM_STATUS_OK) { return status; }
    
    ehsm_key_t key;
    status = retrieveKey(job->keySlot, &key);
    if (status != EHSM_STATUS_OK) { return status; }
    
    return performEncryption(job, &key);
}
```

#### 2.1.2 Open/Closed Principle (OCP)

**Rule:** Modules must be open for extension but closed for modification.

**Reasoning:** Enables adding algorithms without modifying tested code.

**Example:**

```c
/* ❌ BAD: Requires modification for new algorithms */
ehsm_status_t ehsm_crypto_encrypt(ehsm_cryptoJob_t* job, ehsm_algorithm_t algo)
{
    if (algo == EHSM_ALGORITHM_AES_128) {
        return aes128Encrypt(job);
    } else if (algo == EHSM_ALGORITHM_AES_256) {
        return aes256Encrypt(job);
    }
    /* Must modify this function for new algorithms */
    return EHSM_STATUS_ERR_UNSUPPORTED;
}

/* ✅ GOOD: Function pointer table for extension */
typedef struct {
    ehsm_algorithm_t algo;
    ehsm_status_t (*encrypt)(ehsm_cryptoJob_t*);
    ehsm_status_t (*decrypt)(ehsm_cryptoJob_t*);
} ehsm_cryptoHandler_t;

static const ehsm_cryptoHandler_t g_handlers[] = {
    { EHSM_ALGORITHM_AES_128, aes128Encrypt, aes128Decrypt },
    { EHSM_ALGORITHM_AES_256, aes256Encrypt, aes256Decrypt },
    { EHSM_ALGORITHM_NONE,    NULL,            NULL           }
};

ehsm_status_t ehsm_crypto_encrypt(ehsm_cryptoJob_t* job, ehsm_algorithm_t algo)
{
    for (size_t i = 0; g_handlers[i].encrypt != NULL; i++) {
        if (g_handlers[i].algo == algo) {
            return g_handlers[i].encrypt(job);
        }
    }
    return EHSM_STATUS_ERR_UNSUPPORTED;
}
```

#### 2.1.3 Dependency Inversion Principle (DIP)

**Rule:** High-level modules must not depend on low-level modules. Both depend on abstractions.

**Reasoning:** Enables HOST testing with stubs.

**Example:**

```c
/* ❌ BAD: High-level depends on hardware */
#include "stm32f4xx.h"

void ehsm_mcal_uartLog(const char* event)
{
    /* Direct UART register access */
    USART1->DR = event[0];
    while (!(USART1->SR & USART_SR_TC)) { }
}

/* ✅ GOOD: Through abstraction */
/* ehsm_mcal_uart.h */
ehsm_status_t ehsm_mcal_uartTxString(const char* str);

/* Logger implementation */
void ehsm_mcal_uartLog(const char* event)
{
    (void)ehsm_mcal_uartTxString(event);  /* Abstraction */
}

/* On HOST: ehsm_mcal_uartTxString maps to printf */
/* On EMBEDDED: ehsm_mcal_uartTxString maps to UART hardware */
```

### 2.2 Embedded-Specific Principles

#### 2.2.1 DRY (Don't Repeat Yourself)

**Rule:** Every piece of knowledge must have a single representation.

**Caveat:** Avoid over-abstraction that adds runtime overhead.

```c
/* ✅ GOOD: Shared constant */
#define EHSM_AES_BLOCK_SIZE    (16U)

/* Used consistently across all AES operations */
```

#### 2.2.2 KISS (Keep It Simple, Stupid)

**Rule:** Prefer the simplest solution that meets requirements.

**Reasoning:** Simple code has fewer bugs and is easier to certify.

```c
/* ❌ BAD: Over-engineered */
#define GET_BIT(ptr, bit) (((*(ptr + ((bit) >> 3))) >> ((bit) & 0x07U)) & 0x01U)

/* ✅ GOOD: Clear and simple */
static inline uint8_t getBit(const uint8_t* ptr, size_t bitIndex)
{
    size_t byteIndex = bitIndex / 8U;
    size_t bitOffset = bitIndex % 8U;
    return (ptr[byteIndex] >> bitOffset) & 0x01U;
}
```

#### 2.2.3 YAGNI (You Ain't Gonna Need It)

**Rule:** Do not add functionality until it is necessary.

**Reasoning:** Embedded systems have limited flash/RAM.

#### 2.2.4 Deterministic Execution

**Rule:** All operations must have bounded, predictable execution time.

**Reasoning:** Real-time requirements for HSM operations.

```c
/* ❌ BAD: Unbounded loop */
while (uartNotEmpty()) {
    processByte();  /* How many iterations? */
}

/* ✅ GOOD: Bounded loop */
for (uint32_t i = 0; i < EHSM_MAX_UART_BYTES; i++) {
    if (!uartNotEmpty()) { break; }
    processByte();
}
```

---

## 3. Code Formatting Rules

### 3.1 General Formatting

#### 3.1.1 Indentation

**Rule:** Use 4 spaces for indentation. Never use tabs.

**Reasoning:** Consistent display across all editors and terminals.

```c
/* ✅ GOOD */
ehsm_status_t ehsm_api_init(uint32_t flags)
{
    ehsm_status_t status;
    
    if (flags == 0U) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    status = initializeLayers();
    return status;
}

/* ❌ BAD (tabs) */
ehsm_status_t ehsm_api_init(uint32_t flags)
{
	ehsm_status_t status;
	
	if (flags == 0U) {
		return EHSM_STATUS_ERR_INVALID_PARAM;
	}
	
	status = initializeLayers();
	return status;
}
```

#### 3.1.2 Line Length

**Rule:** Maximum line length is 100 characters.

**Reasoning:** Fits in standard terminal windows and diff views.

```c
/* ❌ BAD: Line too long */
ehsm_status_t ehsm_api_initializeHardwareSecurityModuleWithAllSubsystemsAndPerformCompleteSelfTestBeforeAcceptingAnyCommands(void);

/* ✅ GOOD: Broken into readable lines */
ehsm_status_t ehsm_api_init(
    uint32_t configFlags,
    ehsm_callback_t callback
);
```

#### 3.1.3 Trailing Whitespace

**Rule:** No trailing whitespace. Files must end with a single newline.

#### 3.1.4 Semicolons

**Rule:** Always use semicolons. Never omit for single-statement blocks.

```c
/* ✅ GOOD */
for (i = 0; i < count; i++) {
    processItem(i);
}

/* ❌ BAD */
for (i = 0; i < count; i++)
    processItem(i);
```

### 3.2 Braces and Spacing

#### 3.2.1 Brace Style

**Rule:** Use K&R (One True Brace) style.

```c
/* ✅ GOOD */
if (condition) {
    doSomething();
}

while (condition) {
    doSomething();
}

for (i = 0; i < count; i++) {
    doSomething(i);
}

switch (value) {
case 0:
    handleZero();
    break;
default:
    handleOther();
    break;
}

/* ❌ BAD */
if (condition)
{
    doSomething();
}
```

#### 3.2.2 Space Usage

**Rule:**
- Space after keywords (`if`, `for`, `while`, `switch`, `return`)
- Space around binary operators (`+`, `-`, `*`, `/`, `=`, `==`, `!=`)
- No space after unary operators (`&`, `*`, `!`, `~`, `++`, `--`)
- Space after commas in parameter lists

```c
/* ✅ GOOD */
if ((value > 0) && (value < MAX)) {
    result = (a + b) * (c - d);
    *ptr = ~value;
    process(a, b, c);
}

/* ❌ BAD */
if((value>0)&&(value<MAX)) {
    result=(a+b)*(c-d);
    *ptr = ~ value;
    process(a,b,c);
}
```

#### 3.2.3 Pointer Declarations

**Rule:** Attach `*` to the type, not the variable name.

```c
/* ✅ GOOD: Consistent with type semantics */
const uint8_t* data;    /* Pointer to constant data */
uint8_t* const ptr;     /* Constant pointer */
const uint8_t* const c; /* Constant pointer to constant */

/* ❌ BAD: Inconsistent */
uint8_t *data;
uint8_t * const ptr;
```

### 3.3 C-Specific Formatting

#### 3.3.1 Type Definitions

**Rule:** Use `typedef` for structs and enums. Always include `_t` suffix.

```c
/* ✅ GOOD */
typedef enum EhsmStatus {
    EHSM_STATUS_OK = 0,
    EHSM_STATUS_ERR_INVALID_PARAM = -1
} ehsm_status_t;

typedef struct EhsmContext {
    uint32_t flags;
    uint8_t* buffer;
} ehsm_context_t;

/* ❌ BAD */
enum EhsmStatus {
    EHSM_STATUS_OK,
    EHSM_STATUS_ERR
};

struct EhsmContext {
    uint32_t flags;
};
```

#### 3.3.2 Fixed-Width Types

**Rule:** Always use `<stdint.h>` types. Never use bare `int`, `char`, `long`.

```c
/* ✅ GOOD */
uint8_t  byteValue;
uint16_t wordValue;
uint32_t dwordValue;
int32_t  signedValue;
size_t   arrayIndex;

/* ❌ BAD */
int counter;          /* How many bits? */
char flag;            /* Signed or unsigned? */
long timeout;         /* 32 or 64 bits? */
unsigned int value;   /* How wide? */
```

#### 3.3.3 Constants and Literals

**Rule:** Use `U` suffix for unsigned, `L` for long. Use hexadecimal for bitmasks.

```c
/* ✅ GOOD */
#define EHSM_MAX_RETRIES        (5U)
#define EHSM_TIMEOUT_MS         (1000UL)
#define EHSM_STATUS_MASK        (0x00FFU)
#define EHSM_FLAG_INITIALIZED   (1U << 0)
#define EHSM_FLAG_READY         (1U << 1)

/* ❌ BAD */
#define MAX_RETRIES        5       /* Signed? */
#define TIMEOUT_MS         1000    /* Long? */
#define STATUS_MASK        255     /* Unclear bitmask */
```

---

## 4. Naming Conventions

### 4.1 General Naming Rules

#### 4.1.1 Descriptive Names

**Rule:** Names must reveal intent. Avoid abbreviations except for well-known terms.

```c
/* ❌ BAD: Unclear */
uint8_t* buf;
uint32_t cnt;
void procData(void);

/* ✅ GOOD: Clear intent */
uint8_t* ciphertextBuffer;
uint32_t retryCount;
void processCryptographicData(void);

/* ✅ ACCEPTABLE: Standard abbreviations */
uint8_t* txBuffer;      /* Transmit */
uint8_t* rxBuffer;      /* Receive */
uint32_t gpioPin;       /* General Purpose I/O */
void uartInit(void);    /* Universal Asynchronous Receiver-Transmitter */
```

#### 4.1.2 Boolean Variables

**Rule:** Prefix with `is`, `has`, `can`, `should`.

```c
/* ❌ BAD */
bool ready;
bool error;
bool init;

/* ✅ GOOD */
bool isReady;
bool hasError;
bool isInitialized;
```

#### 4.1.3 Magic Numbers

**Rule:** Named constants for all magic numbers (except 0, 1, 2).

```c
/* ❌ BAD */
for (i = 0; i < 32; i++) { ... }
delayMs(100);
if (retries > 3) { ... }

/* ✅ GOOD */
#define EHSM_AES_KEY_SIZE_BYTES    (32U)
#define EHSM_DEBOUNCE_DELAY_MS     (100U)
#define EHSM_MAX_RETRY_COUNT       (3U)

for (i = 0; i < EHSM_AES_KEY_SIZE_BYTES; i++) { ... }
delayMs(EHSM_DEBOUNCE_DELAY_MS);
if (retries > EHSM_MAX_RETRY_COUNT) { ... }
```

### 4.2 Naming by Entity Type

| Entity Type | Convention | Example |
|-------------|------------|---------|
| Functions | `ehsm_<module>_camelCase()` | `ehsm_api_encrypt()`, `ehsm_mw_dispatchJob()` |
| Variables | `camelCase` with descriptive name | `keySlotId`, `ciphertextLen` |
| Types | `ehsm_<name>_t` (lowercase with _t) | `ehsm_status_t`, `ehsm_context_t` |
| Enums | `Ehsm<Name>` typedef to `ehsm_<name>_t` | `ehsm_algorithm_t` |
| Enum Values | `EHSM_<NAME>` (PASCAL_CASE) | `EHSM_STATUS_OK`, `EHSM_STATUS_ERR_INVALID_PARAM` |
| Constants | `EHSM_<NAME>` (PASCAL_CASE) | `EHSM_MAX_KEY_SLOTS`, `EHSM_AES_BLOCK_SIZE` |
| Macros | `EHSM_<NAME>()` | `EHSM_LOG()`, `EHSM_ASSERT()` |
| Files | `snake_case` with module prefix | `ehsm_api.c`, `ehsm_mcal_uart.c` |

### 4.3 Module Prefix Rules

**Rule:** All public symbols must be prefixed with `ehsm_` followed by module identifier.

```c
/* API Layer */
ehsm_api_init, ehsm_api_deinit, ehsm_api_encrypt, ehsm_api_decrypt

/* Middleware Layer */
ehsm_mw_init, ehsm_mw_dispatchJob, ehsm_mw_createSession

/* MCAL Layer */
ehsm_mcal_uartInit, ehsm_mcal_uartTxByte, ehsm_mcal_uartRxByte

/* ❌ BAD: No project prefix */
init, encrypt, decrypt, uartSend  /* Conflicts likely */
```

### 4.4 File Naming

**Rule:**
- Header files: `ehsm_module.h`
- Source files: `ehsm_module.c`
- Match file name to primary module

```
✅ GOOD:
- ehsm_api.h / ehsm_api.c
- ehsm_middleware.h / ehsm_middleware.c
- ehsm_mcal_uart.h / ehsm_mcal_uart.c (and ehsm_mcal_uart_stub.c)

❌ BAD:
- HsmApi.h / hsmapi.c      (inconsistent casing)
- api_hsm.h / api.c       (unclear module)
```

---

## 5. File and Directory Organization

### 5.1 Project Structure

**Rule:** Organize by architectural layer.

```
embedded_hsm/
├── include/ehsm/          # Public headers
│   ├── ehsm_types.h       # Common types (layer-independent)
│   ├── ehsm_api.h         # API layer interface
│   ├── ehsm_middleware.h  # Middleware layer interface
│   ├── ehsm_crypto.h      # Crypto service interface
│   ├── ehsm_keystore.h    # Keystore interface
│   ├── ehsm_storage.h     # Storage driver interface
│   └── ehsm_mcal_uart.h   # MCAL interface
├── src/
│   ├── api/               # API layer implementation
│   ├── middleware/        # Middleware layer
│   ├── services/          # Service layer
│   ├── drivers/           # Driver layer
│   ├── mcal/              # MCAL layer (EMBEDDED)
│   └── stubs/             # MCAL stubs (HOST)
├── tests/
│   ├── unit/              # Unit tests
│   └── integration/       # Integration tests
├── cmake/                 # Build configuration
└── docs/                  # Documentation
```

### 5.2 Header File Structure

**Rule:** All headers must follow this structure:

```c
/**
 * @file ehsm_module.h
 * @brief Brief description of module purpose.
 * 
 * Detailed description if needed.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#ifndef EHSM_MODULE_H
#define EHSM_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm_types.h"

/* ============================================================================
 * DEFINITIONS
 * ============================================================================
 */

/* ============================================================================
 * TYPE DEFINITIONS
 * ============================================================================
 */

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================
 */

#ifdef __cplusplus
}
#endif

#endif /* EHSM_MODULE_H */
```

### 5.3 Source File Structure

**Rule:** All source files must follow this structure:

```c
/**
 * @file ehsm_module.c
 * @brief Implementation of ehsm_module module.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

/* ============================================================================
 * INCLUDES
 * ============================================================================
 */
#include "ehsm_module.h"
#include "ehsm_private.h"

/* ============================================================================
 * PRIVATE DEFINITIONS
 * ============================================================================
 */
#define PRIVATE_CONSTANT    (100U)

/* ============================================================================
 * PRIVATE TYPE DEFINITIONS
 * ============================================================================
 */
typedef struct {
    uint32_t state;
} ModulePrivate_t;

/* ============================================================================
 * PRIVATE VARIABLES
 * ============================================================================
 */
static ModulePrivate_t g_module;

/* ============================================================================
 * PRIVATE FUNCTION DECLARATIONS
 * ============================================================================
 */
static void privateHelperFunction(void);

/* ============================================================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */

/* ============================================================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * ============================================================================
 */
```

### 5.4 Include Guard Rules

**Rule:** All headers must have unique include guards.

```c
/* ✅ GOOD: Unique, matches file path */
#ifndef EHSM_API_H
#define EHSM_API_H
#endif

/* ✅ GOOD: With project prefix */
#ifndef EHSM_MCAL_UART_H
#define EHSM_MCAL_UART_H
#endif

/* ❌ BAD: Generic, may conflict */
#ifndef API_H
#define API_H
#endif
```

---

## 6. Function Design Rules

### 6.1 Function Size

#### 6.1.1 Line Count

**Rule:** Functions should not exceed 40 lines.

**Reasoning:** Small functions are easier to understand, test, and verify.

```c
/* ❌ BAD: Too long, multiple responsibilities */
ehsm_status_t ehsm_mw_processJob(ehsm_cryptoJob_t* job)
{
    /* 80 lines doing validation, key lookup, encryption, logging, cleanup... */
}

/* ✅ GOOD: Decomposed */
ehsm_status_t ehsm_mw_processJob(ehsm_cryptoJob_t* job)
{
    ehsm_status_t status;
    
    status = validateJob(job);
    if (status != EHSM_STATUS_OK) { return status; }
    
    status = acquireKey(job->keySlot, &key);
    if (status != EHSM_STATUS_OK) { return status; }
    
    status = performCrypto(job, &key);
    
    releaseKey(job->keySlot);
    return status;
}
```

#### 6.1.2 Cyclomatic Complexity

**Rule:** Maximum cyclomatic complexity is 10.

**Reasoning:** Complex functions are error-prone and hard to test.

### 6.2 Function Parameters

#### 6.2.1 Parameter Count

**Rule:** Maximum 4 parameters. Use struct for more.

```c
/* ❌ BAD: Too many parameters */
ehsm_status_t ehsm_api_encrypt(
    uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t outputSize,
    size_t* outputLen,
    uint8_t keySlot,
    ehsm_algorithm_t algo,
    uint32_t flags
);

/* ✅ GOOD: Parameter struct */
typedef struct {
    uint8_t* input;
    size_t inputLen;
    uint8_t* output;
    size_t outputSize;
    size_t* outputLen;
    uint8_t keySlot;
    ehsm_algorithm_t algo;
    uint32_t flags;
} ehsm_encryptParams_t;

ehsm_status_t ehsm_api_encrypt(ehsm_encryptParams_t* params);
```

#### 6.2.2 Pointer Parameters

**Rule:** Use `const` for input pointers. Mark output pointers clearly.

```c
/* ✅ GOOD: Clear intent */
ehsm_status_t ehsm_api_encrypt(
    const uint8_t* input,      /* Input: plaintext */
    size_t inputLen,
    uint8_t* output,           /* Output: ciphertext */
    size_t* outputLen          /* In/Out: buffer size / actual length */
);

/* ❌ BAD: Unclear */
ehsm_status_t ehsm_api_encrypt(
    uint8_t* input,
    uint8_t* output,
    size_t* len
);
```

#### 6.2.3 Parameter Validation

**Rule:** Validate all parameters at function entry. Return early on invalid input.

```c
ehsm_status_t ehsm_api_encrypt(const uint8_t* input, size_t len, uint8_t* output)
{
    /* Validate first */
    if (input == NULL) { return EHSM_STATUS_ERR_INVALID_PARAM; }
    if (output == NULL) { return EHSM_STATUS_ERR_INVALID_PARAM; }
    if (len == 0U) { return EHSM_STATUS_ERR_INVALID_PARAM; }
    if (len > EHSM_MAX_BLOCK_SIZE) { return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER; }
    
    /* Then proceed */
    return performEncryption(input, len, output);
}
```

### 6.3 Return Values

#### 6.3.1 Status Type

**Rule:** All functions must return `ehsm_status_t` (or layer-equivalent).

```c
/* ✅ GOOD: Consistent return type */
ehsm_status_t ehsm_api_init(void);
ehsm_status_t ehsm_api_encrypt(...);

/* ❌ BAD: Inconsistent */
int ehsm_api_init(void);           /* What does return value mean? */
bool ehsm_api_encrypt(...);        /* Success or data? */
```

#### 6.3.2 Single Exit Point

**Rule:** Prefer single exit point for cleanup consistency.

```c
/* ✅ GOOD: Single exit with cleanup */
ehsm_status_t ehsm_mw_process(ehsm_job_t* job)
{
    ehsm_status_t status;
    
    status = validate(job);
    if (status != EHSM_STATUS_OK) { goto cleanup; }
    
    status = acquireResources();
    if (status != EHSM_STATUS_OK) { goto cleanup; }
    
    status = execute(job);
    
cleanup:
    releaseResources();
    return status;
}
```

---

## 7. Memory Management Rules

### 7.1 Dynamic Allocation Prohibition

**Rule:** No dynamic memory allocation (`malloc`, `free`, `new`, `delete`).

**Reasoning:** Prevents fragmentation, non-determinism, and memory leaks.

```c
/* ❌ BAD: Dynamic allocation */
void ehsm_api_process(void)
{
    uint8_t* buffer = malloc(256);
    /* ... */
    free(buffer);  /* What if early return? */
}

/* ✅ GOOD: Static allocation */
static uint8_t g_buffer[256];

void ehsm_api_process(void)
{
    /* Use g_buffer */
}
```

### 7.2 Stack Usage

**Rule:** Limit stack usage. Document maximum stack requirements.

```c
/* ❌ BAD: Large stack allocation */
void ehsm_api_process(void)
{
    uint8_t largeBuffer[4096];  /* May overflow stack! */
}

/* ✅ GOOD: Static or heap (if absolutely necessary) */
static uint8_t g_largeBuffer[4096];
```

### 7.3 Buffer Overflow Prevention

**Rule:** Always validate buffer sizes before operations.

```c
/* ❌ BAD: No bounds checking */
void copyData(uint8_t* dest, const uint8_t* src, size_t len)
{
    memcpy(dest, src, len);  /* What if dest is smaller? */
}

/* ✅ GOOD: Bounds checking */
ehsm_status_t copyData(
    uint8_t* dest,
    size_t destSize,
    const uint8_t* src,
    size_t len
)
{
    if (len > destSize) { return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER; }
    (void)memcpy(dest, src, len);
    return EHSM_STATUS_OK;
}
```

### 7.4 Memory Initialization

**Rule:** Initialize all variables before use. Zero sensitive data after use.

```c
/* ❌ BAD: Uninitialized */
ehsm_key_t key;
getKey(&key);  /* What if getKey fails? */

/* ✅ GOOD: Initialized and cleared */
ehsm_key_t key;
(void)memset(&key, 0, sizeof(key));

ehsm_status_t status = getKey(&key);
if (status != EHSM_STATUS_OK) {
    /* key is already zeroed */
    return status;
}

/* Use key... */

/* Clear after use */
(void)memset(&key, 0, sizeof(key));
```

### 7.5 Sensitive Data Handling

**Rule:** Clear sensitive data from memory immediately after use.

```c
ehsm_status_t ehsm_api_decrypt(ehsm_job_t* job)
{
    ehsm_key_t key;
    ehsm_status_t status;
    
    status = getKey(job->keySlot, &key);
    if (status != EHSM_STATUS_OK) { return status; }
    
    /* Perform decryption */
    status = decryptWithKey(job, &key);
    
    /* CRITICAL: Clear key from memory */
    (void)memset(&key, 0, sizeof(key));
    volatile uint8_t* keyPtr = key.data;
    for (size_t i = 0; i < sizeof(key.data); i++) {
        keyPtr[i] = 0U;
    }
    
    return status;
}
```

---

## 8. Error Handling Rules

### 8.1 Status Codes

**Rule:** Use typed status enums. Never use bare integers.

```c
/* ✅ GOOD: Typed status */
typedef enum EhsmStatus {
    EHSM_STATUS_OK = 0,
    EHSM_STATUS_ERR_INVALID_PARAM = -1,
    EHSM_STATUS_ERR_CRYPTO_FAILED = -2
} ehsm_status_t;

ehsm_status_t ehsm_api_init(void);

/* Usage */
ehsm_status_t status = ehsm_api_init();
if (status != EHSM_STATUS_OK) {
    handleError(status);
}

/* ❌ BAD: Integer return */
int ehsm_api_init(void);  /* What does return value mean? */
```

### 8.2 Error Propagation

**Rule:** Propagate errors up the call stack. Handle only where recovery is possible.

```c
/* ✅ GOOD: Propagate with context */
ehsm_status_t ehsm_api_encrypt(ehsm_job_t* job)
{
    ehsm_status_t status;
    
    status = validateJob(job);
    if (status != EHSM_STATUS_OK) {
        return status;  /* Propagate */
    }
    
    status = acquireKey(job->keySlot, &key);
    if (status != EHSM_STATUS_OK) {
        ehsm_mcal_uartLog("[HSM] Key acquisition failed: %d", status);
        return status;  /* Propagate with logging */
    }
    
    return performEncryption(job, &key);
}
```

### 8.3 Error Logging

**Rule:** Log errors at the point of detection. Include context.

```c
/* ✅ GOOD: Contextual logging */
if (status != EHSM_STATUS_OK) {
    ehsm_mcal_uartLog(
        "[HSM] Encrypt failed: slot=%d, algo=%d, status=%d",
        job->keySlot,
        job->algorithm,
        status
    );
}
```

### 8.4 Defensive Programming

**Rule:** Use assertions for internal invariants. Never for input validation.

```c
/* ✅ GOOD: Assertion for invariant */
#include <assert.h>

ehsm_status_t ehsm_internalProcess(ehsm_job_t* job)
{
    assert(job != NULL);           /* Internal invariant */
    assert(job->state == READY);   /* Must be in READY state */
    
    /* Input validation (returns error, not assert) */
    if (job->input.data == NULL) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* ... */
}
```

---

## 9. Interrupt and Concurrency Rules

### 9.1 Interrupt Service Routines

**Rule:** ISRs must be minimal. Defer processing to main context.

```c
/* ✅ GOOD: Minimal ISR */
volatile bool g_uartRxReady = false;
volatile uint8_t g_uartRxByte;

void UART_RX_ISR(void)
{
    g_uartRxByte = uartReadByte();
    g_uartRxReady = true;
    /* ISR exits quickly */
}

/* Main loop processes */
void mainLoop(void)
{
    if (g_uartRxReady) {
        g_uartRxReady = false;
        processByte(g_uartRxByte);  /* Slow processing in main context */
    }
}
```

### 9.2 Shared Data Protection

**Rule:** Protect shared data between ISR and main context.

```c
/* ✅ GOOD: Critical section */
#include <stdint.h>

volatile uint32_t g_sharedCounter;

void incrementCounter(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();  /* Enter critical section */
    
    g_sharedCounter++;
    
    if (primask == 0) {
        __enable_irq();  /* Restore interrupt state */
    }
}
```

### 9.3 Volatile Usage

**Rule:** Use `volatile` for memory-mapped hardware and shared variables.

```c
/* ✅ GOOD: Proper volatile usage */
volatile uint32_t* const UART_DR = (uint32_t*)0x40013804U;  /* Hardware register */
volatile bool g_interruptFlag;                              /* Shared with ISR */

/* ❌ BAD: Missing volatile */
uint32_t* UART_DR;         /* Compiler may optimize incorrectly */
bool g_flag;              /* May be cached in register */
```

---

## 10. Hardware Abstraction Rules

### 10.1 Layer Boundaries

**Rule:** Upper layers must not access hardware directly.

```c
/* ❌ BAD: API layer accessing hardware */
#include "stm32f4xx.h"

ehsm_status_t ehsm_api_encrypt(...)
{
    USART1->DR = data;  /* Direct hardware access! */
}

/* ✅ GOOD: Through MCAL */
#include "ehsm_mcal_uart.h"

ehsm_status_t ehsm_api_encrypt(...)
{
    (void)ehsm_mcal_uartTxByte(data);  /* Through abstraction */
}
```

### 10.2 Register Access

**Rule:** Use memory-mapped structs for register access. No magic addresses.

```c
/* ✅ GOOD: Typed register access */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
} UartRegs_t;

#define UART1_BASE    (0x40013800U)
#define UART1         ((UartRegs_t*)UART1_BASE)

void uartTxByte(uint8_t data)
{
    while ((UART1->SR & 0x80U) == 0U) { }  /* Wait for TX empty */
    UART1->DR = data;
}

/* ❌ BAD: Magic addresses */
#define UART_DR  0x40013804U

void uartTxByte(uint8_t data)
{
    *(volatile uint32_t*)UART_DR = data;
}
```

### 10.3 Endianness

**Rule:** Explicitly handle endianness for network/storage operations.

```c
/* ✅ GOOD: Explicit endianness */
static inline uint32_t hostToNetwork32(uint32_t value)
{
    return ((value & 0x000000FFU) << 24) |
           ((value & 0x0000FF00U) << 8)  |
           ((value & 0x00FF0000U) >> 8)  |
           ((value & 0xFF000000U) >> 24);
}

/* ❌ BAD: Assumes host endianness */
uint32_t networkValue = hostValue;  /* Wrong on little-endian! */
```

---

## 11. Security Rules for HSM

### 11.1 Key Handling

**Rule:** Keys must never be exposed outside secure boundaries.

```c
/* ✅ GOOD: Key stays in secure service */
ehsm_status_t ehsm_api_encrypt(const uint8_t* input, size_t len, uint8_t* output)
{
    /* Key is retrieved, used, and cleared within this scope */
    ehsm_key_t key;
    (void)memset(&key, 0, sizeof(key));
    
    ehsm_status_t status = getKeyFromSecureStorage(&key);
    if (status == EHSM_STATUS_OK) {
        encryptWithKey(input, len, output, &key);
        secureClear(&key);  /* Explicit clear */
    }
    
    return status;
}

/* ❌ BAD: Key exposed to caller */
ehsm_status_t ehsm_keystore_getKey(uint8_t slotId, ehsm_key_t* keyOut);  /* Never! */
```

### 11.2 Constant-Time Operations

**Rule:** Cryptographic operations must execute in constant time.

```c
/* ❌ BAD: Variable-time comparison */
bool compareKeys(const uint8_t* a, const uint8_t* b, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return false;  /* Early exit leaks timing info! */
        }
    }
    return true;
}

/* ✅ GOOD: Constant-time comparison */
bool compareKeys(const uint8_t* a, const uint8_t* b, size_t len)
{
    uint8_t result = 0U;
    
    for (size_t i = 0; i < len; i++) {
        result |= a[i] ^ b[i];  /* Accumulate differences */
    }
    
    return (result == 0U);  /* Single comparison at end */
}
```

### 11.3 Input Validation

**Rule:** Validate all inputs from untrusted sources.

```c
ehsm_status_t ehsm_api_processCommand(const uint8_t* cmd, size_t len)
{
    /* Validate command source */
    if (!isTrustedSource()) {
        return EHSM_STATUS_ERR_AUTH_FAILED;
    }
    
    /* Validate command structure */
    if (len < EHSM_COMMAND_HEADER_SIZE) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Validate command type */
    if (cmd[0] > EHSM_CMD_MAX_VALID) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    /* Validate lengths within command */
    uint16_t payloadLen = (uint16_t)cmd[1] | ((uint16_t)cmd[2] << 8);
    if (payloadLen > (len - EHSM_COMMAND_HEADER_SIZE)) {
        return EHSM_STATUS_ERR_INVALID_PARAM;
    }
    
    return processValidatedCommand(cmd, len);
}
```

### 11.4 Secure Boot Considerations

**Rule:** Verify firmware integrity before execution (future enhancement).

---

## 12. Documentation Standards

### 12.1 Doxygen Comments

**Rule:** All public interfaces must have Doxygen documentation.

```c
/**
 * @brief Encrypt data using specified algorithm and key.
 * 
 * This function performs symmetric encryption on the input data
 * using the key stored in the specified key slot.
 * 
 * @param[in] sessionId Valid session identifier
 * @param[in] keySlotId ID of key slot containing encryption key
 * @param[in] algorithm Encryption algorithm to use
 * @param[in] input Pointer to plaintext data
 * @param[in] inputLen Length of input data in bytes
 * @param[out] output Pointer to ciphertext buffer
 * @param[in,out] outputLen Size of output buffer / actual ciphertext length
 * 
 * @return EHSM_STATUS_OK on success
 * @return EHSM_STATUS_ERR_INVALID_PARAM if parameters are invalid
 * @return EHSM_STATUS_ERR_INVALID_KEY_ID if key slot is invalid
 * @return EHSM_STATUS_ERR_INSUFFICIENT_BUFFER if output buffer is too small
 * @return EHSM_STATUS_ERR_CRYPTO_FAILED if encryption operation fails
 * 
 * @pre Session must be valid
 * @pre Key slot must have ENCRYPT permission
 * @post Output buffer contains ciphertext
 * 
 * @note Execution time is proportional to inputLen
 * @warning Output buffer must be at least inputLen + block_size
 * 
 * @example
 * ```c
 * uint8_t ciphertext[256];
 * size_t len = sizeof(ciphertext);
 * ehsm_status_t status = ehsm_api_encrypt(
 *     sessionId, 0, EHSM_ALGORITHM_AES_256,
 *     plaintext, plaintextLen,
 *     ciphertext, &len
 * );
 * ```
 */
ehsm_status_t ehsm_api_encrypt(
    ehsm_sessionId_t sessionId,
    uint8_t keySlotId,
    ehsm_algorithm_t algorithm,
    const uint8_t* input,
    size_t inputLen,
    uint8_t* output,
    size_t* outputLen
);
```

### 12.2 Inline Comments

**Rule:** Comment *why*, not *what*. Code should be self-documenting.

```c
/* ❌ BAD: Redundant comment */
i++;  /* Increment i */

/* ✅ GOOD: Explains rationale */
i++;  /* Skip header byte */

/* ✅ GOOD: Explains constraint */
if (retryCount > 3) {
    /* Hardware requires max 3 retries per specification */
    return EHSM_STATUS_ERR_TIMEOUT;
}
```

### 12.3 TODO Comments

**Rule:** Use standardized TODO format with owner and date.

```c
/* TODO [datdang 2026-03-12]: Implement hardware RNG integration */
/* FIXME [datdang 2026-03-12]: Handle edge case when buffer is exactly full */
/* NOTE [datdang 2026-03-12]: This is a mock implementation for PoC */
```

---

## 13. Testing Requirements

### 13.1 Unit Test Coverage

**Rule:** All public functions must have unit tests.

```c
/* test_crypto_service.c */
static void testEncryptNullInput(void)
{
    ehsm_status_t status = ehsm_crypto_encrypt(
        EHSM_ALGORITHM_AES_256, 0, NULL, &output
    );
    TEST_ASSERT_EQ(EHSM_STATUS_ERR_INVALID_PARAM, status);
}

static void testEncryptValidInput(void)
{
    /* Setup */
    importTestKey();
    
    /* Execute */
    ehsm_status_t status = ehsm_crypto_encrypt(
        EHSM_ALGORITHM_AES_256, TEST_SLOT, &input, &output
    );
    
    /* Verify */
    TEST_ASSERT_EQ(EHSM_STATUS_OK, status);
    TEST_ASSERT(output.length > 0);
    
    /* Cleanup */
    deleteTestKey();
}
```

### 13.2 HOST Testing

**Rule:** All code must compile and run on HOST for debugging.

```bash
# Build for HOST
cmake -DTARGET_PLATFORM=HOST -B build
cmake --build build

# Run tests
cd build && ctest

# Debug with GDB
gdb ./build/ehsm_host
(gdb) break ehsm_api_encrypt
(gdb) run
```

---

## 14. Static Analysis Rules

### 14.1 clang-tidy Configuration

**Rule:** All code must pass clang-tidy checks.

```yaml
# .clang-tidy
Checks: >
  -*,
  bugprone-*,
  cert-*,
  misc-*,
  performance-*,
  portability-*,
  readability-*,
  -readability-magic-numbers,
  -readability-identifier-length
WarningsAsErrors: '*'
```

### 14.2 Compiler Warnings

**Rule:** All warnings are errors. Zero warnings policy.

```cmake
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wsign-conversion
)
```

---

## 15. Anti-Patterns to Avoid

### 15.1 Embedded Anti-Patterns

| Anti-Pattern | Problem | Solution |
|--------------|---------|----------|
| **Magic Numbers** | Unclear intent, error-prone | Named constants |
| **Global Variables** | Hidden dependencies, race conditions | Encapsulated state |
| **Deep Nesting** | Hard to read, complex flow | Guard clauses, early returns |
| **Large Functions** | Multiple responsibilities | Decomposition |
| **Direct Hardware Access** | Non-portable, untestable | MCAL abstraction |
| **Busy Waiting** | Wastes CPU cycles | Interrupts or RTOS |
| **Ignoring Return Values** | Silent failures | Always check status |

### 15.2 Security Anti-Patterns

| Anti-Pattern | Problem | Solution |
|--------------|---------|----------|
| **Hard-coded Keys** | Keys exposed in binary | Secure key storage |
| **Timing-Variant Comparisons** | Timing attacks | Constant-time algorithms |
| **Unvalidated Input** | Buffer overflows, injection | Validate all inputs |
| **Sensitive Data in Clear** | Memory dumps expose secrets | Clear after use |
| **Predictable RNG** | Weak cryptography | Hardware RNG |

---

## Appendix A: Quick Reference Card

```
NAMING (NEW - camelCase):
  Functions:    ehsm_api_init(), ehsm_api_encrypt()
  Variables:    keySlotId, ciphertextLen
  Types:        ehsm_status_t, ehsm_context_t
  Constants:    EHSM_MAX_KEY_SLOTS, EHSM_AES_BLOCK_SIZE
  Enum Values:  EHSM_STATUS_OK, EHSM_STATUS_ERR_INVALID_PARAM

NAMING (OLD - DO NOT USE):
  Functions:    Hsm_Init(), Hsm_Encrypt()  ❌
  Variables:    key_slot_id, ciphertext_len  ❌
  Types:        Hsm_Status_t, Hsm_Context_t  ❌

FORMATTING:
  Indent:       4 spaces (no tabs)
  Line Length:  100 characters max
  Braces:       K&R style (opening brace on same line)
  Spaces:       After keywords, around operators

MEMORY:
  No malloc/free
  Initialize before use
  Clear sensitive data
  Validate buffer sizes

ERRORS:
  Return ehsm_status_t
  Validate parameters first
  Log with context
  Propagate up the stack

SECURITY:
  Constant-time comparisons
  Validate all inputs
  Clear keys after use
  Never expose key material
```

---

## Appendix B: Checklist for Code Review

- [ ] Uses `ehsm_<module>_camelCase()` naming
- [ ] Functions < 40 lines
- [ ] Parameters < 4 (or uses struct)
- [ ] All parameters validated
- [ ] All return values checked
- [ ] No dynamic memory allocation
- [ ] Sensitive data cleared
- [ ] Doxygen comments complete
- [ ] No compiler warnings
- [ ] Unit tests added/updated
- [ ] clang-tidy passes
- [ ] Works on HOST platform

---

**Document End**
