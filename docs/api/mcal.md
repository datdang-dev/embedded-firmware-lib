# MCAL (Hardware Abstraction Layer) Documentation

## Overview

The MCAL (Microcontroller Abstraction Layer) provides hardware abstraction for peripheral devices. It isolates hardware-specific code from the rest of the firmware, enabling portability across different platforms.

**Files:**
- `iuart.hpp` - UART interface
- `uart_stub.hpp` - UART stub for HOST platform

---

## File: iuart.hpp

### Overview

Abstract interface for UART (Universal Asynchronous Receiver-Transmitter) serial communication. This interface enables logging and serial communication across different hardware platforms.

### Class: IUart

**Purpose:** UART interface for serial communication.

**Pattern:** Hardware Abstraction Interface

**Namespace:** `ehsm::mcal`

#### Lifecycle Methods

##### init()

**Purpose:** Initialize the UART peripheral.

**Signature:**
```cpp
[[nodiscard]] virtual Status init() = 0;
```

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Initialization successful |
| `ERR_HARDWARE` | Hardware initialization failed |
| `ERR_NOT_INITIALIZED` | Dependencies not ready |

**Thread Safety:** No - Call once during system initialization.

---

#### Transmission Methods

##### txByte()

**Purpose:** Transmit a single byte over UART.

**Signature:**
```cpp
[[nodiscard]] virtual Status txByte(uint8_t data) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `data` | [in] | Byte to transmit |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Transmission successful |
| `ERR_HARDWARE` | Hardware transmission failed |
| `ERR_NOT_INITIALIZED` | UART not initialized |
| `ERR_TIMEOUT` | Transmission timeout |

**Usage Example:**
```cpp
Status result = uart->txByte('A');
if (result.isOk()) {
    // Byte transmitted
}
```

**Thread Safety:** Depends on implementation. Hardware UART may require synchronization.

##### txBuffer()

**Purpose:** Transmit a buffer of bytes over UART.

**Signature:**
```cpp
[[nodiscard]] virtual Status txBuffer(std::span<const uint8_t> data) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `data` | [in] | Buffer to transmit |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Transmission successful |
| `ERR_HARDWARE` | Hardware transmission failed |
| `ERR_NOT_INITIALIZED` | UART not initialized |
| `ERR_TIMEOUT` | Transmission timeout |

**Usage Example:**
```cpp
std::array<uint8_t, 8> buffer = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
Status result = uart->txBuffer(buffer);
if (result.isOk()) {
    // Buffer transmitted
}
```

**Thread Safety:** Depends on implementation.

##### txString()

**Purpose:** Transmit a null-terminated string over UART.

**Signature:**
```cpp
[[nodiscard]] virtual Status txString(const char* str) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `str` | [in] | Null-terminated string to transmit |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Transmission successful |
| `ERR_INVALID_PARAM` | Null pointer passed |
| `ERR_HARDWARE` | Hardware transmission failed |
| `ERR_NOT_INITIALIZED` | UART not initialized |

**Usage Example:**
```cpp
Status result = uart->txString("Hello, World!\n");
if (result.isOk()) {
    // String transmitted
}
```

**Thread Safety:** Depends on implementation.

---

#### Logging Methods

##### log()

**Purpose:** Log a formatted message over UART (printf-style).

**Signature:**
```cpp
[[nodiscard]] virtual Status log(const char* format, ...) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `format` | [in] | Format string (printf-style) |
| `...` | [in] | Variable arguments |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Logging successful |
| `ERR_INVALID_PARAM` | Invalid format string |
| `ERR_HARDWARE` | Hardware transmission failed |
| `ERR_NOT_INITIALIZED` | UART not initialized |

**Usage Example:**
```cpp
uart->log("System initialized\n");
uart->log("Value: %d, Name: %s\n", 42, "test");
uart->log("Hex: 0x%08X\n", 0xDEADBEEF);
```

**Thread Safety:** Depends on implementation. Use external synchronization for concurrent logging.

##### vlog()

**Purpose:** Log a formatted message with va_list (for internal use).

**Signature:**
```cpp
[[nodiscard]] virtual Status vlog(const char* format, va_list args) = 0;
```

**Parameters:**
| Parameter | Direction | Description |
|-----------|-----------|-------------|
| `format` | [in] | Format string |
| `args` | [in] | Variable argument list |

**Return Codes:**
| Code | Description |
|------|-------------|
| `OK` | Logging successful |
| `ERR_INVALID_PARAM` | Invalid format string |
| `ERR_HARDWARE` | Hardware transmission failed |

**Usage Example:**
```cpp
void myLogFunction(const char* format, ...) {
    va_list args;
    va_start(args, format);
    uart->vlog(format, args);
    va_end(args);
}
```

**Thread Safety:** Not thread-safe. va_list cannot be safely shared between threads.

---

### Usage Example

