# Implementation Plan: Build System

## Overview

This implementation plan establishes an incremental autoconf/automake-based build system for FMI/OS, following the stage-by-stage development approach. The build system grows incrementally alongside kernel features, starting with minimal x86_64 support and expanding to full multi-architecture capabilities.

**INCREMENTAL APPROACH**:
- Stage 1: Basic x86_64 autoconf/automake for Hello World kernel
- Stage 2: Add ISO generation for Multiboot2 support
- Stage 3: Add UEFI application generation
- Stage 4: Enhanced build features for expanded kernel functionality
- Stage 5: ARM64 cross-compilation support
- Stage 6: RISC-V cross-compilation support

## Tasks

### Stage 1: Basic x86_64 Build System

- [x] 1. Set up minimal autoconf/automake infrastructure for x86_64
**Priority: CRITICAL BLOCKER - Required for Stage 1 Hello World kernel**
**Dependencies: None**

Create minimal build system supporting only x86_64 PVH ELF boot for Stage 1.

- Create basic configure.ac with x86_64-only architecture detection
- Create minimal top-level Makefile.am for x86_64 kernel build
- Set up x86_64-specific compiler flags (kernel mode, no red zone, etc.)
- Configure basic include paths for x86_64 architecture
- Set up minimal kernel linking for PVH ELF format
- Support QEMU testing with `-kernel` flag for PVH ELF
- Exclude Multiboot2 and UEFI support initially
- Focus on minimal working build for Hello World kernel
- _Requirements: 1.1, 1.2, 1.3_

- [x] 1.1 Write property test for basic x86_64 build
  - **Property 1: Basic x86_64 Build Functionality**
  - **Validates: Requirements 1.1, 1.2, 1.3**

- [x] 1.2 Fix build system to use fully qualified paths
**Priority: CRITICAL BLOCKER - Required to fix property test path resolution**
**Dependencies: Task 1.1 (property test implementation)**

Update build system to determine fully qualified top-level directory path and express all other paths relative to topdir variable.

- Configure build system to determine absolute path to project root directory
- Create topdir variable in configure.ac and substitute into Makefiles
- Update all Makefile.am files to use $(topdir) for path references
- Modify property tests to use topdir-relative paths instead of relative paths
- Update QEMU test paths to use topdir-relative references
- Ensure all build artifacts use consistent topdir-based path resolution
- Validate that tests work correctly from any working directory
- _Requirements: 10.5, 12.4, 12.5_

### Stage 2: Multiboot2 Support

- [x] 2. Add ISO generation and GRUB support for Multiboot2
**Priority: HIGH FUNCTIONALITY - Required for Stage 2 Multiboot2 support**
**Dependencies: Task 1.2 (fully qualified path support)**

Extend build system to support Multiboot2 boot protocol via ISO generation.

- Add ISO generation capabilities to build system
- Create GRUB configuration generation for Multiboot2
- Set up proper ELF kernel format for GRUB loading
- Configure QEMU testing with `-cdrom` flag for ISO images
- Remove PVH ELF support (abandoned due to QEMU compatibility issues)
- Add build targets for Multiboot2 testing with GRUB
- Ensure kernel boots properly via GRUB bootloader
- _Requirements: 1.4, 8.1, 8.2_

- [x] 2.1 Fix 32-bit to 64-bit transition in Multiboot2 bootstrap
**Priority: CRITICAL BLOCKER - Required for proper kernel boot**
**Dependencies: Task 2 (basic Multiboot2 implementation)**

Fix the bootstrap code to ensure complete 32-bit to 64-bit mode transition.

- Debug output currently shows `1ABCDEFGH6OPQRS` but missing `IJKLMN` steps
- Missing steps: I=extended CPUID, J=long mode check, K=EFER read, L=long mode enable, M=paging enable, N=GDT load
- Root cause: Problematic CPUID calls causing system to skip transition steps
- Fix CPUID detection to work properly in QEMU Multiboot2 environment
- Ensure all debug steps execute in sequence: `1ABCDEFGHIJKLMN6OPQRS`
- Validate that 32-bit to 64-bit transition completes all required steps
- Test with `make clean && make && make qemu-test` to verify fix
- _Requirements: 1.4, 8.1_

- [x] 2.2 Write property test for ISO generation
  - **Property 2: ISO Generation and Boot Protocol Support**
  - **Validates: Requirements 1.4, 8.1, 8.2**

### Stage 3: UEFI Support

- [ ] 3. Add UEFI application generation
**Priority: HIGH FUNCTIONALITY - Required for Stage 3 UEFI support**
**Dependencies: Task 2.1 (fixed Multiboot2 bootstrap)**

Extend build system to support UEFI boot protocol.

- Add UEFI application (.efi) generation capabilities
- Configure UEFI-specific linking and format requirements
- Set up QEMU testing with OVMF firmware for UEFI boot
- Maintain support for both Multiboot2 and UEFI boot protocols
- Add build targets for comprehensive boot protocol testing
- Ensure consistent kernel behavior across all boot methods
- _Requirements: 8.3, 8.4, 8.5_

