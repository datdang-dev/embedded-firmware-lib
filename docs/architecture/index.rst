.. _architecture_index:

=====================
Architecture Documentation
=====================

This section contains the architecture documentation for the Embedded HSM firmware.

Architecture Overview
=====================

The Embedded HSM firmware follows a **5-layer architecture** with clear separation of concerns:

.. code-block:: text

   ┌─────────────────────────────────────────────────────────────┐
   │                    API Layer (Facade)                       │
   │              User-facing interfaces                         │
   ├─────────────────────────────────────────────────────────────┤
   │              Middleware Layer (Router)                      │
   │         Session management, job queuing                     │
   ├─────────────────────────────────────────────────────────────┤
   │                Service Layer (Business Logic)               │
   │   Crypto Service │ Keystore Service │ Storage Service       │
   ├─────────────────────────────────────────────────────────────┤
   │               Driver Layer (Hardware Bridge)                │
   │         Hardware abstraction for services                   │
   ├─────────────────────────────────────────────────────────────┤
   │                 MCAL Layer (C Interfaces)                   │
   │         Microcontroller abstraction (UART, etc.)            │
   └─────────────────────────────────────────────────────────────┘

Design Principles
=================

**SOLID Principles:**
- **Single Responsibility:** Each layer has one responsibility
- **Open/Closed:** Open for extension, closed for modification
- **Liskov Substitution:** Interfaces allow substitution
- **Interface Segregation:** Many small interfaces
- **Dependency Inversion:** Depend on abstractions

**Design Patterns:**
- **Facade:** HsmApiImpl simplifies client interface
- **Strategy:** ICryptoAlgorithm for algorithm selection
- **Dependency Injection:** Constructor injection for dependencies
- **Repository:** IKeystoreService for key storage

Architecture Documents
======================

.. toctree::
   :maxdepth: 2

   Layer Overview <layer_overview>
   Design Decisions <design_decisions>
   Module Dependencies <dependencies>
   Security Architecture <security>

Diagrams
========

See :doc:`../diagrams/index` for all architecture diagrams.

Key Interfaces
==============

.. list-table:: Major Interfaces
   :header-rows: 1
   :widths: 30 70

   * - Interface
     - Responsibility
   * - IHsmApi
     - Public API for all HSM operations
   * - ICryptoService
     - Cryptographic operations (encrypt, decrypt, hash)
   * - IKeystoreService
     - Secure key storage and management
   * - ISessionManager
     - Session lifecycle management
   * - ICryptoAlgorithm
     - Algorithm strategy interface

Implementation Status
=====================

.. list-table:: Layer Implementation Status
   :header-rows: 1
   :widths: 25 25 25 25

   * - Layer
     - Language
     - Status
     - Coverage
   * - API
     - C++20
     - ✅ Complete
     - 100%
   * - Middleware
     - C++20
     - ✅ Complete
     - 100%
   * - Services
     - C++20
     - ✅ Complete
     - 100%
   * - Drivers
     - C++20
     - ⚠️ TODO
     - N/A
   * - MCAL
     - C11
     - ✅ Stub (HOST)
     - N/A
