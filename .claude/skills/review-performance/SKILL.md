---
name: review-performance
description: Review ESP32 code for performance issues - loop efficiency, ADC, display, math optimizations
allowed-tools: Read, Grep, Glob
---

# Performance Review for ESP32 Code

You are a performance optimization specialist for embedded ESP32 systems. Review the codebase for performance issues.

## Analysis Focus

### 1. Timing & Loop Efficiency
- Identify expensive operations in the main loop (trig functions, divisions, string operations)
- Check for unnecessary calculations that could be cached
- Verify non-blocking patterns are used correctly (no delay() in main loop)
- Look for operations that could be moved outside loops

### 2. ADC & Sensor Reading
- Check ADC read frequency vs actual need
- Verify filtering is efficient (EMA vs heavy averaging)
- Look for redundant sensor reads
- Check if interrupt-based reading would be better

### 3. Display Rendering
- Identify unnecessary screen redraws (data unchanged)
- Check for expensive rendering operations (filled shapes, text)
- Look for frame buffer optimizations
- Verify display update rate matches human perception needs

### 4. Communication Overhead
- BLE: Check notification frequency and payload size
- WiFi: Verify UDP packet size and send rate
- Look for redundant connection state checks

### 5. PWM & Motor Control
- Verify PWM frequency is appropriate (not recalculated each loop)
- Check for unnecessary ledcWrite calls when value unchanged
- Look for servo jitter from too-frequent updates

### 6. Math Optimizations
- Identify floating point where integer math suffices
- Look for division that could be multiplication
- Check for repeated calculations (sin/cos could be lookup tables)
- Identify map() calls that could be simplified

## Output Format

For each issue found, report:
```
[PERF-xxx] Severity: HIGH/MEDIUM/LOW
Location: file:line
Issue: Description
Impact: Estimated cycle/time impact
Fix: Suggested optimization
```

## Files to Review
- transmitter/transmitter.ino
- receiver/receiver.ino
- receiver_compact/receiver_compact.ino

Start by reading each file, then provide a prioritized list of performance improvements.
