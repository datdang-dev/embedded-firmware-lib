# 🤖 AI Agent Debug Prompt - Embedded HSM Firmware

**Purpose:** Guide AI agents to find injected bugs using AI Debug Extension  
**Branch:** ai-debug-training  
**Bugs Injected:** 6 (BUG-001, BUG-002, BUG-003, BUG-004, BUG-005, BUG-006, BUG-007, BUG-010)

---

## 📋 COPY THIS PROMPT TO AI AGENT

```markdown
# AI Debug Challenge - Embedded HSM Firmware

## 🎯 Your Mission

You are debugging an Embedded HSM Firmware project that has **6-8 injected bugs**. Your task is to find ALL bugs using the AI Debug Extension.

## 📍 Context

- **Project:** Embedded HSM Firmware (C++)
- **Branch:** ai-debug-training
- **Location:** /home/datdang/working/common_dev/embedded_hsm
- **Debug Tool:** ai-debug.sh script
- **Bugs:** 6-8 injected bugs of various types

## 🛠️ Available Tools

### Debug Commands (source ai-debug.sh first):
```bash
ai_launch ./build/ehsm_host true    # Start debug session
ai_stack                             # Get stack trace
ai_vars                              # Get all variables
ai_bp <file> <line>                  # Set breakpoint
ai_continue                          # Continue execution
ai_next                              # Step over
ai_step_in                           # Step into function
ai_eval '<expression>'               # Evaluate expression
ai_last_stop                         # Get stop info
```

### Test Commands:
```bash
cd build
./tests/test_crypto_service          # Run crypto tests
./tests/test_integration             # Run integration tests
```

## 🐛 Bug Types to Find

You are looking for these types of bugs:

1. **Buffer Overflow** - Missing size validation
2. **Wrong Function Call** - Calling wrong method
3. **Null Pointer** - Missing null checks
4. **Off-by-One** - Wrong comparison operators
5. **Uninitialized Variable** - Variables not initialized
6. **Memory Leak** - Allocated but not freed
7. **Wrong Return Value** - Hardcoded/wrong values
8. **Wrong Logic Operator** - || instead of &&

## 📊 Your Workflow

### Phase 1: Initial Analysis (5 minutes)

1. **Source the debug script:**
   ```bash
   cd /home/datdang/working/common_dev/embedded_hsm
   source ai-debug.sh
   ```

2. **Launch debug session:**
   ```bash
   ai_launch ./build/ehsm_host true
   ```

3. **Gather initial context:**
   ```bash
   ai_stack    # Where are we?
   ai_vars     # What variables exist?
   ```

4. **Run failing tests:**
   ```bash
   cd build && ./tests/test_crypto_service 2>&1 | head -40
   ```

### Phase 2: Strategic Debugging (15 minutes)

5. **Set breakpoints at suspicious code:**
   - crypto_service.cpp (encrypt/decrypt functions)
   - keystore_service.cpp (importKey, getKey)
   - session_manager.cpp (createSession, closeSession, isSessionValid)
   - hsm_api_impl.cpp (init function)

6. **Step through execution:**
   ```bash
   ai_bp crypto_service.cpp 58    # Suspected bug location
   ai_continue                     # Run to BP
   ai_step_in                      # Step into function call
   ai_eval 'algorithm_'            # Check what's called
   ```

7. **Track variable changes:**
   ```bash
   ai_vars                         # Before step
   ai_next                         # Step over
   ai_vars                         # After step - what changed?
   ```

### Phase 3: Bug Detection (15 minutes)

8. **Look for these patterns:**

   **Uninitialized Variables:**
   - Values like 32767, 0xCC, 0xCDCDCDCD
   - Random/inconsistent values
   
   **Null Pointers:**
   - Pointer values = 0x0
   - Crashes on dereference
   
   **Wrong Function Calls:**
   - Test expects A() but B() called
   - Mock expectation violations
   
   **Off-by-One Errors:**
   - Valid buffers rejected
   - Boundary conditions fail
   
   **Buffer Overflows:**
   - No size validation before memcpy
   - Can write beyond buffer bounds
   
   **Memory Leaks:**
   - new without delete
   - Error paths skip cleanup
   
   **Wrong Logic:**
   - Conditions always true/false
   - Wrong branches taken

9. **Evaluate suspicious expressions:**
   ```bash
   ai_eval 'sessionId'             # Check initialization
   ai_eval 'keyData.size()'        # Check sizes
   ai_eval 'output.size() < input.size()'  # Check conditions
   ```

### Phase 4: Report Findings (5 minutes)

10. **Document each bug found:**

```markdown
## Bug #X: <Type>

