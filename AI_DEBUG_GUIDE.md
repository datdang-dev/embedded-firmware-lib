# 🤖 AI Debug Agent - Complete Guide

**Purpose:** Complete guide for using AI Debug Extension with Embedded HSM Firmware  
**Created:** March 13, 2026  
**Branch:** ai-debug-training

---

## 📚 DOCUMENTATION INDEX

### Core Documents

| Document | Purpose | Location |
|----------|---------|----------|
| **SKILLS.md** | Complete debugging skill guide | `/embedded_hsm/SKILLS.md` |
| **AI_DEBUG_PROMPT.md** | Ready-to-use AI prompt | `/embedded_hsm/AI_DEBUG_PROMPT.md` |
| **BUG_INJECTION_COMPLETE.md** | List of all injected bugs | `/embedded_hsm/BUG_INJECTION_COMPLETE.md` |
| **AI_DEBUG_EFFECTIVENESS_REPORT.md** | Extension test results | `/embedded_hsm/AI_DEBUG_EFFECTIVENESS_REPORT.md` |

---

## 🚀 QUICK START

### For AI Agents

1. **Copy this prompt:** `AI_DEBUG_PROMPT.md`
2. **Paste to AI agent** (Claude, GPT, Cursor, etc.)
3. **AI follows workflow** in SKILLS.md
4. **AI reports bugs found**

### For Humans

1. **Read effectiveness report:** `AI_DEBUG_EFFECTIVENESS_REPORT.md`
2. **Review bug list:** `BUG_INJECTION_COMPLETE.md`
3. **Test extension:** Follow SKILLS.md workflow

---

## 📋 FILES CREATED

### Documentation (6 files):

```
embedded_hsm/
├── SKILLS.md                           # AI debugging skill guide
├── AI_DEBUG_PROMPT.md                  # Ready-to-use AI prompt
├── AI_DEBUG_EFFECTIVENESS_REPORT.md   # Extension test results
├── BUG_INJECTION_COMPLETE.md           # Bug injection summary
├── BUG_INJECTION_REPORT.md             # Detailed bug specs
└── AI_DEBUG_EXTENSION_TEST_REPORT.md  # Test session report
```

### Scripts (1 file):

```
embedded_hsm/
└── ai-debug.sh                         # Debug CLI tool
```

---

## 🎯 HOW TO USE

### Scenario 1: Test AI Agent Debugging

```bash
# 1. Give AI agent the prompt
cat AI_DEBUG_PROMPT.md | copy

# 2. Paste to AI agent
# (Claude, GPT-4, Cursor, etc.)

# 3. AI follows workflow
# AI sources ai-debug.sh
# AI launches debug session
# AI finds bugs

# 4. Review AI's report
# Compare with BUG_INJECTION_COMPLETE.md
```

### Scenario 2: Manual Debugging

```bash
# 1. Source debug script
cd /home/datdang/working/common_dev/embedded_hsm
source ai-debug.sh

# 2. Follow SKILLS.md workflow
ai_launch ./build/ehsm_host true
ai_stack
ai_vars
# ... etc
```

### Scenario 3: Extension Development

```bash
# 1. Read effectiveness report
cat AI_DEBUG_EFFECTIVENESS_REPORT.md

# 2. Review test results
cat AI_DEBUG_EXTENSION_TEST_REPORT.md

# 3. Identify improvements
# See "Areas for Improvement" sections
```

---

## 🐛 INJECTED BUGS SUMMARY

### Quick Reference

| Bug ID | Type | Location | Severity |
|--------|------|----------|----------|
| BUG-001 | Buffer Overflow | keystore_service.cpp:56 | 🔴 Critical |
| BUG-002 | Wrong Function | crypto_service.cpp:58 | 🔴 Critical |
| BUG-003 | Null Pointer | session_manager.cpp:45 | 🟡 High |
| BUG-004 | Off-by-One | crypto_service.cpp:41,75 | 🟠 Medium |
| BUG-005 | Uninitialized | session_manager.cpp:96 | 🟠 Medium |
| BUG-006 | Memory Leak | hsm_api_impl.cpp:28-60 | 🟠 Medium |
| BUG-007 | Wrong Enum | crypto_service.cpp:117 | 🟢 Low |
| BUG-010 | Wrong Logic | session_manager.cpp:81 | 🟢 Low |

### Detailed Specs

See: `BUG_INJECTION_REPORT.md` for full specifications

---

## 🧪 AI DEBUGGING WORKFLOW

### Phase 1: Context Gathering
```bash
source ai-debug.sh
ai_launch ./build/ehsm_host true
ai_stack
ai_vars
```

### Phase 2: Strategic Debugging
```bash
ai_bp <file> <line>
ai_continue
ai_next
ai_eval '<variable>'
```

### Phase 3: Bug Detection
- Check for uninitialized variables
- Look for null pointers
- Trace function calls
- Verify buffer sizes
- Check memory management

