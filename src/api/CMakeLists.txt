# =============================================================================
# API Layer CMakeLists.txt
# =============================================================================

set(API_SOURCES
    ehsm_api.c
)

add_library(ehsm_api STATIC ${API_SOURCES})

target_include_directories(ehsm_api
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ehsm_api PUBLIC ehsm_middleware)
