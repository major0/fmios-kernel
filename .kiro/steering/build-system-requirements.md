---
inclusion: always
---

# Build System Requirements for All Specs

All FMI/OS subsystems must comply with the autoconf/automake-based build system requirements. This ensures consistent compilation, testing, and deployment across all kernel components.

## Core Build System Principles

### Autoconf/Automake Integration
- **Configuration Management**: Use autoconf/automake for all build configuration
- **Architecture Selection**: Support `--target=x86_64-fmios`, `--target=aarch64-fmios`, `--target=riscv64-fmios`
- **Feature Configuration**: Provide configure options for enabling/disabling subsystem features
- **Cross-Compilation**: Seamless support for multiple target architectures

### Architecture Separation
- **Generic Code**: Top-level `include/` and `lib/` directories for architecture-neutral code
- **Architecture-Specific Code**: `arch/<arch>/include/`, `arch/<arch>/lib/`, `arch/<arch>/drivers/`
- **Symlink Management**: `include/arch` and `lib/arch` symlinks managed by build system
- **Clean Separation**: Generic code must not contain inline assembly or endian-specific code

## Dual Klibc Implementation Strategy

### Kernel Klibc (`kernel/lib/c/`)
- **Freestanding Environment**: Compiled with `-ffreestanding -nostdlib -fno-builtin`
- **Self-Contained**: No dependencies on external libraries or system calls
- **Essential Functions**: kprintf, klogf, kmalloc, kfree, kpanic, etc.
- **Compiler Headers**: May use compiler-provided headers (stddef.h, stdint.h, stdbool.h, stdatomic.h)

### Testing Klibc (`tests/klibc/`)
- **Hosted Environment**: Compiled without `-ffreestanding` for standard library access
- **Wrapper Functions**: Wraps standard libc calls for testing purposes
- **Interface Parity**: Identical interface to kernel klibc (no extra or missing functions)
- **Debugging Features**: Additional instrumentation and debugging capabilities

### Build System Selection
- **Kernel Builds**: Link against `-Llib/c/` and `-Llib/arch/`
- **Test Builds**: Link against `-Ltests/klibc/`
- **Automatic Selection**: Linker flags select appropriate implementation without preprocessor directives

## QEMU Integration Testing

### Smoke Test Requirements
- **Real Environment Testing**: Test kernel components in actual kernel environment using QEMU
- **Multi-Architecture Support**: x86_64, ARM64, RISC-V testing in QEMU
- **Automated Execution**: Integration with build system for continuous testing
- **Console Capture**: Automated validation of kernel output through serial interface

### QEMU Configuration
- **Serial Interface**: Configure QEMU stdin/stdout as serial interface to kernel
- **Console Parameters**: Set kernel console parameters to use serial interface
- **Output Capture**: Use QEMU serial redirection for automated validation
- **Boot Validation**: Ensure kernel boots, executes tests, and shuts down cleanly

## Compiler Flags and Standards

### Kernel Build Flags
```bash
# Required kernel flags
KERNEL_CFLAGS="-std=c11 -Wall -Wextra -Werror -ffreestanding -nostdlib -fno-builtin"
KERNEL_CFLAGS="$KERNEL_CFLAGS -fPIC -fno-plt"  # Position Independent Code
KERNEL_CFLAGS="$KERNEL_CFLAGS -mretpoline -mindirect-branch=thunk-extern"  # Spectre mitigations
KERNEL_CFLAGS="$KERNEL_CFLAGS -fcf-protection=full -fstack-protector-strong"  # Security
```

### Test Build Flags
```bash
# Test environment flags
TEST_CFLAGS="-std=c11 -Wall -Wextra -Werror -O0 -g -D_POSIX_C_SOURCE=200809L"
```

### Architecture-Specific Flags
- **x86_64**: `-mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2`
- **ARM64**: `-mcpu=cortex-a57 -mgeneral-regs-only`
- **RISC-V**: `-march=rv64imac -mabi=lp64`

## Shell Script Standards

### POSIX Compliance
- **POSIX Shell Syntax**: All shell scripts use POSIX syntax without Bash extensions
- **POSIXLY_CORRECT**: Set `POSIXLY_CORRECT='No bashing shell'` in all scripts
- **Case Patterns**: Use `(label)` instead of `label)` in case statements
- **Variable References**: Use `${var}` form, not `$var`
- **Conditional Expressions**: Use `test` command, not `[ ... ]` brackets
- **String Quoting**: Single quotes for literals, double quotes for expansions

## Memory Reclamation and Linker Scripts

### Initialization Code Grouping
- **Linker Sections**: Group initialization code in dedicated memory segments
- **Section Attributes**: Mark initializers with `__attribute__((section(".init.text")))`
- **Memory Reclamation**: Provide `__init_start` and `__init_end` symbols
- **Runtime Cleanup**: Free initialization memory after scheduler startup

### Bootstrap Integration
- **Bootstrap Sections**: Include bootstrap code in `.init.text` segment
- **One-Time Code**: Mark all initialization-only code for reclamation
- **Runtime Separation**: Ensure no runtime functions in reclaimable segments

## Code Quality Enforcement

### Forward Declaration Prohibition
- **Header Includes**: All source files must use proper header includes
- **No Custom Declarations**: Prohibit custom forward declarations for existing interfaces
- **Build Validation**: Detect and prevent duplicate interface declarations
- **Maintainability**: Ensure single source of truth for all declarations

### Interface Compliance
- **Consistent APIs**: All subsystems use standardized interface patterns
- **Error Handling**: Consistent error codes and handling across subsystems
- **Documentation**: Clear interface documentation and usage examples

## Testing Integration

### Unit Testing Framework
- **Comprehensive Coverage**: Unit tests for all subsystem components
- **Property-Based Testing**: Universal property validation across inputs
- **Integration Testing**: Cross-subsystem interaction validation
- **Performance Testing**: Validate performance characteristics and requirements

### Continuous Integration
- **Automated Building**: All architectures and configurations
- **Test Execution**: Unit tests, integration tests, and QEMU smoke tests
- **Quality Gates**: All tests must pass before code integration
- **Performance Monitoring**: Track performance metrics and regressions

This build system framework ensures consistent, reliable, and maintainable builds across all FMI/OS subsystems while supporting multiple architectures and comprehensive testing.