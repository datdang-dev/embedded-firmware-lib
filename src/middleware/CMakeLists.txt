# =============================================================================
# Middleware Layer CMakeLists.txt
# =============================================================================

set(MIDDLEWARE_SOURCES
    ehsm_middleware.c
)

add_library(ehsm_middleware STATIC ${MIDDLEWARE_SOURCES})

target_include_directories(ehsm_middleware
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ehsm_middleware PUBLIC ehsm_services)
