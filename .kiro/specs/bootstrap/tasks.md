# Implementation Plan: Bootstrap Subsystem

## Overview

This implementation plan follows the incremental development approach, building bootstrap functionality stage by stage. Each stage produces a working, testable kernel that boots and demonstrates implemented features.

**INCREMENTAL APPROACH**:
- Stage 1: Minimal x86_64 Multiboot2 bootstrap with GRUB (Hello World)
- Stage 2: Add UEFI support to x86_64
- Stage 3: ARM64 bootstrap (U-Boot + UEFI)
- Stage 4: RISC-V bootstrap (U-Boot + UEFI)

## Tasks

- [x] 1. Remove Multiboot v1 Support from Entire Code Tree
**Priority: CRITICAL BLOCKER - Must be removed before any development to prevent interference**
**Dependencies: None**

Remove all Multiboot v1 support from the entire codebase, including build system, documentation, and assembly code. This must be done first as multiboot1 code will interfere with minimal kernel development.

**Phase 1: Build System Cleanup** ✅ COMPLETED
- Remove --enable-multiboot1 option from configure.ac
- Remove ENABLE_MULTIBOOT1 preprocessor definitions from build flags
- Remove multiboot1-related conditional compilation from Makefiles
- Remove multiboot1 entries from GRUB configuration generation scripts
- Update configure script help text to remove multiboot1 references
- Ensure Multiboot v2 remains available for x86_64 targets with GRUB bootloader
- Test configure script with various architecture and protocol combinations

**Phase 2: Assembly Code Cleanup** ✅ COMPLETED
- Remove Multiboot v1 header generation and magic numbers from bootstrap.S
- Remove Multiboot v1 specific entry point (_start32) while preserving generic 32-bit entry capability
- Remove multiboot1_info storage and related data structures
- Remove ENABLE_MULTIBOOT1 conditional compilation blocks
- Clean up linker script sections that were specific to multiboot1
- Update debug output to reflect removal of Multiboot v1 specific support

**Phase 3: Documentation Cleanup** ✅ COMPLETED
- Update build system documentation to reflect multiboot1 removal
- Remove multiboot1 references from all documentation files
- Update architecture documentation to focus on Multiboot v2 with GRUB bootloader
- _Requirements: 1.7, 1.8, 1.1, 1.5, 1.6_

- [x] 1.1 Verify multiboot1 removal completeness
  - Scan entire codebase for remaining multiboot1 references
  - Test build system without multiboot1 dependencies
  - _Requirements: 1.1, 1.5, 1.6_

- [x] 2. Simplify and Minimize Existing Kernel
**Priority: CRITICAL FOUNDATION - Required for all subsequent work**
**Dependencies: Task 1 (multiboot1 removal)**

Simplify the existing kernel to create a minimal bootstrap validation environment, removing complex subsystems and standardizing the architecture-independent interface.

**Phase 1: Simplify Console Implementation**
- Simplify existing console code to use direct serial port output only
- Remove complex console multiplexing, buffering, and driver management
- Ensure kprintf() works immediately after bootstrap without subsystem initialization
- Keep basic format string support (strings, integers, hex values) that already exists
- Remove dependencies on high-level console subsystem initialization

**Phase 2: Implement Boot-Mode Specific Main Functions**
- Create mb2_main() for Multiboot v2 specific initialization
- Create uefi_main() for UEFI specific initialization
- Create uboot_main() for U-Boot specific initialization
- Each boot-mode main handles protocol-specific setup before calling kmain()
- Implement standardized boot_info_t population in each boot-mode main
- Ensure clean separation between protocol-specific and generic initialization

**Phase 3: Minimize Kernel Main**
- Simplify existing kmain() to receive standardized boot_info_t
- Remove high-level subsystem initialization (memory manager, scheduler, etc.)
- Add basic system information display using simplified kprintf()
- Add memory information display from boot_info_t
- Add CPU information display from boot_info_t
- Add boot protocol identification display
- Add command line parameter display
- Implement clean shutdown mechanism after information display
- _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 6.10_

- [x] 2.1 Write QEMU smoke tests for all architectures
  - **Property 6: QEMU Smoke Test Validation**
  - **Validates: Requirements 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 6.10**
  - **Status: IMPLEMENTED** - Tests fail as expected due to missing kernel image
  - **PBT Status: FAIL (Expected)** - Kernel image not found, tests correctly detect missing dependency

- [ ]* 2.2 Write automated test validation scripts

