/**
 * @file iuart.hpp
 * @brief UART interface.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "status.hpp"
#include <span>
#include <cstdarg>

namespace ehsm::mcal {

/**
 * @brief UART interface for serial communication.
 */
class IUart {
public:
    virtual ~IUart() = default;

    [[nodiscard]] virtual Status init() = 0;
    [[nodiscard]] virtual Status txByte(uint8_t data) = 0;
    [[nodiscard]] virtual Status txBuffer(std::span<const uint8_t> data) = 0;
    [[nodiscard]] virtual Status txString(const char* str) = 0;
    [[nodiscard]] virtual Status log(const char* format, ...) = 0;
    [[nodiscard]] virtual Status vlog(const char* format, va_list args) = 0;
};

} // namespace ehsm::mcal
