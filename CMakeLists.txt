# =============================================================================
# Embedded HSM Firmware - Root CMakeLists.txt
# =============================================================================
# Enterprise-grade build system for Hardware Security Module firmware.
# Supports HOST (PC debugging with GDB) and EMBEDDED targets.
#
# Naming Convention: ehsm_<module>_camelCase()
#
# Build Examples:
#   HOST:   cmake -DTARGET_PLATFORM=HOST -B build && cmake --build build
#   EMBEDDED: cmake -DTARGET_PLATFORM=EMBEDDED -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -B build
#
# Targets:
#   ehsm_firmware   - Main firmware library
#   ehsm_host       - Host executable for testing
#   docs            - Generate Doxygen documentation
#   test            - Run unit tests (CTest)
# =============================================================================

cmake_minimum_required(VERSION 3.20)

project(embedded_hsm
    VERSION 1.0.0
    DESCRIPTION "Embedded Hardware Security Module Firmware"
    LANGUAGES C
)

# Enforce out-of-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Out-of-source builds required. Create a build directory.")
endif()

# Set C standard (C11 for embedded compatibility)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# Include CMake Modules
# =============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CompilerWarnings)
include(PlatformConfig)
include(Doxygen)
include(CTest)

# =============================================================================
# Configure Compiler Warnings
# =============================================================================
set_project_warnings(${PROJECT_NAME})

# Add platform-specific flags
if(DEFINED PLATFORM_C_FLAGS)
    add_compile_options(${PLATFORM_C_FLAGS})
endif()

if(DEFINED PLATFORM_LINK_FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PLATFORM_LINK_FLAGS}")
endif()

# =============================================================================
# Global Include Directories
# =============================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# =============================================================================
# Add Subdirectories (Layer-based Architecture)
# =============================================================================

# MCAL Layer - Microcontroller Abstraction
add_subdirectory(src/mcal)

# Driver Layer - Hardware Drivers
add_subdirectory(src/drivers)

# Service Layer - Core Business Logic
add_subdirectory(src/services)

# Middleware Layer - Job Router & Session Management
add_subdirectory(src/middleware)

# API Layer - Public Interfaces
add_subdirectory(src/api)

# =============================================================================
# Host Executable (for PC debugging)
# =============================================================================
if(TARGET_PLATFORM STREQUAL "HOST")
    add_executable(ehsm_host
        main.c
    )
    
    target_link_libraries(ehsm_host PRIVATE
        ehsm_api
        ehsm_middleware
        ehsm_services
        ehsm_drivers
        ehsm_mcal
    )
    
    message(STATUS "Host executable 'ehsm_host' configured")
endif()

# =============================================================================
# Unit Tests
# =============================================================================
if(BUILD_TESTING AND TARGET_PLATFORM STREQUAL "HOST")
    add_subdirectory(tests)
endif()

# =============================================================================
# Print Configuration Summary
# =============================================================================
message(STATUS "")
message(STATUS "=== Embedded HSM Configuration Summary ===")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Platform:       ${TARGET_PLATFORM}")
message(STATUS "  C Compiler:     ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "  C Standard:     C${CMAKE_C_STANDARD}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  Warnings:       ${WARNINGS_AS_ERRORS}")
message(STATUS "  Documentation:  ${BUILD_DOCS}")
message(STATUS "  Testing:        ${BUILD_TESTING}")
message(STATUS "  Naming Conv:    ehsm_<module>_camelCase()")
message(STATUS "==========================================")
message(STATUS "")
