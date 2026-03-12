/**
 * @file status.cpp
 * @brief Status class implementation.
 * 
 * @copyright Copyright (c) 2024 Embedded HSM Project
 * @license MIT License
 */

#include "ehsm/types/status.hpp"

namespace ehsm::types {

const char* Status::message() const {
    return toString(code_);
}

const char* toString(StatusCode code) {
    switch (code) {
        case StatusCode::OK:
            return "OK";
        case StatusCode::ERR_INVALID_PARAM:
            return "Invalid parameter";
        case StatusCode::ERR_INSUFFICIENT_BUFFER:
            return "Insufficient buffer";
        case StatusCode::ERR_CRYPTO_FAILED:
            return "Crypto operation failed";
        case StatusCode::ERR_SESSION_INVALID:
            return "Invalid session";
        case StatusCode::ERR_NOT_INITIALIZED:
            return "Not initialized";
        case StatusCode::ERR_KEY_SLOT_FULL:
            return "Key slot full";
        case StatusCode::ERR_INVALID_KEY_ID:
            return "Invalid key ID";
        case StatusCode::ERR_AUTH_FAILED:
            return "Authentication failed";
        case StatusCode::ERR_QUEUE_FULL:
            return "Queue full";
        case StatusCode::ERR_HARDWARE:
            return "Hardware error";
        case StatusCode::ERR_UNSUPPORTED:
            return "Operation unsupported";
        case StatusCode::ERR_TIMEOUT:
            return "Operation timeout";
        case StatusCode::ERR_INTERNAL:
            return "Internal error";
        case StatusCode::ERR_BUSY:
            return "HSM is busy";
        default:
            return "Unknown error";
    }
}

} // namespace ehsm::types
