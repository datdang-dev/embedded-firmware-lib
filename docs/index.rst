.. Embedded HSM Firmware Documentation master file

========================================
Embedded HSM Firmware Documentation
========================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   api/index
   architecture/index
   tests/index
   diagrams/index

Overview
========

The Embedded HSM Firmware is a C++ implementation of a Hardware Security Module with a 5-layer architecture following enterprise design patterns.

Key Features
------------

* **5-Layer Architecture:** API → Middleware → Service → Driver → MCAL
* **Design Patterns:** Facade, Strategy, Dependency Injection, Interface Segregation
* **Language:** C++20 (services), C11 (MCAL)
* **Security:** Key protection, secure erasure, permission-based access
* **Portability:** Hardware abstraction via MCAL interfaces
* **Testability:** Comprehensive unit tests with mocks

Quick Start
-----------

.. code-block:: cpp

   #include "ihsm_api.hpp"
   #include "crypto_service.hpp"
   #include "keystore_service.hpp"
   
   int main() {
       // Create services
       auto keystore = std::make_shared<KeystoreService>();
       auto crypto = std::make_unique<CryptoService>(
           keystore, 
           std::make_unique<Aes256Algorithm>()
       );
       
       // Create API
       auto hsm = std::make_unique<HsmApiImpl>(
           std::make_unique<SessionManager>(),
           std::move(crypto),
           keystore
       );
       
       // Initialize
       hsm->init();
       
       // Use HSM
       hsm->encrypt(...);
       
       return 0;
   }

Build Instructions
------------------

.. code-block:: bash

   # Configure for HOST platform
   cmake -DTARGET_PLATFORM=HOST -B build
   
   # Build
   cmake --build build
   
   # Run demo
   ./build/ehsm_host
   
   # Run tests
   cd build && ctest

Documentation Structure
-----------------------

* :doc:`api/index` - API reference documentation for all public interfaces
* :doc:`architecture/index` - Architecture diagrams and design decisions
* :doc:`tests/index` - Test documentation and coverage reports
* :doc:`diagrams/index` - PlantUML diagrams index

Indices and Tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