- [ ] 3.1 Write property test for UEFI application generation
  - **Property 3: UEFI Application Generation**
  - **Validates: Requirements 8.3, 8.4, 8.5**

### Stage 4: Enhanced Build Features

- [ ] 4. Implement enhanced build system features
**Priority: MEDIUM ENHANCEMENT - Required for Stage 4 feature expansion**
**Dependencies: Task 3 (UEFI support)**

Add advanced build system features to support expanded kernel functionality.

- Implement architecture separation and symlink management
- Add code quality enforcement (forward declaration detection)
- Configure dual klibc strategy (kernel vs testing)
- Add feature configuration support for kernel components
- Implement memory reclamation support for linker scripts
- Add comprehensive build validation and error checking
- Set up automated quality checks in build process
- _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 6.1, 6.2, 6.3, 7.1, 7.2, 11.1, 11.2_

- [ ] 4.1 Write property test for enhanced build features
  - **Property 4: Enhanced Build System Features**
  - **Validates: Requirements 3.1, 3.2, 6.1, 7.1, 11.1**

### Stage 5: ARM64 Cross-Compilation

- [ ] 5. Add ARM64 architecture support
**Priority: HIGH FUNCTIONALITY - Required for Stage 5 ARM64 port**
**Dependencies: Task 4 (enhanced build features)**

Extend build system to support ARM64 cross-compilation.

- Add ARM64 architecture detection and configuration
- Set up ARM64 cross-compilation toolchain support
- Configure ARM64-specific compiler and linker flags
- Add ARM64 U-Boot and UEFI boot support
- Set up QEMU testing for ARM64 architecture
- Implement ARM64-specific symlink management
- Ensure feature parity with x86_64 build capabilities
- _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_

- [ ] 5.1 Write property test for ARM64 cross-compilation
  - **Property 5: ARM64 Cross-Compilation Support**
  - **Validates: Requirements 9.1, 9.2, 9.3, 9.4**

### Stage 6: RISC-V Cross-Compilation

- [ ] 6. Add RISC-V architecture support
**Priority: HIGH FUNCTIONALITY - Required for Stage 6 RISC-V port**
**Dependencies: Task 5 (ARM64 support)**

Complete multi-architecture build system with RISC-V support.

- Add RISC-V architecture detection and configuration
- Set up RISC-V cross-compilation toolchain support
- Configure RISC-V-specific compiler and linker flags
- Add RISC-V U-Boot and UEFI boot support
- Set up QEMU testing for RISC-V architecture
- Implement comprehensive multi-architecture testing
- Validate feature parity across all three architectures
- _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_

- [ ] 6.1 Write property test for RISC-V cross-compilation
  - **Property 6: RISC-V Cross-Compilation Support**
  - **Validates: Requirements 9.1, 9.2, 9.3, 9.4**

### Continuous Integration Features

- [ ] 7. Implement comprehensive build validation
**Priority: MEDIUM ENHANCEMENT - Quality assurance across all stages**
**Dependencies: Task 6 (RISC-V support)**

Add comprehensive build validation and quality enforcement.

- Implement dependency and tool validation
- Add clear error messaging for configuration failures
- Create feature/architecture compatibility checking
- Add compiler flag compatibility validation
- Ensure path and symlink validation
- Set up QEMU testing integration for all architectures
- Add automated regression testing
- Implement performance monitoring for build times
- _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5, 12.1, 12.2, 12.3, 12.4, 12.5_

- [ ] 7.1 Write property test for build validation
  - **Property 7: Comprehensive Build Validation**
  - **Validates: Requirements 10.4, 10.5, 12.1, 12.2, 12.3**

- [ ] 8. Final integration and testing
**Priority: HIGH FUNCTIONALITY - Complete system validation**
**Dependencies: Task 7 (build validation)**

Integrate all components into complete build system and validate thoroughly.

- Test all supported architectures and feature combinations
- Validate cross-compilation scenarios for all architectures
- Run comprehensive test suite across all stages
- Ensure backward compatibility with earlier stages
- Validate incremental development workflow
- Test build system with various toolchain versions
- _Requirements: All requirements integration_

- [ ] 8.1 Write integration tests for complete build system
  - Test end-to-end build scenarios for all stages
  - Validate all architecture and feature combinations
  - _Requirements: All requirements_

- [ ] 9. Final checkpoint - Ensure all tests pass
Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks are organized by development stage to support incremental approach
- Each stage builds upon previous stage capabilities
- Stage 1 focuses on minimal x86_64 Hello World kernel
- Stages 2-3 add boot protocol support to x86_64
- Stage 4 adds enhanced build features for kernel expansion
- Stages 5-6 add ARM64 and RISC-V architecture support
- Each task references specific requirements for traceability
- Property tests validate universal correctness properties
- Build system grows incrementally with kernel functionality
- QEMU testing integrated at each stage for validation
- Cross-compilation support added when architectures are introduced