- [x] 3. Implement x86_64 Multiboot2 Bootstrap with GRUB
**Priority: HIGH - Core x86_64 boot protocol correctness**
**Dependencies: Task 2 (simplified kernel infrastructure)**

Implement x86_64 bootstrap code for Multiboot2 protocol using GRUB bootloader, providing reliable 32-bit to 64-bit mode transition.

**Phase 1: Multiboot2 Bootstrap Assembly**
- Create bootstrap.S that handles Multiboot2 protocol with proper header
- Implement Multiboot2 header with required tags for GRUB compatibility
- Handle 32-bit entry point from GRUB bootloader
- Implement 32-bit to 64-bit mode transition for Multiboot2
- Jump to mb2_main() function with Multiboot2 information structure
- Remove PVH ELF support and focus solely on Multiboot2

**Phase 2: Mode Transition Implementation**
- Implement CPU feature detection (CPUID, long mode, PAE support)
- Create minimal page table structure (PML4→PDP→PD with 2MB pages)
- Set up temporary GDT with 64-bit code and data segments
- Enable PAE, long mode, and paging in proper sequence
- Perform far jump to 64-bit code segment
- Validate successful 64-bit mode operation

**Phase 3: GRUB Integration and Testing**
- Create ISO image with GRUB bootloader for QEMU testing
- Test Multiboot2 boot using GRUB in QEMU environment
- Validate Multiboot2 information structure parsing
- Ensure proper memory map and command line parameter extraction
- _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10_



- [x] 4. Implement and Validate UEFI Boot Protocol Support
**Priority: HIGH - Modern firmware boot protocol correctness**
**Dependencies: Task 3 (Multiboot v2 implementation)**

Implement UEFI boot protocol support for modern firmware environments, tested using the simplified kernel.

**Phase 1: UEFI Protocol Implementation**
- Implement UEFI boot protocol detection and initialization
- Add UEFI memory map extraction and parsing
- Implement UEFI command line parameter capture
- Add UEFI system table and boot services integration
- Handle UEFI-specific memory layout and addressing

**Phase 2: UEFI Boot Information Processing**
- Parse UEFI memory maps to identify available, reserved, bad, and ACPI regions
- Extract UEFI-provided system information (CPU, memory, devices)
- Handle UEFI graphics and console initialization
- Implement UEFI runtime services integration

**Phase 3: Testing with Simplified Kernel**
- Test UEFI boot using simplified kernel in QEMU with OVMF firmware
- Validate UEFI-specific boot information exposure
- Test UEFI memory map parsing and validation
- Validate that standardized boot_info_t structure is properly populated from UEFI data
- _Requirements: 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3_

- [ ]* 4.1 Write unit tests for UEFI support
  - Test UEFI memory map parsing
  - Test UEFI command line parameter extraction
  - _Requirements: 3.1, 3.2, 4.1, 4.2_

- [ ]* 4.2 Write QEMU UEFI integration tests
  - Test UEFI boot in QEMU with OVMF firmware
  - Validate UEFI-specific boot information
  - _Requirements: 3.3, 3.4, 4.3_

- [ ] 5. Implement ARM64 Bootstrap Code
**Priority: HIGH - ARM64 architecture support**
**Dependencies: Task 4 (UEFI implementation for shared code)**

Implement ARM64 bootstrap code with U-Boot and UEFI protocol support, tested using the simplified kernel.

**Phase 1: ARM64 Bootstrap Implementation**
- Implement ARM64 bootstrap code in `arch/arm64/bootstrap.S`
- Handle U-Boot and UEFI protocols for ARM64
- Set up ARM64-specific stack and registers
- Enable ARM64-specific CPU features
- Handle ARM64-specific interrupt setup
- Provide debug output capabilities for ARM64

**Phase 2: ARM64 Boot Protocol Integration**
- Implement ARM64 memory detection for U-Boot and UEFI bootloaders
- Add ARM64 command line parameter capture
- Validate ARM64 hardware compatibility before proceeding
- Populate standardized boot_info_t structure with ARM64-specific data

**Phase 3: Testing with Simplified Kernel**
- Test ARM64 bootstrap using simplified kernel in QEMU
- Validate ARM64-specific system information discovery
- Test both U-Boot and UEFI boot paths for ARM64
- Ensure consistent behavior with x86_64 architecture
- _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 5.10_

