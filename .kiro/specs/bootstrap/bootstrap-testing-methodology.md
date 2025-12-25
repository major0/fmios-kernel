# Bootstrap Testing Methodology

## Overview

Bootstrap code testing for FMI/OS **MUST** be performed exclusively through QEMU smoke testing using a custom minimal test kernel. Traditional unit testing is not applicable for bootstrap code due to its hardware-dependent nature and early execution context.

## Testing Requirements

### Mandatory QEMU Testing

**All bootstrap code validation MUST use QEMU smoke testing with the following requirements:**

1. **No Unit Testing**: Bootstrap assembly code cannot be unit tested due to hardware dependencies
2. **QEMU-Only Validation**: All testing must be performed in QEMU virtual machines
3. **Custom Test Kernel**: Use a minimal test kernel specifically designed for bootstrap validation
4. **Serial Console Output**: All test output must go through serial console for automated capture
5. **Dynamic Discovery**: Test kernel must dynamically discover and report system information

### Custom Test Kernel Requirements

The minimal test kernel **MUST**:

1. **Minimal Implementation**: Only implement essential functionality for testing
2. **Serial Console Only**: Use simple serial console output (no VGA, no complex drivers)
3. **Dynamic Information Discovery**: Discover system information through bootstrap-provided data
4. **Environment Validation**: Validate that discovered information matches QEMU parameters
5. **Clean Shutdown**: Perform proper shutdown after displaying information

### Required System Information Display

The test kernel **MUST** dynamically discover and display:

1. **Architecture Information**:
  - Target architecture (x86_64, arm64, riscv64)
  - CPU type and model
  - Number of CPU cores
  - CPU features (long mode, PAE, etc.)

2. **Memory Information**:
  - Total physical memory
  - Usable memory regions
  - Memory map from bootloader
  - Number of memory regions

3. **Boot Protocol Information**:
  - Detected boot protocol (Multiboot v2, PVH ELF, U-Boot, UEFI)
  - Bootloader-provided information structures
  - Command line parameters

4. **Hardware Information**:
  - Available hardware devices
  - System configuration
  - Any other bootloader-provided hardware information

### QEMU Test Configuration

**Standard QEMU invocations for bootstrap testing:**

```bash
# x86_64 bootstrap testing
qemu-system-x86_64 \
  -kernel tests/bootstrap/test-kernel-x86_64 \
  -serial mon:stdio \
  -nographic \
  -m 512M \
  -smp 2 \
  -append "console=ttyS0 test=bootstrap arch=x86_64"

# ARM64 bootstrap testing
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a57 \
  -kernel tests/bootstrap/test-kernel-arm64 \
  -serial mon:stdio \
  -nographic \
  -m 512M \
  -smp 2 \
  -append "console=ttyS0 test=bootstrap arch=arm64"

# RISC-V bootstrap testing
qemu-system-riscv64 \
  -machine virt \
  -cpu rv64 \
  -kernel tests/bootstrap/test-kernel-riscv64 \
  -serial mon:stdio \
  -nographic \
  -m 512M \
  -smp 2 \
  -append "console=ttyS0 test=bootstrap arch=riscv64"
```

### Environment Validation Requirements

The test kernel **MUST** validate that discovered information matches QEMU parameters:

1. **Memory Validation**: Discovered memory should match `-m 512M` parameter
2. **CPU Validation**: Discovered CPU count should match `-smp 2` parameter
3. **Architecture Validation**: Discovered architecture should match expected target
4. **Command Line Validation**: Discovered command line should match `-append` parameter

### Expected Output Format

**Standard output format for all architectures:**

```
=== Bootstrap Test Kernel ===
Architecture: x86_64
CPU Type: QEMU Virtual CPU
CPU Cores: 2
CPU Features: long_mode pae pse nx
Total Memory: 512 MB
Usable Memory: 510 MB
Memory Regions: 3
Boot Protocol: Multiboot v2
Command Line: console=ttyS0 test=bootstrap arch=x86_64
=== Environment Validation ===
Memory: PASS (512 MB matches QEMU -m parameter)
CPU Cores: PASS (2 cores matches QEMU -smp parameter)
Architecture: PASS (x86_64 matches expected)
Command Line: PASS (matches QEMU -append parameter)
=== Bootstrap Test PASSED ===
Shutting down...
```

### Automated Test Validation

**Test validation scripts MUST:**

1. **Pattern Matching**: Validate expected output patterns are present
2. **Environment Correlation**: Verify discovered values match QEMU parameters
3. **Error Detection**: Detect and report any bootstrap failures
4. **Cross-Architecture**: Work consistently across all supported architectures

### Test Kernel Implementation Guidelines

**The minimal test kernel MUST:**

1. **Use Bootstrap Data**: Access all information through bootstrap-provided structures
2. **Minimal Dependencies**: No complex kernel subsystems or drivers
3. **Serial Output Only**: Simple character-by-character serial output
4. **Error Handling**: Graceful handling of missing or invalid bootstrap data
5. **Architecture Agnostic**: Same test kernel source works on all architectures

### Integration with Build System

**Build system integration requirements:**

```makefile
# Bootstrap testing targets
check-bootstrap: test-kernel-x86_64 test-kernel-arm64 test-kernel-riscv64
  @echo "Running bootstrap QEMU smoke tests..."
  ./scripts/qemu-bootstrap-test.sh x86_64 512M 2
  ./scripts/qemu-bootstrap-test.sh arm64 512M 2
  ./scripts/qemu-bootstrap-test.sh riscv64 512M 2
  @echo "All bootstrap tests passed"

# Test kernel builds
test-kernel-%: tests/bootstrap/test_kernel.c arch/%/bootstrap.S
  $(CC_FOR_ARCH) $(TEST_KERNEL_CFLAGS) -o $@ $^
```

## Rationale

### Why QEMU-Only Testing?

1. **Hardware Dependencies**: Bootstrap code directly interacts with CPU, memory, and bootloader
2. **Early Execution**: Runs before any kernel infrastructure is available
3. **Architecture-Specific**: Contains assembly code that requires actual execution environment
4. **Bootloader Integration**: Requires real bootloader handoff and data structures

### Why Custom Test Kernel?

1. **Minimal Overhead**: No complex kernel subsystems that could mask bootstrap issues
2. **Focused Testing**: Specifically designed to validate bootstrap functionality
3. **Automated Validation**: Produces predictable output for automated testing
4. **Cross-Architecture**: Single implementation works across all supported architectures

### Why Dynamic Discovery?

1. **Real Validation**: Ensures bootstrap code actually discovers and processes system information
2. **Environment Correlation**: Validates that bootstrap correctly interprets bootloader data
3. **Regression Detection**: Catches issues where bootstrap fails to detect system resources
4. **Comprehensive Coverage**: Tests all aspects of bootstrap information gathering

This methodology ensures comprehensive, reliable, and automated testing of bootstrap functionality across all supported architectures while maintaining the simplicity and focus required for early kernel initialization code.