```cpp
#include "iuart.hpp"

class UartExample {
public:
    Status run(std::shared_ptr<ehsm::mcal::IUart> uart) {
        // Initialize UART
        Status status = uart->init();
        if (!status.isOk()) {
            return status;
        }
        
        // Log startup message
        uart->log("=== System Startup ===\n");
        
        // Transmit single byte
        uart->txByte(0x55);
        
        // Transmit buffer
        std::array<uint8_t, 4> header = {0xAA, 0xBB, 0xCC, 0xDD};
        uart->txBuffer(header);
        
        // Transmit string
        uart->txString("Operation complete\n");
        
        // Formatted logging
        int value = 42;
        const char* name = "sensor";
        uart->log("Sensor %s reading: %d\n", name, value);
        
        return Status(ehsm::types::StatusCode::OK);
    }
};
```

---

## File: uart_stub.hpp

### Overview

UART stub implementation for HOST platform (non-embedded). This class wraps C-style UART functions and provides a C++ interface for testing and development on desktop systems.

### Class: UartStub

**Purpose:** UART stub implementation for HOST platform.

**Pattern:** Hardware Abstraction (stub implementation)

**Inheritance:** `public IUart`

**Namespace:** `ehsm::mcal`

#### C Interface Functions

The stub wraps these C functions (extern "C"):

```cpp
extern "C" {
    ehsm_status_t ehsm_mcal_uartInit(void);
    ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data);
    ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length);
    ehsm_status_t ehsm_mcal_uartTxString(const char* str);
    ehsm_status_t ehsm_mcal_uartLog(const char* format, ...);
}
```

#### Implementation Details

##### init()

```cpp
Status init() override {
    return convertStatus(ehsm_mcal_uartInit());
}
```

##### txByte()

```cpp
Status txByte(uint8_t data) override {
    return convertStatus(ehsm_mcal_uartTxByte(data));
}
```

##### txBuffer()

```cpp
Status txBuffer(std::span<const uint8_t> data) override {
    return convertStatus(ehsm_mcal_uartTxBuffer(data.data(), data.size()));
}
```

##### txString()

```cpp
Status txString(const char* str) override {
    return convertStatus(ehsm_mcal_uartTxString(str));
}
```

##### log()

```cpp
Status log(const char* format, ...) override {
    va_list args;
    va_start(args, format);
    Status status = vlog(format, args);
    va_end(args);
    return status;
}
```

##### vlog()

```cpp
Status vlog(const char* format, va_list args) override {
    // For C stub, we just call the C function directly
    // This is a limitation - in real code, implement properly
    (void)format;
    (void)args;
    return Status(types::StatusCode::OK);
}
```

**Note:** The current implementation is a placeholder. In production, this should properly format and transmit the log message.

#### Private Methods

##### convertStatus()

**Purpose:** Convert C status code to C++ Status.

**Signature:**
```cpp
static Status convertStatus(ehsm_status_t cStatus);
```

**Conversion Table:**

| C Status | C++ Status |
|----------|------------|
| 0 | `StatusCode::OK` |
| -1 | `StatusCode::ERR_INVALID_PARAM` |
| -2 | `StatusCode::ERR_NOT_INITIALIZED` |
| Other | `StatusCode::ERR_INTERNAL` |

---

### Usage Example

```cpp
#include "uart_stub.hpp"

using namespace ehsm::mcal;

int main() {
    // Create UART stub
    UartStub uart;
    
    // Initialize
    Status status = uart.init();
    if (!status.isOk()) {
        return -1;
    }
    
    // Use UART
    uart.log("Running on HOST platform\n");
    uart.txString("Test message\n");
    
    std::array<uint8_t, 4> data = {0x01, 0x02, 0x03, 0x04};
    uart.txBuffer(data);
    
    return 0;
}
```

---

### Thread Safety

**Partial** - The stub implementation delegates to underlying C functions. Thread safety depends on the C implementation:
- If C functions are thread-safe, the stub is thread-safe
- If C functions use shared resources, external synchronization may be required

---

### Platform Support

| Platform | Implementation |
|----------|----------------|
| HOST (Linux/Windows/macOS) | `UartStub` - wraps C functions |
| Embedded (STM32, etc.) | Platform-specific implementation |

---

### Creating Platform-Specific Implementations

To create a UART implementation for a new platform:

```cpp
// stm32_uart.hpp
#pragma once

#include "iuart.hpp"

namespace ehsm::mcal {

class Stm32Uart final : public IUart {
public:
    explicit Stm32Uart(UART_HandleTypeDef* huart) : huart_(huart) {}
    
    Status init() override {
        // STM32-specific initialization
        if (HAL_UART_Init(huart_) == HAL_OK) {
            return Status(types::StatusCode::OK);
        }
        return Status(types::StatusCode::ERR_HARDWARE);
    }
    
    Status txByte(uint8_t data) override {
        if (HAL_UART_Transmit(huart_, &data, 1, 100) == HAL_OK) {
            return Status(types::StatusCode::OK);
        }
        return Status(types::StatusCode::ERR_TIMEOUT);
    }
    
    Status txBuffer(std::span<const uint8_t> data) override {
        if (HAL_UART_Transmit(huart_, data.data(), data.size(), 1000) == HAL_OK) {
            return Status(types::StatusCode::OK);
        }
        return Status(types::StatusCode::ERR_TIMEOUT);
    }
    
    Status txString(const char* str) override {
        if (str == nullptr) {
            return Status(types::StatusCode::ERR_INVALID_PARAM);
        }
        return txBuffer(std::as_bytes(std::span(str, strlen(str))));
    }
    
    Status log(const char* format, ...) override {
        // Implement formatted logging
        // ...
        return Status(types::StatusCode::OK);
    }
    
    Status vlog(const char* format, va_list args) override {
        // Implement va_list logging
        // ...
        return Status(types::StatusCode::OK);
    }
    
private:
    UART_HandleTypeDef* huart_;
};

} // namespace ehsm::mcal
```

