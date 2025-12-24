# Multiboot2 QEMU Testing Standard

## Overview

This document establishes the mandatory use of GRUB bootloader for all QEMU testing of multiboot2 targets in FMI/OS development. This ensures consistent boot behavior, proper multiboot2 protocol compliance, and reliable testing across all development environments.

## Core Requirement

**ALL multiboot2 kernel testing in QEMU MUST use the GRUB bootloader.** Direct kernel loading (`-kernel` flag) is prohibited for multiboot2 targets as it bypasses the multiboot2 protocol and can mask boot-related issues.

## Multiboot2 Target Identification

A target is considered multiboot2 if:
- The kernel binary contains a multiboot2 header
- The kernel is designed to be loaded by a multiboot2-compliant bootloader
- The kernel expects multiboot2 boot information structures
- The kernel uses multiboot2-specific features (modules, memory maps, etc.)

## Required QEMU Configuration

### Mandatory GRUB Usage
```bash
# CORRECT: Using GRUB bootloader for multiboot2 kernel
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic

# CORRECT: Using GRUB with specific boot device
qemu-system-x86_64 -drive file=fmios.iso,format=raw,if=ide -serial mon:stdio -nographic

# INCORRECT: Direct kernel loading bypasses multiboot2 protocol
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic  # PROHIBITED
```

### ISO Image Requirements
All multiboot2 testing MUST use ISO images containing:
- **GRUB Bootloader**: Properly configured GRUB installation
- **Kernel Binary**: Multiboot2-compliant kernel with proper header
- **GRUB Configuration**: Valid `grub.cfg` with kernel boot entry
- **Boot Structure**: ISO9660 filesystem with El Torito boot record

### Standard QEMU Flags
In addition to GRUB requirements, all invocations MUST follow the existing QEMU invocation standard:
- **Serial Configuration**: `-serial mon:stdio` (never `-serial stdio`)
- **Graphics Disable**: `-nographic` for headless testing
- **Monitor Access**: Available via `Ctrl+A, C`

## Build System Integration

### ISO Generation Requirement
The build system MUST provide ISO generation targets for multiboot2 kernels:

```makefile
# Required target for multiboot2 testing
fmios.iso: fmi-kernel grub.cfg
	$(MAKE_ISO_SCRIPT) $@ $^

# QEMU testing target
qemu-test-multiboot2: fmios.iso
	qemu-system-x86_64 -cdrom $< -serial mon:stdio -nographic
```

### GRUB Configuration Management
The build system MUST maintain proper GRUB configuration:

```bash
# Example grub.cfg for multiboot2 kernel
menuentry "FMI/OS Kernel" {
    multiboot2 /boot/fmi-kernel
    boot
}
```

## Testing Script Requirements

### Automated Testing Scripts
All automated testing scripts for multiboot2 targets MUST:

```bash
#!/bin/bash
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Verify ISO exists
if ! test -f fmios.iso; then
    echo "ERROR: fmios.iso not found - required for multiboot2 testing"
    exit 1
fi

# Use GRUB bootloader (REQUIRED)
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic -monitor none
```

### Prohibited Patterns
The following patterns are STRICTLY FORBIDDEN for multiboot2 targets:

```bash
# FORBIDDEN: Direct kernel loading
qemu-system-x86_64 -kernel fmi-kernel -serial mon:stdio -nographic

# FORBIDDEN: Bypassing bootloader
qemu-system-x86_64 -kernel fmi-kernel -initrd modules.img -serial mon:stdio -nographic

# FORBIDDEN: Non-standard serial configuration
qemu-system-x86_64 -cdrom fmios.iso -serial stdio -nographic
```

## Architecture-Specific Requirements

### x86_64 Multiboot2 Testing
```bash
# Standard x86_64 multiboot2 testing
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic

# With debugging enabled
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic -d int,cpu_reset

# With memory debugging
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic -d mmu,guest_errors
```

### ARM64 and RISC-V Considerations
- **ARM64**: If multiboot2 support is implemented, use appropriate GRUB for ARM64
- **RISC-V**: If multiboot2 support is implemented, use appropriate GRUB for RISC-V
- **Current Status**: Multiboot2 is primarily x86_64-focused; other architectures may use different boot protocols

## Validation and Compliance

### Build System Validation
The build system MUST validate multiboot2 compliance:

