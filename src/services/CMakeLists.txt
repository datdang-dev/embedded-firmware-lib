# =============================================================================
# Service Layer CMakeLists.txt
# =============================================================================

set(SERVICES_SOURCES
    ehsm_crypto.c
    ehsm_keystore.c
)

add_library(ehsm_services STATIC ${SERVICES_SOURCES})

target_include_directories(ehsm_services
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ehsm_services PUBLIC ehsm_drivers)
