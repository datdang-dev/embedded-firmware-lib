.. _diagrams_index:

=================
Diagrams Index
=================

This page contains all PlantUML diagrams for the Embedded HSM project.

Class Diagrams
==============

.. list-table:: Class Diagrams
   :header-rows: 1
   :widths: 30 70

   * - Diagram
     - Description
   * - :download:`IHsmApi <diagrams/classes/ihsm_api.puml>`
     - Public API interface with all methods
   * - :download:`ICryptoService <diagrams/classes/icrypto_service.puml>`
     - Crypto service interface
   * - :download:`IKeystoreService <diagrams/classes/ikeystore_service.puml>`
     - Keystore service interface
   * - :download:`ICryptoAlgorithm <diagrams/classes/icrypto_algorithm.puml>`
     - Strategy pattern interface for algorithms
   * - :download:`ISessionManager <diagrams/classes/isession_manager.puml>`
     - Session manager interface

Component Diagrams
==================

.. list-table:: Component Diagrams
   :header-rows: 1
   :widths: 30 70

   * - Diagram
     - Description
   * - :download:`Component Architecture <diagrams/components/component_architecture.puml>`
     - Full system component architecture with dependencies

Sequence Diagrams
=================

.. list-table:: Sequence Diagrams
   :header-rows: 1
   :widths: 30 70

   * - Diagram
     - Description
   * - :download:`Encryption Flow <diagrams/sequences/encrypt_flow.puml>`
     - Complete encryption flow through all layers
   * - :download:`Key Import Flow <diagrams/sequences/key_import_flow.puml>`
     - Key import process with validation

Viewing Diagrams
================

To view these diagrams, you can:

1. Use the PlantUML VS Code extension
2. Use the online PlantUML server at https://www.plantuml.com/plantuml/
3. Generate PNG/SVG using the PlantUML JAR:

.. code-block:: bash

   java -jar plantuml.jar docs/diagrams/**/*.puml
