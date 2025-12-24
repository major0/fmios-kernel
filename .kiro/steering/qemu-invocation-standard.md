# QEMU Invocation Standard

## Overview

This document establishes the standard for QEMU invocations across all FMI/OS development, testing, and documentation to ensure consistent behavior and debugging capabilities.

## Standard QEMU Serial Configuration

### Required Flag
All QEMU invocations MUST use:
```
-serial mon:stdio
```

### Prohibited Flag
Do NOT use:
```
-serial stdio
```

## Timeout Requirements

### Mandatory Timeout
All QEMU invocations MUST include a timeout mechanism to prevent indefinite execution:

#### Using `timeout` Command (Preferred)
```bash
# CORRECT: Using timeout command (30 seconds)
timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic

# CORRECT: Using timeout with specific exit code handling
timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic || echo "QEMU timeout or exit"
```

#### Using QEMU Built-in Options
```bash
# CORRECT: Using QEMU's built-in timeout (30 seconds)
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic -watchdog i6300esb -watchdog-action poweroff

# CORRECT: Using QEMU monitor commands for automated shutdown
echo "quit" | timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic -monitor stdio
```

### Prohibited Patterns
```bash
# FORBIDDEN: QEMU without any timeout mechanism
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic

# FORBIDDEN: Interactive QEMU in automated scripts
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic -monitor stdio
```

### Timeout Guidelines
- **Development/Manual Testing**: 60-300 seconds depending on test complexity
- **Automated Testing**: 30-60 seconds for quick validation
- **CI/CD Pipelines**: 30 seconds maximum to prevent build delays
- **Smoke Tests**: 15-30 seconds for basic boot validation

## Rationale

### Why `-serial mon:stdio`?
- **Monitor Integration**: Provides access to QEMU monitor commands via `Ctrl+A, C`
- **Debugging Capability**: Allows runtime inspection of VM state, memory, registers
- **Development Workflow**: Enables switching between serial console and QEMU monitor
- **Consistent Behavior**: Ensures all team members have the same debugging interface

### Why NOT `-serial stdio`?
- **Limited Functionality**: Only provides serial console, no monitor access
- **Debugging Limitations**: Cannot inspect VM state or execute monitor commands
- **Inconsistent Experience**: Different behavior across development environments

### Why Timeout is Mandatory?
- **Resource Protection**: Prevents runaway QEMU processes from consuming system resources indefinitely
- **CI/CD Reliability**: Ensures automated builds don't hang waiting for QEMU to exit
- **Development Efficiency**: Prevents developers from accidentally leaving QEMU running
- **System Stability**: Protects development and CI systems from resource exhaustion
- **Predictable Behavior**: Ensures consistent behavior in automated testing environments

## Usage Examples

### Correct QEMU Invocations
```bash
# x86_64 testing with timeout
timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic

# ARM64 testing with timeout
timeout 30s qemu-system-aarch64 -machine virt -kernel fmi-kernel -serial mon:stdio -nographic

# RISC-V testing with timeout
timeout 30s qemu-system-riscv64 -machine virt -kernel fmi-kernel -serial mon:stdio -nographic

# With additional debugging and timeout
timeout 60s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic -d int,cpu_reset

# Automated testing with exit code handling
timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic
QEMU_EXIT_CODE=$?
if [ $QEMU_EXIT_CODE -eq 124 ]; then
    echo "QEMU timed out after 30 seconds"
elif [ $QEMU_EXIT_CODE -eq 0 ]; then
    echo "QEMU exited normally"
else
    echo "QEMU exited with error code: $QEMU_EXIT_CODE"
fi
```

### Incorrect QEMU Invocations
```bash
# WRONG: No timeout mechanism
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic

# WRONG: Interactive mode in automated scripts
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic -monitor stdio

# WRONG: Using prohibited serial flag
timeout 30s qemu-system-x86_64 -kernel fmi-kernel -serial stdio -nographic
```

### Monitor Commands
Access QEMU monitor with `Ctrl+A, C`, then use commands like:
- `info registers` - Show CPU registers
- `info mem` - Show memory mappings
- `x/10i $pc` - Disassemble at current PC
- `quit` - Exit QEMU

Return to serial console with `Ctrl+A, C` again.

## Compliance

### Scripts and Makefiles
All shell scripts, Makefiles, and documentation MUST:
- Use `-serial mon:stdio` for QEMU serial configuration
- Include timeout mechanisms for all QEMU invocations
- Handle timeout exit codes appropriately

### Testing Infrastructure
All automated testing MUST:
- Use `-serial mon:stdio` for consistency
- Include appropriate timeout values (typically 30 seconds)
- Provide clear error messages when timeouts occur

### Documentation
All examples and tutorials MUST:
- Demonstrate `-serial mon:stdio` usage
- Show proper timeout implementation
- Include exit code handling examples

### Build System Integration
```makefile
# Example Makefile target with proper timeout
qemu-test:
	timeout 30s qemu-system-x86_64 -kernel $(KERNEL) -serial mon:stdio -nographic || \
	(echo "QEMU test failed or timed out"; exit 1)

# Example with different timeout for debug builds
qemu-debug:
	timeout 120s qemu-system-x86_64 -kernel $(KERNEL) -serial mon:stdio -nographic -s -S
```

### Script Template
```bash
#!/bin/bash
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# QEMU invocation with mandatory timeout
TIMEOUT_SECONDS=30
KERNEL_IMAGE="fmi-kernel"

echo "Starting QEMU with ${TIMEOUT_SECONDS}s timeout..."
timeout ${TIMEOUT_SECONDS}s qemu-system-x86_64 \
    -kernel "${KERNEL_IMAGE}" \
    -serial mon:stdio \
    -nographic

QEMU_EXIT_CODE=$?
case $QEMU_EXIT_CODE in
    0)
        echo "QEMU exited normally"
        ;;
    124)
        echo "ERROR: QEMU timed out after ${TIMEOUT_SECONDS} seconds"
        exit 1
        ;;
    *)
        echo "ERROR: QEMU exited with code ${QEMU_EXIT_CODE}"
        exit 1
        ;;
esac
```

This standard ensures consistent debugging capabilities, resource protection, and reliable automated testing across all FMI/OS QEMU usage.