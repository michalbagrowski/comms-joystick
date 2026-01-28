---
name: review-all
description: Comprehensive ESP32 code review - safety, bugs, performance, memory, code quality
allowed-tools: Read, Grep, Glob
---

# Comprehensive Code Review for ESP32 Joystick Project

Run a complete code review covering all aspects. This is the "full audit" command.

## Review Scope

Execute ALL review categories in sequence:

### 1. Safety Review (HIGHEST PRIORITY)
- Motor runaway prevention
- Communication loss handling
- Battery safety
- Startup safety
- Input validation

### 2. Bug Hunting
- Integer overflow/underflow
- Race conditions
- State machine bugs
- Boundary conditions
- Error handling gaps

### 3. Performance Analysis
- Loop efficiency
- Display optimization
- Communication overhead
- Math optimizations

### 4. Memory Review
- Static/heap usage
- Stack safety
- Memory leaks
- Buffer sizing

### 5. Code Quality
- Magic numbers (extract to constants)
- Code duplication across files
- Naming consistency
- Comment quality

### 6. Configuration Consistency
- Pin assignments match CLAUDE.md
- Joystick center values match hardware
- Motor deadzone consistent
- PWM frequencies appropriate

## Output Format

Provide a summary report:

```
=== ESP32 JOYSTICK CODE REVIEW ===

CRITICAL ISSUES (fix immediately):
- [SAFETY-001] ...
- [BUG-001] ...

HIGH PRIORITY (fix soon):
- [PERF-001] ...
- [MEM-001] ...

MEDIUM PRIORITY (improve when convenient):
- ...

LOW PRIORITY (nice to have):
- ...

STATISTICS:
- Total issues found: X
- Critical: X
- High: X
- Medium: X
- Low: X

POSITIVE FINDINGS:
- Good practices observed
- Well-implemented patterns
```

## Files to Review
- transmitter/transmitter.ino
- receiver/receiver.ino
- receiver_compact/receiver_compact.ino
- CLAUDE.md (for reference)

## Focus Areas by File

**transmitter.ino:**
- Joystick reading accuracy
- Display rendering efficiency
- BLE transmission reliability
- Battery monitoring

**receiver.ino:**
- Motor control safety
- Servo precision
- Failsafe behavior
- Command validation

**receiver_compact.ino:**
- DRV8833 driver differences
- XIAO-specific considerations
- Size optimization tradeoffs

Start the review now and provide actionable findings.