**Location:** `<file>:<line>`

**Severity:** Critical/High/Medium/Low

**Symptom:** 
<What test fails or what behavior is wrong>

**Root Cause:**
<Why this bug causes the failure>

**Evidence:**
```bash
# Debug output showing the bug
ai_eval '<variable>'
ai_stack
```

**Fix:**
```cpp
// Current (buggy) code
<current code>

// Fixed code
<fixed code>
```

**Confidence:** High/Medium/Low
```

## 🎯 Expected Output

Please provide:

1. **List of ALL bugs found** (should be 6-8)
2. **Exact location** of each bug (file:line)
3. **Root cause analysis** for each
4. **Specific fix** for each
5. **Confidence level** (High/Medium/Low)
6. **Evidence** from debug session

## 📝 Report Template

```markdown
# AI Debug Report - Embedded HSM

## Summary
- **Total Bugs Found:** X/8
- **Critical:** X
- **High:** X
- **Medium:** X
- **Low:** X

## Bugs Found

### Bug #1: <Type>
- **Location:** file.cpp:line
- **Severity:** Critical
- **Symptom:** Test TC_CRYPTO_001 fails
- **Root Cause:** encrypt() calls decrypt() instead
- **Evidence:** 
  ```
  ai_stack → crypto_service.cpp:58
  ai_eval 'algorithm_->decrypt' → called instead of encrypt
  ```
- **Fix:** Change line 58 from decrypt() to encrypt()
- **Confidence:** High

### Bug #2: ...

## Recommendations
1. Fix Critical bugs first
2. Add unit tests for edge cases
3. Enable compiler warnings
4. Use static analysis tools

## Debug Session Info
- **Duration:** X minutes
- **Commands Used:** ai_launch, ai_stack, ai_vars, ai_bp, ai_continue, ai_next, ai_eval
- **Breakpoints Set:** X
- **Tests Run:** Y
```

## 💡 Tips for Success

1. **Start with test failures** - They point to bug locations
2. **Set BPs strategically** - At function entries and suspected bugs
3. **Step through slowly** - Watch each line execution
4. **Check variables frequently** - Before and after each step
5. **Compare expected vs actual** - What should happen vs what does
6. **Use mock expectations** - They show what was expected
7. **Trace call chains** - Follow the execution flow
8. **Look for patterns** - Common bug types repeat

## 🏆 Success Criteria

You succeed when you:

✅ Find ALL 6-8 injected bugs  
✅ Identify exact locations (file:line)  
✅ Explain root causes clearly  
✅ Suggest specific fixes  
✅ Provide evidence from debug session  
✅ Assign accurate confidence levels  

## 🚀 Start Now!

```bash
cd /home/datdang/working/common_dev/embedded_hsm
source ai-debug.sh
ai_launch ./build/ehsm_host true
# Begin debugging!
```

Good luck, AI debugger! 🐛🔍
```

---

## 📊 EXPECTED RESULTS

### AI Should Find:

| Bug ID | Type | Location | Difficulty |
|--------|------|----------|------------|
| BUG-001 | Buffer Overflow | keystore_service.cpp:56 | Medium |
| BUG-002 | Wrong Function | crypto_service.cpp:58 | Easy |
| BUG-003 | Null Pointer | session_manager.cpp:45 | Medium |
| BUG-004 | Off-by-One | crypto_service.cpp:41,75 | Easy |
| BUG-005 | Uninitialized | session_manager.cpp:96 | Medium |
| BUG-006 | Memory Leak | hsm_api_impl.cpp:28-60 | Hard |
| BUG-007 | Wrong Enum | crypto_service.cpp:117 | Easy |
| BUG-010 | Wrong Logic | session_manager.cpp:81 | Medium |

### Expected Performance:

- **Excellent AI:** Finds 7-8/8 bugs in 30 minutes
- **Good AI:** Finds 5-6/8 bugs in 45 minutes
- **Average AI:** Finds 3-4/8 bugs in 60 minutes

---

## 🎯 USAGE INSTRUCTIONS

1. **Copy the prompt** above (from "📋 COPY THIS PROMPT" section)
2. **Paste to AI agent** (Claude, GPT, Cursor, etc.)
3. **Let AI debug** using ai-debug.sh commands
4. **Review AI's report** for accuracy
5. **Compare with expected bugs** list

---

**Created:** March 13, 2026  
**For:** AI Debug Extension Testing  
**Branch:** ai-debug-training
