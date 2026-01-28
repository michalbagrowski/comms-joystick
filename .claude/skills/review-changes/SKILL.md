---
name: review-changes
description: Review staged/unstaged git changes for safety, bugs, and protocol compatibility before committing
allowed-tools: Read, Grep, Glob, Bash
---

# Review Staged/Unstaged Changes

Review the current git changes for issues before committing.

## Process

1. Run `git diff` and `git diff --staged` to see all changes
2. Analyze each change for:
   - Safety implications (motor control, battery)
   - Bug introduction
   - Performance regressions
   - Memory issues
   - Breaking changes to communication protocol

## Review Checklist

### Safety
- [ ] Motor control changes don't introduce runaway risk
- [ ] Battery thresholds unchanged or improved
- [ ] Failsafe behavior preserved
- [ ] Motor enable circuit logic intact

### Protocol Compatibility
- [ ] JoystickData struct unchanged (or both TX/RX updated)
- [ ] BLE UUIDs unchanged
- [ ] WiFi protocol compatible

### Code Quality
- [ ] No new magic numbers introduced
- [ ] No unnecessary code duplication
- [ ] Follows existing patterns
- [ ] Comments explain why, not what

### Testing
- [ ] Changes are testable
- [ ] No obvious broken edge cases

## Output Format

```
=== CHANGE REVIEW ===

Files Changed: X

CONCERNS:
- [file:line] Issue description

SUGGESTIONS:
- Improvement ideas

VERDICT: APPROVE / NEEDS CHANGES / REJECT

Reason: Brief explanation
```

Run `git diff HEAD` now and review all pending changes.
