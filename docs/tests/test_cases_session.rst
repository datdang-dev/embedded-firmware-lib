.. _test_cases_session:

================================
Session Manager Test Cases
================================

This document contains all test cases for the Session Manager module.

Module Overview
===============

**Module:** SessionManager  
**Interface:** ISessionManager  
**Implementation:** ehsm::mw::SessionManager  
**Location:** include/session_manager.hpp, src/middleware/session_manager.cpp  
**Public Functions:**
- ``init()``
- ``deinit()``
- ``createSession(sessionId)``
- ``closeSession(sessionId)``
- ``isSessionValid(sessionId)``

**Constants:**
- kMaxSessions = 4
- kInvalidSessionId = 0xFFFFFFFF

Test Environment
================

**Framework:** Catch2  
**Dependencies:** None (self-contained)

Test Cases
==========

TC_SESS_001 - Create Session Successfully
------------------------------------------

**Interface:** ISessionManager::createSession()

**Purpose:** Verify session creation works

**Preconditions:**
- SessionManager initialized
- Session slots available

**Test Steps:**
1. Create SessionManager
2. Call init()
3. Call createSession(sessionId)
4. Verify return status
5. Verify sessionId is valid (not 0, not 0xFFFFFFFF)
6. Call isSessionValid(sessionId)
7. Verify session is valid

**Expected Result:**
- Status::OK returned
- sessionId assigned (1, 2, 3, or 4)
- isSessionValid() returns true

**Coverage:**
- **Lines:** session_manager.cpp:44-66
- **Branches:** Success path, first available slot
- **C0:** 100%
- **C1:** 100%

**Test Code:**
.. code-block:: cpp

   TEST_CASE("TC_SESS_001 - Create session successfully", "[session][create]") {
       SessionManager sessionMgr;
       sessionMgr.init();
       
       SessionId sessionId;
       auto status = sessionMgr.createSession(sessionId);
       
       REQUIRE(status.isOk());
       REQUIRE(sessionId != kInvalidSessionId);
       REQUIRE(sessionMgr.isSessionValid(sessionId));
   }

---

TC_SESS_002 - Create Multiple Sessions
---------------------------------------

**Interface:** ISessionManager::createSession()

**Purpose:** Verify multiple sessions can be created

**Preconditions:**
- SessionManager initialized
- All 4 session slots available

**Test Steps:**
1. Create SessionManager and initialize
2. Create 4 sessions in a loop
3. Verify each returns OK
4. Verify each has unique sessionId
5. Verify all are valid

**Expected Result:**
- All 4 creations return Status::OK
- Session IDs are unique (1, 2, 3, 4)
- All sessions are valid

**Coverage:**
- **Lines:** session_manager.cpp:44-66
- **Branches:** Multiple iterations, slot allocation
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_003 - Create Session When All Slots Full
-------------------------------------------------

**Interface:** ISessionManager::createSession()

**Purpose:** Verify creation fails when no slots available

**Preconditions:**
- SessionManager initialized
- All 4 session slots occupied

**Test Steps:**
1. Create 4 sessions
2. Attempt to create 5th session
3. Verify return status

**Expected Result:**
- 5th creation returns Status::ERR_SESSION_INVALID
- No session ID assigned

**Coverage:**
- **Lines:** session_manager.cpp:60-64
- **Branches:** No available slots path
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_004 - Close Valid Session
----------------------------------

**Interface:** ISessionManager::closeSession()

**Purpose:** Verify session close works

**Preconditions:**
- SessionManager initialized
- Session exists and is active

**Test Steps:**
1. Create session
2. Call closeSession(sessionId)
3. Verify return status
4. Call isSessionValid(sessionId)
5. Verify session is invalid

**Expected Result:**
- Status::OK returned
- isSessionValid() returns false
- Session slot freed

**Coverage:**
- **Lines:** session_manager.cpp:69-83
- **Branches:** Success path
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_005 - Close Invalid Session
------------------------------------

**Interface:** ISessionManager::closeSession()

**Purpose:** Verify close fails for invalid session

**Preconditions:**
- SessionManager initialized

**Test Steps:**
1. Call closeSession(kInvalidSessionId)
2. Verify return status

**Expected Result:**
- Status::ERR_SESSION_INVALID returned

**Coverage:**
- **Lines:** session_manager.cpp:73-77
- **Branches:** Invalid session check
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_006 - Close Already Closed Session
-------------------------------------------

**Interface:** ISessionManager::closeSession()

**Purpose:** Verify close fails for already closed session

**Preconditions:**
- SessionManager initialized
- Session created and closed

