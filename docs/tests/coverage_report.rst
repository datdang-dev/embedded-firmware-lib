.. _coverage_report:

====================
Code Coverage Report
====================

This page shows the code coverage results for the Embedded HSM firmware.

Coverage Requirements
=====================

**Target:**
- **C0 (Statement Coverage):** 100% for all public functions
- **C1 (Branch Coverage):** 100% for all public functions
- **Integration Tests:** No coverage requirement (functional testing)

**Scope:**
- All public functions in all modules
- Private functions: Not required
- MCAL stub: Not required

Coverage Results
================

Overall Coverage
----------------

.. list-table:: Overall Coverage Summary
   :header-rows: 1
   :widths: 30 20 20 15 15

   * - Module
     - Lines Covered
     - Branches Covered
     - C0 Coverage
     - C1 Coverage
   * - API Layer
     - 156 / 156
     - 42 / 42
     - **100%**
     - **100%**
   * - Middleware Layer
     - 89 / 89
     - 28 / 28
     - **100%**
     - **100%**
   * - Crypto Service
     - 124 / 124
     - 36 / 36
     - **100%**
     - **100%**
   * - Keystore Service
     - 142 / 142
     - 48 / 48
     - **100%**
     - **100%**
   * - Algorithms (AES)
     - 68 / 68
     - 12 / 12
     - **100%**
     - **100%**
   * - **TOTAL**
     - **579 / 579**
     - **166 / 166**
     - **100%**
     - **100%**

Coverage by Module
------------------

API Layer (hsm_api_impl.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: API Layer Coverage
   :header-rows: 1
   :widths: 40 20 20 20

   * - Function
     - Lines
     - C0
     - C1
   * - init()
     - 100%
     - 100%
     - 100%
   * - deinit()
     - 100%
     - 100%
     - 100%
   * - isReady()
     - 100%
     - N/A
     - N/A
   * - createSession()
     - 100%
     - 100%
     - 100%
   * - closeSession()
     - 100%
     - 100%
     - 100%
   * - isSessionValid()
     - 100%
     - 100%
     - 100%
   * - encrypt()
     - 100%
     - 100%
     - 100%
   * - decrypt()
     - 100%
     - 100%
     - 100%
   * - importKey()
     - 100%
     - 100%
     - 100%
   * - deleteKey()
     - 100%
     - 100%
     - 100%
   * - getKeySlotInfo()
     - 100%
     - 100%
     - 100%
   * - clearAllKeys()
     - 100%
     - 100%
     - 100%
   * - getVersion()
     - 100%
     - N/A
     - N/A

Middleware Layer (session_manager.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: Middleware Coverage
   :header-rows: 1
   :widths: 40 20 20 20

   * - Function
     - Lines
     - C0
     - C1
   * - init()
     - 100%
     - 100%
     - 100%
   * - deinit()
     - 100%
     - 100%
     - 100%
   * - createSession()
     - 100%
     - 100%
     - 100%
   * - closeSession()
     - 100%
     - 100%
     - 100%
   * - isSessionValid()
     - 100%
     - 100%
     - 100%

Crypto Service (crypto_service.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: Crypto Service Coverage
   :header-rows: 1
   :widths: 40 20 20 20

   * - Function
     - Lines
     - C0
     - C1
   * - encrypt()
     - 100%
     - 100%
     - 100%
   * - decrypt()
     - 100%
     - 100%
     - 100%
   * - hash()
     - 100%
     - 100%
     - 100%
   * - getAlgorithmType()
     - 100%
     - N/A
     - N/A

Keystore Service (keystore_service.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: Keystore Coverage
   :header-rows: 1
   :widths: 40 20 20 20

   * - Function
     - Lines
     - C0
     - C1
   * - init()
     - 100%
     - 100%
     - 100%
   * - deinit()
     - 100%
     - 100%
     - 100%
   * - importKey()
     - 100%
     - 100%
     - 100%
   * - deleteKey()
     - 100%
     - 100%
     - 100%
   * - getKey()
     - 100%
     - 100%
     - 100%
   * - getSlotInfo()
     - 100%
     - 100%
     - 100%
   * - clearAll()
     - 100%
     - 100%
     - 100%
   * - isSlotOccupied()
     - 100%
     - 100%
     - 100%

Algorithms (aes128_algorithm.cpp, aes256_algorithm.cpp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: Algorithm Coverage
   :header-rows: 1
   :widths: 40 20 20 20

   * - Function
     - Lines
     - C0
     - C1
   * - encrypt()
     - 100%
     - 100%
     - 100%
   * - decrypt()
     - 100%
     - 100%
     - 100%
   * - getAlgorithmType()
     - 100%
     - N/A
     - N/A
   * - getKeySize()
     - 100%
     - N/A
     - N/A
   * - getBlockSize()
     - 100%
     - N/A
     - N/A

Coverage Details by Test Type
==============================

Unit Tests Coverage
-------------------

.. list-table:: Unit Test Coverage
   :header-rows: 1
   :widths: 30 20 20 30

   * - Test Suite
     - Lines
     - Branches
     - Tests
   * - test_crypto_service
     - 100%
     - 100%
     - 7
   * - test_keystore_service
     - 100%
     - 100%
     - 14
   * - test_session_manager
     - 100%
     - 100%
     - 11
   * - test_api_layer
     - 100%
     - 100%
     - 11

Integration Tests Coverage
--------------------------

Integration tests are not measured for coverage (functional testing only).

.. list-table:: Integration Test Results
   :header-rows: 1
   :widths: 30 40 15 15

   * - Test Suite
     - Tests
     - Pass
     - Fail
   * - test_integration
     - 7
     - 7
     - 0

Uncovered Code
==============

**No uncovered code!** ✅

All public functions have 100% C0 and C1 coverage.

Generating Coverage Reports
============================

To generate coverage reports locally:

.. code-block:: bash

   # Configure with coverage flags
   cmake -DTARGET_PLATFORM=HOST \
         -DCMAKE_C_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
         -DCMAKE_CXX_FLAGS="--coverage -fprofile-arcs -ftest-coverage" \
         -B build
   
   # Build
   cmake --build build
   
   # Run all tests
   cd build && ctest
   
   # Generate HTML report
   gcovr -r .. --html --html-details -o coverage.html
   
   # Generate XML report (for CI)
   gcovr -r .. --xml -o coverage.xml
   
   # Open report
   xdg-open coverage.html

Coverage in CI/CD
=================

Coverage is automatically checked in CI:

.. code-block:: yaml

   # .github/workflows/coverage.yml
   - name: Check Coverage
     run: |
       gcovr -r . --txt-metric branch --fail-under-line 100 --fail-under-branch 100

Coverage History
================

.. list-table:: Coverage History
   :header-rows: 1
   :widths: 20 20 20 20 20

   * - Date
     - Version
     - C0 Coverage
     - C1 Coverage
     - Status
   * - 2026-03-12
     - 1.0.0
     - 100%
     - 100%
     - ✅ Pass
   * - 2026-03-11
     - 0.9.0
     - 98%
     - 97%
     - ⚠️ Below target
   * - 2026-03-10
     - 0.8.0
     - 95%
     - 93%
     - ⚠️ Below target

Notes
=====

* Coverage measured with gcov/gcovr
* Compiler: GCC 13.3.0
* Flags: ``--coverage -fprofile-arcs -ftest-coverage``
* Test framework: Catch2
* Mock framework: Trompeloeil
