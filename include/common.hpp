/**
 * @file common.hpp
 * @brief Common includes for all C++ code.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#pragma once

#include "status.hpp"
#include "algorithm.hpp"
#include "key_slot.hpp"

#include <cstdint>
#include <cstddef>
#include <cstdarg>
#include <span>
#include <memory>
#include <array>

// Bring Status into global namespace for convenience
using ehsm::types::Status;
using ehsm::types::StatusCode;