---

## MCAL Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│                                                              │
│    ┌──────────────────────────────────────────────────┐     │
│    │              HSM API / Services                  │     │
│    └──────────────────────────────────────────────────┘     │
│                            │                                 │
│              ┌─────────────▼─────────────┐                  │
│              │      IUart Interface      │                  │
│              │      (Abstraction)        │                  │
│              └─────────────┬─────────────┘                  │
│                            │                                 │
│         ┌──────────────────┴──────────────────┐             │
│         │                                     │             │
│ ┌───────▼────────┐                  ┌────────▼────────┐    │
│ │   UartStub     │                  │  Stm32Uart      │    │
│ │ (HOST)         │                  │  (STM32)        │    │
│ └────────────────┘                  └─────────────────┘    │
│         │                                     │             │
│         ▼                                     ▼             │
│ ┌────────────────┐                  ┌─────────────────┐    │
│ │ C Functions    │                  │ HAL_UART_*      │    │
│ │ ehsm_mcal_*    │                  │ (STM32 HAL)     │    │
│ └────────────────┘                  └─────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

---

## Complete MCAL Example

```cpp
#include "common.hpp"
#include "iuart.hpp"
#include "uart_stub.hpp"

using namespace ehsm;

class McalExample {
public:
    Status run() {
        // Create UART implementation (stub for HOST)
        auto uart = std::make_shared<mcal::UartStub>();
        
        // Initialize
        Status status = uart->init();
        if (!status.isOk()) {
            uart->txString("Failed to initialize UART\n");
            return status;
        }
        
        // Log system info
        uart->log("=== MCAL Example ===\n");
        uart->log("Platform: HOST\n");
        uart->log("Timestamp: %lu\n", static_cast<unsigned long>(time(nullptr)));
        
        // Send binary data
        std::array<uint8_t, 8> header = {
            0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11
        };
        status = uart->txBuffer(header);
        if (!status.isOk()) {
            uart->log("Buffer transmission failed: %s\n", 
                      types::toString(status.code()));
            return status;
        }
        
        // Send text
        status = uart->txString("Transmission complete\n");
        
        // Debug logging
        #ifdef DEBUG
        uart->log("DEBUG: All operations successful\n");
        #endif
        
        return Status(types::StatusCode::OK);
    }
};

// Platform-specific factory
std::shared_ptr<mcal::IUart> createUart() {
    #ifdef PLATFORM_HOST
        return std::make_shared<mcal::UartStub>();
    #elif defined(PLATFORM_STM32)
        extern UART_HandleTypeDef huart1;
        return std::make_shared<mcal::Stm32Uart>(&huart1);
    #else
        #error "Unsupported platform"
    #endif
}
```

---

## Related Files

| File | Relationship |
|------|--------------|
| [types.md](types.md) | Uses Status type |
| [api_layer.md](api_layer.md) | API layer may use UART for logging |

---

## Cross-References

- `common.hpp` - Includes standard headers used by MCAL
- `status.hpp` - Status type for error handling
- `key_slot.hpp` - Defines system constants

---

## Integration Notes

### Using MCAL in Your Code

1. **Include the interface:**
   ```cpp
   #include "iuart.hpp"
   ```

2. **Inject the implementation:**
   ```cpp
   void myFunction(std::shared_ptr<mcal::IUart> uart);
   ```

3. **Use in production:**
   ```cpp
   auto uart = createUart();  // Platform-specific factory
   myFunction(uart);
   ```

### Testing with Mocks

For unit testing, create a mock UART implementation:

```cpp
class MockUart : public mcal::IUart {
public:
    Status init() override {
        initCalled_++;
        return Status(types::StatusCode::OK);
    }
    
    Status txByte(uint8_t data) override {
        txBytes_.push_back(data);
        return Status(types::StatusCode::OK);
    }
    
    Status txBuffer(std::span<const uint8_t> data) override {
        txBytes_.insert(txBytes_.end(), data.begin(), data.end());
        return Status(types::StatusCode::OK);
    }
    
    Status txString(const char* str) override {
        txStrings_.push_back(str);
        return Status(types::StatusCode::OK);
    }
    
    Status log(const char* format, ...) override {
        logCalls_++;
        return Status(types::StatusCode::OK);
    }
    
    Status vlog(const char* format, va_list args) override {
        (void)format;
        (void)args;
        return Status(types::StatusCode::OK);
    }
    
    // Test assertions
    int initCalled_ = 0;
    int logCalls_ = 0;
    std::vector<uint8_t> txBytes_;
    std::vector<std::string> txStrings_;
};
```