- [ ]* 5.1 Write property test for ARM64 bootstrap correctness
  - **Property 6: Architecture-Specific Bootstrap Correctness**
  - **Validates: Requirements 5.5, 5.6**

- [ ]* 5.2 Write unit tests for ARM64 CPU feature detection
  - **Property 5: CPU Feature Detection Reliability**
  - **Validates: Requirements 5.1, 5.2**

- [ ] 6. Implement RISC-V Bootstrap Code
**Priority: HIGH - RISC-V architecture support**
**Dependencies: Task 5 (ARM64 implementation for shared patterns)**

Implement RISC-V bootstrap code with U-Boot and UEFI protocol support, tested using the simplified kernel.

**Phase 1: RISC-V Bootstrap Implementation**
- Implement RISC-V bootstrap code in `arch/riscv64/bootstrap.S`
- Handle U-Boot and UEFI protocols for RISC-V
- Set up RISC-V-specific stack and registers
- Enable RISC-V-specific CPU features
- Handle RISC-V-specific interrupt setup
- Provide debug output capabilities for RISC-V

**Phase 2: RISC-V Boot Protocol Integration**
- Implement RISC-V memory detection for U-Boot and UEFI bootloaders
- Add RISC-V command line parameter capture
- Validate RISC-V hardware compatibility before proceeding
- Populate standardized boot_info_t structure with RISC-V-specific data

**Phase 3: Testing with Simplified Kernel**
- Test RISC-V bootstrap using simplified kernel in QEMU
- Validate RISC-V-specific system information discovery
- Test both U-Boot and UEFI boot paths for RISC-V
- Ensure consistent behavior across all architectures (x86_64, ARM64, RISC-V)
- _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7, 5.8, 5.9, 5.10_

- [ ]* 6.1 Write property test for RISC-V bootstrap correctness
  - **Property 6: Architecture-Specific Bootstrap Correctness**
  - **Validates: Requirements 5.5, 5.6**

- [ ]* 6.2 Write unit tests for RISC-V CPU feature detection
  - **Property 5: CPU Feature Detection Reliability**
  - **Validates: Requirements 5.1, 5.2**

- [ ] 7. Implement Unified Memory Detection and Command Line Systems
**Priority: MEDIUM - Supporting infrastructure for all architectures**
**Dependencies: Task 6 (RISC-V implementation)**

Implement bootloader-agnostic memory detection and command line parameter systems that work across all architectures, tested using the simplified kernel.

**Phase 1: Unified Memory Detection System**
- Create unified memory_map_t interface for all bootloader types (already defined in standardized interface)
- Implement memory detection for UEFI bootloaders (x86_64, ARM64, RISC-V)
- Implement memory detection for Multiboot v2 bootloaders (x86_64)
- Implement memory detection for U-Boot bootloaders (ARM64, RISC-V)
- Parse bootloader memory maps to identify available, reserved, bad, and ACPI regions
- Calculate total usable memory and highest memory address
- Implement memory_region_is_usable() function for region validation
- Handle different memory region types (MEMORY_AVAILABLE, MEMORY_RESERVED, MEMORY_BAD, MEMORY_HOLE)
- Support up to 128 memory regions for complex memory layouts

**Phase 2: Unified Command Line Parameter System**
- Create unified cmdline_info_t interface (already defined in standardized interface)
- Capture command line parameters from UEFI bootloaders (all architectures)
- Capture command line parameters from Multiboot v2 bootloaders (x86_64)
- Capture command line parameters from U-Boot bootloaders (ARM64, RISC-V)
- Copy command line data into kernel memory during bootstrap
- Implement cmdline_get_param() function for parameter retrieval by name
- Parse console parameters (e.g., console=ttyS0,115200)
- Handle command line parameters up to 4096 characters
- Parse key=value parameter pairs and boolean flags

**Phase 3: Testing with Simplified Kernel**
- Test memory detection across all architectures and bootloaders using simplified kernel
- Test command line parameter parsing across all architectures using simplified kernel
- Validate that standardized structures are properly populated
- Ensure consistent behavior across all supported combinations
- _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 3.8, 3.9, 3.10, 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 4.10_

- [ ]* 7.1 Write property test for memory map consistency
  - **Property 3: Memory Map Consistency**
  - **Validates: Requirements 3.5, 3.6**

- [ ]* 7.2 Write property test for command line parameter preservation
  - **Property 4: Command Line Parameter Preservation**
  - **Validates: Requirements 4.4, 4.5**

