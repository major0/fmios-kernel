# Build System Guidelines

This directory contains guidelines for the autoconf/automake-based build system and compilation requirements.

## Documents

### [autoconf-build-system.md](autoconf-build-system.md)
Comprehensive guidelines for the FMI/OS autoconf/automake build system:
- Directory structure requirements with architecture separation
- Configure script requirements for target architecture selection
- Makefile.am structure and organization
- Compiler and linker flag management
- Code quality enforcement including forward declaration prohibition

### [build-flags.md](build-flags.md)
Specifies compiler flags for different build targets:
- Kernel build flags (`-ffreestanding`, `-nostdlib`, etc.)
- Test build flags (hosted environment)
- Architecture-specific flags for x86_64, ARM64, RISC-V
- Include and linker path configuration

### [include-paths.md](include-paths.md)
Guidelines for proper use of `#include` directives:
- Architecture separation with symlink management
- Autoconf/automake integration for include paths
- Forward declaration prohibition rules
- Standard vs. project header usage patterns

## Key Features

1. **Multi-Architecture Support** - Clean separation and cross-compilation
2. **Code Quality Enforcement** - Automated detection of architectural violations
3. **Standard Compliance** - Uses GNU autotools for maximum portability
4. **Testing Integration** - Dual klibc strategy and QEMU smoke tests

The build system provides a clean, maintainable, and standards-compliant approach to building FMI/OS across multiple architectures with proper feature configuration and code quality enforcement.
