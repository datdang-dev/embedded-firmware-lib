# 🤖 AI Debug Skills - Embedded HSM Firmware

**Version:** 1.0  
**Purpose:** Guide AI agents to effectively debug C++ embedded firmware  
**Extension:** AI VSCode Debug Proxy v0.1.2-beta  
**Project:** Embedded HSM Firmware

---

## 🎯 OVERVIEW

This skill teaches AI agents how to:
1. Use the `ai-debug.sh` script effectively
2. Gather complete debugging context
3. Detect common C++ bugs
4. Trace root causes
5. Suggest accurate fixes

---

## 📋 PREREQUISITES

### Environment Setup
```bash
# Ensure debug proxy is running
curl http://localhost:9999/api/ping

# Expected response:
# {"success":true,"data":{"message":"pong",...}}
```

### Source Debug Script
```bash
cd /home/datdang/working/common_dev/embedded_hsm
source ai-debug.sh
```

---

## 🔍 AI DEBUG WORKFLOW

### Phase 1: Gather Context

**Step 1: Launch Debug Session**
```bash
ai_launch ./build/ehsm_host true
```

**What AI Should Check:**
- ✅ Session ID received
- ✅ Stop reason (entry/breakpoint/step)
- ✅ No errors

**Step 2: Get Stack Trace**
```bash
ai_stack
```

**What AI Should Extract:**
- Current function name
- Source file path
- Line number
- Column

**Step 3: Get All Variables**
```bash
ai_vars
```

**What AI Should Analyze:**
- Variable names and types
- Current values
- Null pointers
- Uninitialized values (garbage like 32767, 0xCC, etc.)
- Smart pointer states

---

### Phase 2: Strategic Debugging

**Step 4: Set Breakpoints at Suspicious Code**
```bash
# Set BP at function entry
ai_bp function_name.cpp 42

# Set BP before suspected bug
ai_bp suspected_file.cpp 58
```

**AI Should Set BPs At:**
- Function entries
- Before/after suspicious operations
- Error handling code
- Memory operations

**Step 5: Continue to Breakpoint**
```bash
ai_continue
```

**AI Should Check:**
- Which BP hit?
- Why stopped?
- What changed?

**Step 6: Step-Through Execution**
```bash
ai_next    # Step over
ai_step_in # Step into
ai_step_out # Step out
```

**AI Should Watch:**
- Variable changes after each step
- Control flow (if/else/loops)
- Function calls and returns

---

### Phase 3: Bug Detection

**Step 7: Evaluate Expressions**
```bash
ai_eval 'variable_name'
ai_eval '*pointer'
ai_eval 'array[0]'
```

**AI Should Look For:**
- Null pointers (`0x0`)
- Uninitialized values (`32767`, `0xCCCCCCCC`)
- Out-of-bounds indices
- Invalid states

**Step 8: Get Stop Info**
```bash
ai_last_stop
```

**AI Should Analyze:**
- Stop reason
- Thread state
- Source location

---

## 🐛 COMMON BUG PATTERNS

### Pattern 1: Uninitialized Variable

**Symptoms:**
```
Variable value: 32767, 0xCC, 0xCDCDCDCD
Random/inconsistent values
```

**AI Detection:**
```bash
ai_vars | jq '.[] | select(.value == "32767")'
```

**AI Report:**
```
🐛 BUG DETECTED: Uninitialized Variable

Location: <file>:<line>
Variable: <name>
Value: 32767 (garbage)
Expected: Initialized value

Fix: Initialize variable before use
```

---

### Pattern 2: Null Pointer Dereference

**Symptoms:**
```
Pointer value: 0x0
Crash on dereference
```

**AI Detection:**
```bash
ai_vars | jq '.[] | select(.type | contains("ptr"))'
ai_eval 'pointer_variable'
```

**AI Report:**
```
🐛 BUG DETECTED: Null Pointer

Location: <file>:<line>
Pointer: <name>
Value: 0x0 (null)

Fix: Check for null before dereference
```

---

### Pattern 3: Wrong Function Call

**Symptoms:**
```
Test expects function A() to be called
Mock shows function B() was called
```

**AI Detection:**
```bash
# Set BP at suspected line
ai_bp crypto_service.cpp 58

# Continue and check
ai_continue
ai_eval 'algorithm_'
```

