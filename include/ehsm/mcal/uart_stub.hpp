/**
 * @file uart_stub.hpp
 * @brief UART stub C++ wrapper for HOST platform.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "ehsm/mcal/iuart.hpp"
#include <cstdint>

extern "C" {
    ehsm_status_t ehsm_mcal_uartInit(void);
    ehsm_status_t ehsm_mcal_uartTxByte(uint8_t data);
    ehsm_status_t ehsm_mcal_uartTxBuffer(const uint8_t* data, size_t length);
    ehsm_status_t ehsm_mcal_uartTxString(const char* str);
    ehsm_status_t ehsm_mcal_uartLog(const char* format, ...);
}

namespace ehsm::mcal {

/**
 * @brief UART stub implementation for HOST platform.
 */
class UartStub final : public IUart {
public:
    Status init() override {
        return convertStatus(ehsm_mcal_uartInit());
    }

    Status txByte(uint8_t data) override {
        return convertStatus(ehsm_mcal_uartTxByte(data));
    }

    Status txBuffer(std::span<const uint8_t> data) override {
        return convertStatus(ehsm_mcal_uartTxBuffer(data.data(), data.size()));
    }

    Status txString(const char* str) override {
        return convertStatus(ehsm_mcal_uartTxString(str));
    }

    Status log(const char* format, ...) override {
        va_list args;
        va_start(args, format);
        Status status = vlog(format, args);
        va_end(args);
        return status;
    }

    Status vlog(const char* format, va_list args) override {
        // For C stub, we just call the C function directly
        // This is a limitation - in real code, implement properly
        (void)format;
        (void)args;
        return Status(types::StatusCode::OK);
    }

private:
    static Status convertStatus(ehsm_status_t cStatus) {
        switch (cStatus) {
            case 0: return Status(types::StatusCode::OK);
            case -1: return Status(types::StatusCode::ERR_INVALID_PARAM);
            case -2: return Status(types::StatusCode::ERR_NOT_INITIALIZED);
            default: return Status(types::StatusCode::ERR_INTERNAL);
        }
    }
};

} // namespace ehsm::mcal