- [ ]* 7.3 Write unit tests for memory detection and command line parsing
  - Test memory detection for all bootloader types
  - Test command line parameter parsing for all bootloader types
  - Test key=value pairs and boolean flag parsing
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3_

- [ ] 8. Implement Boot Information Exposure System
**Priority: MEDIUM - System introspection support**
**Dependencies: Task 7 (unified memory and command line systems)**

Implement boot information exposure through the KFS interface, providing unified access to boot information across all architectures.

**Phase 1: KFS Interface Implementation**
- Expose boot protocol information via `//kern/boot/protocol`
- Expose multiboot v2 information via `//kern/boot/multiboot_info` (x86_64 only)
- Expose UEFI information via `//kern/boot/uefi_info` (all architectures)
- Expose U-Boot information via `//kern/boot/uboot_info` (ARM64, RISC-V)
- Expose memory map via `//kern/boot/memory_map` (all architectures)
- Expose command line via `//kern/boot/cmdline` (all architectures)
- Expose CPU features via `//kern/boot/cpu_features` (all architectures)

**Phase 2: Information Formatting and Access**
- Provide read-only access to boot information
- Format boot information for human readability
- Update boot information atomically
- Maintain boot information throughout kernel lifetime
- Provide thread-safe access with spinlock protection

**Phase 3: Testing with Simplified Kernel**
- Test boot information exposure across all architectures using simplified kernel
- Validate information formatting and consistency
- Test atomic updates and thread-safe access
- Ensure boot information persists throughout kernel lifetime
- _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 6.10_

- [ ]* 8.1 Write property test for boot information exposure consistency
  - **Property 7: Boot Information Exposure Consistency**
  - **Validates: Requirements 6.1, 6.9**

- [ ]* 8.2 Write unit tests for KFS boot information interface
  - Test boot information formatting and access
  - Test atomic updates and consistency
  - _Requirements: 6.7, 6.8, 6.9, 6.10_

- [ ] 9. Final Integration Testing and Validation
**Priority: HIGH - Comprehensive system validation**
**Dependencies: Task 8 (boot information exposure)**

Perform comprehensive integration testing across all architectures and boot protocols using the simplified kernel.

**Phase 1: Cross-Architecture Validation**
- Test all boot protocols on x86_64 (Multiboot v2, UEFI) using simplified kernel
- Test all boot protocols on ARM64 (U-Boot, UEFI) using simplified kernel
- Test all boot protocols on RISC-V (U-Boot, UEFI) using simplified kernel
- Validate consistent behavior across all architecture/protocol combinations
- Ensure standardized boot_info_t structure is properly populated in all cases

**Phase 2: Regression Testing**
- Run comprehensive QEMU smoke tests across all supported configurations
- Validate that multiboot1 removal doesn't affect other protocols
- Test 32-bit to 64-bit mode transitions work correctly
- Verify memory detection and command line parsing work across all combinations
- Validate boot information exposure works consistently

**Phase 3: Performance and Reliability Testing**
- Test bootstrap performance across all architectures
- Validate error handling and recovery mechanisms
- Test edge cases and boundary conditions
- Ensure clean shutdown and resource cleanup
- _Requirements: All bootstrap requirements across all architectures_

- [ ]* 9.1 Write comprehensive integration tests
  - Test all architecture/protocol combinations
  - Validate cross-architecture consistency
  - _Requirements: All bootstrap requirements_

- [ ] 10. Checkpoint - Ensure all tests pass
Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties
- Unit tests validate specific examples and edge cases
- **CRITICAL: All bootstrap testing MUST be performed via QEMU smoke testing using the simplified kernel**
- **NO unit testing is applicable for bootstrap assembly code due to hardware dependencies**
- The bootstrap subsystem is critical for system stability and must be thoroughly tested
- Architecture-specific code requires testing on each target platform via QEMU
- Boot protocol support enables deployment flexibility across different environments
- The simplified kernel must dynamically discover and validate system information against QEMU parameters
- **PRIORITY ORDER**: Tasks are ordered to prioritize multiboot1 removal first (to prevent development interference), followed by kernel simplification, then systematic testing of each boot protocol using the simplified kernel
- **STANDARDIZED INTERFACE**: All architectures must populate standardized boot_info_t, memory_map_t, cmdline_info_t, and cpu_info_t structures before calling kmain()
- **COMPREHENSIVE TESTING**: Each implementation phase is validated using the simplified kernel to ensure correctness before proceeding