**AI Report:**
```
🐛 BUG DETECTED: Wrong Function Call

Location: crypto_service.cpp:58
Expected: algorithm_->encrypt()
Actual: algorithm_->decrypt()

Fix: Change decrypt() to encrypt()
```

---

### Pattern 4: Off-by-One Error

**Symptoms:**
```
Valid buffers rejected
Boundary conditions fail
```

**AI Detection:**
```bash
ai_bp crypto_service.cpp 41
ai_continue
ai_eval 'output.size()'
ai_eval 'input.size()'
```

**AI Report:**
```
🐛 BUG DETECTED: Off-by-One Error

Location: crypto_service.cpp:41
Condition: output.size() <= input.size()
Issue: Rejects exact-size buffers

Fix: Change <= to <
```

---

### Pattern 5: Buffer Overflow

**Symptoms:**
```
No size validation
memcpy with unchecked size
Stack corruption
```

**AI Detection:**
```bash
ai_bp keystore_service.cpp 56
ai_continue
ai_eval 'keyData.size()'
ai_eval 'types::kMaxKeySize'
```

**AI Report:**
```
🐛 BUG DETECTED: Buffer Overflow Risk

Location: keystore_service.cpp:56
Issue: No size validation before memcpy
Risk: Can overflow 64-byte buffer

Fix: Add size validation
```

---

### Pattern 6: Memory Leak

**Symptoms:**
```
new without delete
Error paths skip cleanup
```

**AI Detection:**
```bash
ai_bp hsm_api_impl.cpp 28
ai_continue
ai_eval 'tempBuffer'
# Trigger error path
ai_continue
# Check if deleted
```

**AI Report:**
```
🐛 BUG DETECTED: Memory Leak

Location: hsm_api_impl.cpp:28
Allocated: new uint8_t[1024]
Freed: Only on success path
Leak: 1024 bytes on error

Fix: Delete on all error paths
```

---

### Pattern 7: Wrong Logic Operator

**Symptoms:**
```
Condition always true/false
Wrong branches taken
```

**AI Detection:**
```bash
ai_bp session_manager.cpp 81
ai_continue
ai_eval 'session.id'
ai_eval 'sessionId'
ai_eval 'session.isActive'
```

**AI Report:**
```
🐛 BUG DETECTED: Wrong Logic Operator

Location: session_manager.cpp:81
Condition: (id == sessionId || isActive)
Issue: || should be &&

Fix: Change || to &&
```

---

### Pattern 8: Wrong Enum/Return Value

**Symptoms:**
```
Function always returns same value
Ignores actual state
```

**AI Detection:**
```bash
ai_bp crypto_service.cpp 117
ai_continue
ai_eval 'algorithm_'
ai_eval 'algorithm_->getAlgorithmType()'
```

**AI Report:**
```
🐛 BUG DETECTED: Wrong Return Value

Location: crypto_service.cpp:117
Returns: AES_128 (hardcoded)
Should: algorithm_->getAlgorithmType()

Fix: Return actual algorithm type
```

---

## 🎯 AI DEBUG PROMPT TEMPLATE

Use this prompt to guide AI agents:

```markdown
# AI Debug Task

## Context
You are debugging Embedded HSM Firmware with injected bugs.

## Available Tools
- ai-debug.sh script for debug control
- Full GDB capabilities via proxy
- C++ type system support

## Your Task

### 1. Gather Initial Context
```bash
source ai-debug.sh
ai_launch ./build/ehsm_host true
ai_stack
ai_vars
```

### 2. Analyze State
- What function are we in?
- What are the variable values?
- Any suspicious values (null, uninitialized, etc.)?

### 3. Set Strategic Breakpoints
Based on test failures, set BPs at:
- Suspected bug locations
- Function entries
- Before/after suspicious operations

### 4. Step Through
```bash
ai_continue  # Run to BP
ai_next      # Step over
ai_step_in   # Step into
ai_eval 'variable'  # Check values
```

### 5. Detect Bugs
Look for:
- [ ] Uninitialized variables
- [ ] Null pointers
- [ ] Wrong function calls
- [ ] Off-by-one errors
- [ ] Buffer overflows
- [ ] Memory leaks
- [ ] Wrong logic operators
- [ ] Wrong return values

### 6. Report Findings
For each bug found:
```
🐛 BUG: <Type>