```bash
# Validate multiboot2 header presence
check_multiboot2_header() {
    if objdump -h fmi-kernel | grep -q ".multiboot2"; then
        echo "Multiboot2 header found - GRUB bootloader required"
        return 0
    fi
    return 1
}

# Enforce GRUB usage for multiboot2 targets
validate_qemu_invocation() {
    if check_multiboot2_header; then
        if echo "$QEMU_CMD" | grep -q "\-kernel"; then
            echo "ERROR: Multiboot2 kernel detected but using -kernel flag"
            echo "REQUIRED: Use GRUB bootloader with ISO image"
            exit 1
        fi
    fi
}
```

### Testing Infrastructure Integration
All continuous integration MUST enforce this standard:

```yaml
# Example CI configuration
test_multiboot2:
  script:
    - make fmios.iso
    - ./scripts/validate-multiboot2-qemu.sh
    - qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic -monitor none
```

## Rationale

### Why GRUB is Mandatory for Multiboot2

1. **Protocol Compliance**: GRUB properly implements the multiboot2 specification
2. **Boot Information**: GRUB provides correct multiboot2 boot information structures
3. **Module Loading**: GRUB handles multiboot2 module loading correctly
4. **Memory Layout**: GRUB establishes proper memory layout as per multiboot2 spec
5. **Real-World Testing**: Tests the actual boot path used in production

### Why Direct Kernel Loading is Prohibited

1. **Protocol Bypass**: `-kernel` flag bypasses multiboot2 protocol entirely
2. **Missing Boot Info**: QEMU doesn't provide multiboot2 boot information structures
3. **False Positives**: Kernel may appear to work but fail with real bootloader
4. **Module Issues**: Cannot test multiboot2 module loading functionality
5. **Memory Layout**: Different memory layout than multiboot2 specification

### Benefits of This Standard

1. **Consistency**: All multiboot2 testing uses the same boot path
2. **Reliability**: Catches boot-related issues early in development
3. **Compliance**: Ensures multiboot2 specification adherence
4. **Production Parity**: Testing matches production boot environment
5. **Debugging**: GRUB provides additional debugging information

## Error Detection and Prevention

### Automated Detection
Scripts MUST detect and prevent incorrect usage:

```bash
# Detect multiboot2 kernel and enforce GRUB usage
detect_multiboot2_violation() {
    local qemu_cmd="$1"
    
    # Check if kernel has multiboot2 header
    if objdump -s -j .multiboot2 fmi-kernel >/dev/null 2>&1; then
        # Multiboot2 kernel detected
        if echo "$qemu_cmd" | grep -q "\-kernel"; then
            echo "VIOLATION: Multiboot2 kernel using -kernel flag"
            echo "REQUIRED: qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic"
            return 1
        fi
    fi
    return 0
}
```

### Build System Integration
```makefile
# Validate QEMU command for multiboot2 compliance
validate-qemu-multiboot2:
	@if objdump -s -j .multiboot2 fmi-kernel >/dev/null 2>&1; then \
		echo "Multiboot2 kernel detected - validating QEMU usage..."; \
		if test "$(QEMU_FLAGS)" != "-cdrom fmios.iso -serial mon:stdio -nographic"; then \
			echo "ERROR: Invalid QEMU flags for multiboot2 kernel"; \
			echo "REQUIRED: -cdrom fmios.iso -serial mon:stdio -nographic"; \
			exit 1; \
		fi; \
	fi

qemu-test: validate-qemu-multiboot2 fmios.iso
	qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic
```

## Documentation Requirements

### Code Comments
All QEMU invocations in code MUST include explanatory comments:

```bash
# Use GRUB bootloader for multiboot2 protocol compliance
# Direct kernel loading (-kernel) is prohibited for multiboot2 targets
qemu-system-x86_64 -cdrom fmios.iso -serial mon:stdio -nographic
```

### Script Headers
All testing scripts MUST include standard headers:

```bash
#!/bin/bash
# Multiboot2 QEMU Testing Script
# REQUIREMENT: Uses GRUB bootloader for multiboot2 protocol compliance
# PROHIBITION: Direct kernel loading (-kernel flag) is not permitted

POSIXLY_CORRECT='No bashing shell'
```

## Compliance Verification

All FMI/OS development MUST demonstrate:

1. **ISO Generation**: Proper ISO images with GRUB bootloader
2. **QEMU Usage**: Correct QEMU invocation with `-cdrom` flag
3. **Serial Configuration**: Standard `-serial mon:stdio` usage
4. **Build Integration**: Automated validation in build system
5. **Script Compliance**: All testing scripts follow this standard
6. **Documentation**: Clear indication of multiboot2 requirements

This standard ensures that multiboot2 kernel development and testing maintains the highest level of boot protocol compliance and reliability across all FMI/OS development activities.