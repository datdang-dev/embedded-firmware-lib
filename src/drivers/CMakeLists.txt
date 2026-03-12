# =============================================================================
# Driver Layer CMakeLists.txt
# =============================================================================

set(DRIVER_SOURCES
    ehsm_storage_driver.c
)

add_library(ehsm_drivers STATIC ${DRIVER_SOURCES})

target_include_directories(ehsm_drivers
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ehsm_drivers PUBLIC ehsm_mcal)
