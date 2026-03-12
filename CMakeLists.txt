# =============================================================================
# Embedded HSM Firmware - CMakeLists.txt (C++ Version)
# =============================================================================

cmake_minimum_required(VERSION 3.20)

project(embedded_hsm_cpp
    VERSION 1.0.0
    DESCRIPTION "Embedded HSM Firmware - C++ Implementation"
    LANGUAGES C CXX
)

# C++20 standard (required for std::span)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C standard for MCAL
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Out-of-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Out-of-source builds required.")
endif()

# Platform selection
set(TARGET_PLATFORM "HOST" CACHE STRING "Target platform: HOST or EMBEDDED")
set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS HOST EMBEDDED)

# Compiler warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror")

# Platform-specific flags
if(TARGET_PLATFORM STREQUAL "HOST")
    message(STATUS "Configuring for HOST platform")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    add_compile_definitions(TARGET_PLATFORM_HOST)
else()
    message(STATUS "Configuring for EMBEDDED platform")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    add_compile_definitions(TARGET_PLATFORM_EMBEDDED)
endif()

# Include directories - flat structure
include_directories(${CMAKE_SOURCE_DIR}/include)

# =============================================================================
# Source Files
# =============================================================================

# Types
set(TYPES_SOURCES
    src/types/status.cpp
)

# Middleware
set(MIDDLEWARE_SOURCES
    src/middleware/session_manager.cpp
)

# Services
set(SERVICES_SOURCES
    src/services/crypto/crypto_service.cpp
    src/services/crypto/aes128_algorithm.cpp
    src/services/crypto/aes256_algorithm.cpp
    src/services/keystore_service.cpp
)

# API
set(API_SOURCES
    src/api/hsm_api_impl.cpp
)

# MCAL (C code)
if(TARGET_PLATFORM STREQUAL "HOST")
    set(MCAL_SOURCES
        src/mcal/uart/mcal_uart_stub.c
    )
    message(STATUS "MCAL: Using UART stub for HOST")
else()
    set(MCAL_SOURCES
        src/mcal/uart/mcal_uart_real.c
    )
    message(STATUS "MCAL: Using real UART driver for EMBEDDED")
endif()

# =============================================================================
# Libraries
# =============================================================================

# MCAL library (C)
add_library(ehsm_mcal STATIC ${MCAL_SOURCES})
target_include_directories(ehsm_mcal PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Services library
add_library(ehsm_services STATIC ${SERVICES_SOURCES})
target_include_directories(ehsm_services PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ehsm_services PUBLIC ehsm_mcal)

# Middleware library
add_library(ehsm_middleware STATIC ${MIDDLEWARE_SOURCES})
target_include_directories(ehsm_middleware PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ehsm_middleware PUBLIC ehsm_services)

# API library
add_library(ehsm_api STATIC ${API_SOURCES} ${TYPES_SOURCES})
target_include_directories(ehsm_api PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ehsm_api PUBLIC ehsm_middleware)

# =============================================================================
# Host Executable
# =============================================================================

if(TARGET_PLATFORM STREQUAL "HOST")
    add_executable(ehsm_host main.cpp)
    target_link_libraries(ehsm_host PRIVATE
        ehsm_api
        ehsm_middleware
        ehsm_services
        ehsm_mcal
    )
    message(STATUS "Host executable 'ehsm_host' configured")
endif()

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "=== Embedded HSM C++ Configuration ===")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  Platform:     ${TARGET_PLATFORM}")
message(STATUS "  C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "  C Standard:   C${CMAKE_C_STANDARD}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "======================================")
message(STATUS "")