**Test Steps:**
1. Create session
2. Close session
3. Attempt to close again
4. Verify return status

**Expected Result:**
- Second close returns Status::ERR_SESSION_INVALID

**Coverage:**
- **Lines:** session_manager.cpp:73-77
- **Branches:** Session not active check
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_007 - Validate Session
-------------------------------

**Interface:** ISessionManager::isSessionValid()

**Purpose:** Verify session validation works

**Preconditions:**
- SessionManager initialized
- Mix of valid and invalid sessions

**Test Steps:**
1. Create 2 sessions (IDs: 1, 2)
2. Close session 1
3. Call isSessionValid(1)
4. Call isSessionValid(2)
5. Call isSessionValid(invalid_id)

**Expected Result:**
- isSessionValid(1) returns false (closed)
- isSessionValid(2) returns true (active)
- isSessionValid(invalid) returns false

**Coverage:**
- **Lines:** session_manager.cpp:88-97
- **Branches:** Both true and false paths
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_008 - Session ID Wraparound
------------------------------------

**Interface:** ISessionManager::createSession()

**Purpose:** Verify session ID wraparound is handled

**Preconditions:**
- SessionManager initialized
- Session ID counter near kInvalidSessionId

**Test Steps:**
1. Create and close many sessions to increment counter
2. Verify wraparound to 1 when counter reaches kInvalidSessionId

**Expected Result:**
- Session ID wraps to 1 after reaching 0xFFFFFFFF
- No duplicate IDs

**Coverage:**
- **Lines:** session_manager.cpp:60-62
- **Branches:** Wraparound check
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_009 - Initialize Twice
-------------------------------

**Interface:** ISessionManager::init()

**Purpose:** Verify double initialization is safe

**Preconditions:**
- SessionManager created

**Test Steps:**
1. Call init()
2. Call init() again
3. Verify both return OK

**Expected Result:**
- Both calls return Status::OK
- Idempotent operation

**Coverage:**
- **Lines:** session_manager.cpp:13-16
- **Branches:** Already initialized check
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_010 - Deinitialize Without Init
----------------------------------------

**Interface:** ISessionManager::deinit()

**Purpose:** Verify deinit fails without init

**Preconditions:**
- SessionManager created (not initialized)

**Test Steps:**
1. Call deinit() without init()
2. Verify return status

**Expected Result:**
- Status::ERR_NOT_INITIALIZED returned

**Coverage:**
- **Lines:** session_manager.cpp:29-33
- **Branches:** Not initialized check
- **C0:** 100%
- **C1:** 100%

---

TC_SESS_011 - Deinitialize With Active Sessions
------------------------------------------------

**Interface:** ISessionManager::deinit()

**Purpose:** Verify deinit clears active sessions

**Preconditions:**
- SessionManager initialized
- 2 active sessions exist

**Test Steps:**
1. Create 2 sessions
2. Call deinit()
3. Verify return status
4. Call init() again
5. Verify old session IDs are invalid

**Expected Result:**
- Status::OK returned
- All sessions cleared
- Old session IDs invalid after reinit

**Coverage:**
- **Lines:** session_manager.cpp:35-42
- **Branches:** Session cleanup loop
- **C0:** 100%
- **C1:** 100%

---

Test Summary
============

.. list-table:: Session Manager Test Coverage
   :header-rows: 1
   :widths: 20 35 15 15 15

   * - Test Case
     - Function Tested
     - C0 Coverage
     - C1 Coverage
     - Status
   * - TC_SESS_001
     - createSession() - success
     - 100%
     - 100%
     - Pass
   * - TC_SESS_002
     - createSession() - multiple
     - 100%
     - 100%
     - Pass
   * - TC_SESS_003
     - createSession() - slots full
     - 100%
     - 100%
     - Pass
   * - TC_SESS_004
     - closeSession() - success
     - 100%
     - 100%
     - Pass
   * - TC_SESS_005
     - closeSession() - invalid
     - 100%
     - 100%
     - Pass
   * - TC_SESS_006
     - closeSession() - already closed
     - 100%
     - 100%
     - Pass
   * - TC_SESS_007
     - isSessionValid()
     - 100%
     - 100%
     - Pass
   * - TC_SESS_008
     - createSession() - wraparound
     - 100%
     - 100%
     - Pass
   * - TC_SESS_009
     - init() - idempotent
     - 100%
     - 100%
     - Pass
   * - TC_SESS_010
     - deinit() - not initialized
     - 100%
     - 100%
     - Pass
   * - TC_SESS_011
     - deinit() - with sessions
     - 100%
     - 100%
     - Pass

**Total Coverage:** 100% C0, 100% C1 for all public functions
