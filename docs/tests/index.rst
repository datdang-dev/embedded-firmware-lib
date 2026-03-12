.. _tests_index:

=================
Test Documentation
=================

This section contains comprehensive test documentation for all Embedded HSM modules.

Test Strategy
=============

The testing strategy follows these principles:

* **Unit Tests:** Test individual public functions in isolation
* **Integration Tests:** Test API → Service layer integration
* **Coverage Target:** 100% C0/C1 for all public functions
* **Mock Framework:** Trompeloeil + Catch2

Test Structure
==============

.. toctree::
   :maxdepth: 1

   test_cases_crypto
   test_cases_keystore
   test_cases_session
   test_cases_api
   test_integration
   coverage_report

Test Case Template
==================

Each test case follows this template:

.. code-block:: rst

   Test Case: TC_<MODULE>_<NUMBER>
   ================================

   **Interface:** <InterfaceName>::<functionName>()

   **Purpose:** <What is being tested>

   **Preconditions:**
   - <condition 1>
   - <condition 2>

   **Test Steps:**
   1. <step 1>
   2. <step 2>

   **Expected Result:**
   - <expected outcome 1>
   - <expected outcome 2>

   **Coverage:**
   - Lines: <line numbers>
   - Branches: <branch coverage>

Test Traceability Matrix
========================

.. list-table:: Test Traceability
   :header-rows: 1
   :widths: 20 30 20 15 15

   * - Test Case
     - Interface/Function
     - Test Type
     - Coverage
     - Status
   * - TC_CRYPTO_001
     - ICryptoService::encrypt()
     - Unit
     - C0, C1
     - Pass
   * - TC_CRYPTO_002
     - ICryptoService::decrypt()
     - Unit
     - C0, C1
     - Pass
   * - TC_KEY_001
     - IKeystoreService::importKey()
     - Unit
     - C0, C1
     - Pass
   * - TC_KEY_002
     - IKeystoreService::getKey()
     - Unit
     - C0, C1
     - Pass
   * - TC_SESS_001
     - ISessionManager::createSession()
     - Unit
     - C0, C1
     - Pass
   * - IT_ENCRYPT_001
     - IHsmApi::encrypt()
     - Integration
     - N/A
     - Pass

Running Tests
=============

.. code-block:: bash

   # Build with tests
   cmake -DTARGET_PLATFORM=HOST -DBUILD_TESTING=ON -B build
   
   # Run all tests
   cd build && ctest
   
   # Run specific test
   ctest -R test_crypto_service
   
   # Run with verbose output
   ctest -V
   
   # Generate coverage report
   cmake --build build --target coverage

Coverage Requirements
=====================

* **C0 Coverage (Statement):** 100% for all public functions
* **C1 Coverage (Branch):** 100% for all public functions
* **Integration Tests:** No coverage requirement (functional testing)