### Phase 4: Reporting
```markdown
## Bug #X: <Type>
- **Location:** file.cpp:line
- **Severity:** Critical/High/Medium/Low
- **Symptom:** <what fails>
- **Root Cause:** <why it fails>
- **Fix:** <specific change>
- **Confidence:** High/Medium/Low
```

---

## 📊 EXPECTED AI PERFORMANCE

### Excellent AI (90%+ accuracy)
- ✅ Finds 7-8/8 bugs
- ✅ Accurate locations
- ✅ Clear root causes
- ✅ Specific fixes
- ✅ < 30 minutes

### Good AI (70-89% accuracy)
- ✅ Finds 5-6/8 bugs
- ✅ Mostly accurate
- ✅ Good analysis
- ✅ < 45 minutes

### Average AI (50-69% accuracy)
- ✅ Finds 3-4/8 bugs
- ⚠️ Some misses
- ⚠️ Vague fixes
- ✅ < 60 minutes

---

## 🎯 SUCCESS METRICS

### For AI Agents

| Metric | Target | Excellent | Good | Average |
|--------|--------|-----------|------|---------|
| Bugs Found | 8 | 7-8 | 5-6 | 3-4 |
| Accuracy | 100% | 90%+ | 70-89% | 50-69% |
| Time | <60min | <30min | <45min | <60min |
| Fix Quality | Specific | Excellent | Good | Fair |

### For Extension

| Metric | Target | Result |
|--------|--------|--------|
| Launch Success | 100% | ✅ 100% |
| Variable Access | 100% | ✅ 100% |
| Stack Accuracy | 100% | ✅ 100% |
| Step Execution | 100% | ✅ 100% |
| Response Time | <500ms | ✅ <200ms |

---

## 🔧 TROUBLESHOOTING

### Issue: Debug Proxy Not Running

```bash
# Check if running
curl http://localhost:9999/api/ping

# If fails, restart proxy
cd /home/datdang/working/common_dev/ai_vscode_debug/ai-debug-proxy
npm start &
```

### Issue: Binary Not Found

```bash
# Rebuild
cd /home/datdang/working/common_dev/embedded_hsm
cmake --build build --target ehsm_host
```

### Issue: Tests Not Failing

```bash
# Verify bugs injected
git status  # Should show modified files
cat BUG_INJECTION_COMPLETE.md  # Verify injection status
```

---

## 📝 BEST PRACTICES

### For AI Agents

1. ✅ **Always source ai-debug.sh first**
2. ✅ **Gather full context before debugging**
3. ✅ **Set breakpoints strategically**
4. ✅ **Step through slowly**
5. ✅ **Check variables frequently**
6. ✅ **Compare expected vs actual**
7. ✅ **Document evidence for each bug**
8. ✅ **Assign confidence levels**

### For Humans

1. ✅ **Read SKILLS.md before starting**
2. ✅ **Use AI_DEBUG_PROMPT.md as template**
3. ✅ **Review BUG_INJECTION_COMPLETE.md for expected bugs**
4. ✅ **Compare AI results with expected**
5. ✅ **Document improvements needed**

---

## 🎓 LEARNING RESOURCES

### Beginner

1. Read: `AI_DEBUG_EXTENSION_TEST_REPORT.md`
2. Follow: `SKILLS.md` basic workflow
3. Practice: Simple bug detection

### Intermediate

1. Read: `BUG_INJECTION_REPORT.md`
2. Follow: `SKILLS.md` advanced techniques
3. Practice: Multiple bug detection

### Advanced

1. Read: `AI_DEBUG_EFFECTIVENESS_REPORT.md`
2. Analyze: AI performance patterns
3. Optimize: Debug workflows

---

## 🚀 NEXT STEPS

### Immediate

1. ✅ Test AI agents with `AI_DEBUG_PROMPT.md`
2. ✅ Measure AI performance
3. ✅ Document results

### Short-term

1. Inject remaining bugs (BUG-008, BUG-009)
2. Add more test cases
3. Improve AI prompts

### Long-term

1. Automate bug detection
2. Create AI training dataset
3. Build bug pattern library

---

## 📞 SUPPORT

### Documentation Issues
- Check: All .md files in `/embedded_hsm/`
- Reference: `SKILLS.md` for workflows

### Extension Issues
- Check: Proxy running at localhost:9999
- Reference: `AI_DEBUG_EFFECTIVENESS_REPORT.md`

### Bug Detection Issues
- Check: `BUG_INJECTION_COMPLETE.md`
- Reference: `BUG_INJECTION_REPORT.md`

---

## 🏆 CONCLUSION

This guide provides:

✅ **Complete AI debugging workflow**  
✅ **Ready-to-use prompts**  
✅ **Bug specifications**  
✅ **Performance metrics**  
✅ **Best practices**  
✅ **Troubleshooting guide**  

**AI Debug Extension is production-ready for AI-assisted debugging!**

---

**Created:** March 13, 2026  
**Branch:** ai-debug-training  
**Status:** ✅ Complete and Ready
