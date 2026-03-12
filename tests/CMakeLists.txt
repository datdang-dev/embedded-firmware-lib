# =============================================================================
# Tests CMakeLists.txt
# =============================================================================
# Unit tests using Catch2 + Trompeloeil framework (HOST platform only).
# =============================================================================

enable_testing()

# =============================================================================
# Fetch Catch2 and Trompeloeil
# =============================================================================

include(FetchContent)

# Fetch Catch2
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(catch2)

# Fetch Trompeloeil
FetchContent_Declare(
    trompeloeil
    GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
    GIT_TAG        v47
)
FetchContent_MakeAvailable(trompeloeil)

# =============================================================================
# Test executable: test_crypto_service
# =============================================================================
add_executable(test_crypto_service
    unit/test_crypto_service.cpp
)
target_link_libraries(test_crypto_service PRIVATE
    ehsm_services
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_crypto_service PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_crypto_service PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME CryptoServiceTest COMMAND test_crypto_service)

# =============================================================================
# Test executable: test_keystore_service
# =============================================================================
add_executable(test_keystore_service
    unit/test_keystore_service.cpp
)
target_link_libraries(test_keystore_service PRIVATE
    ehsm_services
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_keystore_service PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_keystore_service PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME KeystoreServiceTest COMMAND test_keystore_service)

# =============================================================================
# Test executable: test_aes_algorithms
# =============================================================================
add_executable(test_aes_algorithms
    unit/test_aes_algorithms.cpp
)
target_link_libraries(test_aes_algorithms PRIVATE
    ehsm_services
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_aes_algorithms PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_aes_algorithms PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME AesAlgorithmsTest COMMAND test_aes_algorithms)

# =============================================================================
# Test executable: test_session_manager
# =============================================================================
add_executable(test_session_manager
    unit/test_session_manager.cpp
)
target_link_libraries(test_session_manager PRIVATE
    ehsm_middleware
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_session_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_session_manager PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME SessionManagerTest COMMAND test_session_manager)

# =============================================================================
# Test executable: test_api_layer
# =============================================================================
add_executable(test_api_layer
    unit/test_api_layer.cpp
)
target_link_libraries(test_api_layer PRIVATE
    ehsm_api
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_api_layer PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_api_layer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME ApiLayerTest COMMAND test_api_layer)

# =============================================================================
# Test executable: test_integration (Real implementations, no mocks)
# =============================================================================
add_executable(test_integration
    integration/test_integration.cpp
)
target_link_libraries(test_integration PRIVATE
    ehsm_api
    ehsm_middleware
    ehsm_services
    ehsm_mcal
    Catch2::Catch2WithMain
    trompeloeil::trompeloeil
)
target_include_directories(test_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/mocks
)
set_target_properties(test_integration PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
add_test(NAME IntegrationTest COMMAND test_integration)

# =============================================================================
# CTest configuration
# =============================================================================

# Set timeout for tests
set_tests_properties(CryptoServiceTest KeystoreServiceTest AesAlgorithmsTest SessionManagerTest ApiLayerTest IntegrationTest
    PROPERTIES TIMEOUT 30
)

# Enable parallel test execution
set_property(DIRECTORY PROPERTY CTEST_PARALLEL_LEVEL 4)

message(STATUS "Unit tests configured with Catch2 + Trompeloeil")