Location: <file>:<line>
Symptom: <what you observed>
Root Cause: <why it happens>
Fix: <specific code change>
```

## Expected Output
- List of all bugs found
- Location of each bug
- Suggested fix for each
- Confidence level (High/Medium/Low)

## Tips
- Use ai_vars frequently to track changes
- Set BPs before suspected bugs
- Step through line by line
- Compare expected vs actual behavior
- Check mock expectations vs actual calls
```

---

## 📊 DEBUG CHECKLIST

### Before Starting
- [ ] Debug proxy running (`curl localhost:9999/api/ping`)
- [ ] Script sourced (`source ai-debug.sh`)
- [ ] Binary built (`cmake --build build`)
- [ ] Test failures documented

### During Debug
- [ ] Stack trace captured
- [ ] All variables inspected
- [ ] Suspicious values noted
- [ ] Breakpoints set strategically
- [ ] Step-through executed
- [ ] Expressions evaluated
- [ ] Stop info analyzed

### After Debug
- [ ] All bugs documented
- [ ] Root causes identified
- [ ] Fixes suggested
- [ ] Confidence levels assigned
- [ ] Evidence collected

---

## 🎓 EXAMPLE AI DEBUG SESSION

### Session: Finding BUG-002 (Wrong Function Call)

**AI Agent Workflow:**

```bash
# 1. Launch
$ ai_launch ./build/ehsm_host true
✓ Debug session launched

# 2. Get context
$ ai_stack
main() at main.cpp:47

$ ai_vars
[7 variables with types]

# 3. Test fails - check crypto service
$ ai_bp crypto_service.cpp 58
✓ Breakpoint set

$ ai_continue
✓ Continued (hit BP at line 58)

# 4. Inspect
$ ai_eval 'algorithm_'
0x55555576e2a0

$ ai_step_in
✓ Stepped into algorithm_->decrypt()

# 5. Detect bug
AI Analysis:
"encrypt() test called decrypt() - WRONG FUNCTION!"

# 6. Report
🐛 BUG DETECTED: Wrong Function Call
Location: crypto_service.cpp:58
Expected: algorithm_->encrypt()
Actual: algorithm_->decrypt()
Fix: Change decrypt() to encrypt()
Confidence: HIGH
```

---

## 🔧 ADVANCED TECHNIQUES

### Watch Multiple Variables
```bash
# Create watch loop
while true; do
  ai_vars | jq '.[] | {name, value}'
  ai_next
  sleep 0.5
done
```

### Compare Before/After
```bash
# Before
ai_vars > before.txt
ai_next
# After
ai_vars > after.txt
# Diff
diff before.txt after.txt
```

### Trace Execution Path
```bash
# Log each step
echo "Execution Trace:" > trace.log
for i in {1..10}; do
  ai_stack | jq '.[0] | {file: .sourcePath, line: .line}' >> trace.log
  ai_next
done
cat trace.log
```

---

## 📝 AI REPORT TEMPLATE

```markdown
# AI Debug Report

## Session Info
- **Date:** <date>
- **Binary:** <path>
- **Test:** <test name>
- **Duration:** <time>

## Bugs Found

### Bug #1: <Type>
- **Location:** <file>:<line>
- **Severity:** Critical/High/Medium/Low
- **Symptom:** <what failed>
- **Root Cause:** <why it failed>
- **Evidence:** <debug output>
- **Fix:** <specific change>
- **Confidence:** High/Medium/Low

### Bug #2: ...

## Summary
- **Total Bugs:** X
- **Critical:** X
- **High:** X
- **Medium:** X
- **Low:** X

## Recommendations
1. <priority fix>
2. <priority fix>
3. <priority fix>
```

---

## ✅ SUCCESS CRITERIA

AI Debug session is successful when:

1. ✅ All injected bugs found
2. ✅ Accurate locations identified
3. ✅ Root causes explained
4. ✅ Specific fixes suggested
5. ✅ Evidence provided
6. ✅ Confidence levels assigned

---

## 🚀 QUICK START

```bash
# 1. Setup
cd /home/datdang/working/common_dev/embedded_hsm
source ai-debug.sh

# 2. Launch
ai_launch ./build/ehsm_host true

# 3. Debug
ai_stack
ai_vars
ai_bp <file> <line>
ai_continue
ai_next
ai_eval '<variable>'

# 4. Report bugs found
```

---

**Skill Version:** 1.0  
**Last Updated:** March 13, 2026  
**Maintained By:** AI Debug Team
