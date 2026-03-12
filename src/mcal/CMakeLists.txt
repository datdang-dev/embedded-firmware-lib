# =============================================================================
# MCAL Layer CMakeLists.txt
# =============================================================================
# Microcontroller Abstraction Layer - Conditionally compiles real UART driver
# for EMBEDDED platform or UART stub for HOST platform.
# =============================================================================

# MCAL source files based on platform
if(TARGET_PLATFORM STREQUAL "HOST")
    set(MCAL_SOURCES
        stubs/ehsm_mcal_uart_stub.c
    )
    message(STATUS "MCAL: Using UART stub for HOST platform")
else()
    set(MCAL_SOURCES
        mcal/ehsm_mcal_uart.c
    )
    message(STATUS "MCAL: Using real UART driver for EMBEDDED platform")
endif()

# Create MCAL library
add_library(ehsm_mcal STATIC ${MCAL_SOURCES})

target_include_directories(ehsm_mcal
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(TARGET_PLATFORM STREQUAL "HOST")
    target_compile_definitions(ehsm_mcal PUBLIC EHSM_MCAL_UART_STUB)
endif()
