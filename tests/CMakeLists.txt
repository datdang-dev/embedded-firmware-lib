# =============================================================================
# Tests CMakeLists.txt
# =============================================================================
# Unit tests using CTest framework (HOST platform only).
# =============================================================================

enable_testing()

# Unit test for crypto service
add_executable(test_crypto_service unit/test_crypto_service.c)
target_link_libraries(test_crypto_service PRIVATE hsm_services)
add_test(NAME CryptoServiceTest COMMAND test_crypto_service)

# Unit test for keystore
add_executable(test_keystore unit/test_keystore.c)
target_link_libraries(test_keystore PRIVATE hsm_services)
add_test(NAME KeystoreTest COMMAND test_keystore)

message(STATUS "Unit tests configured")
