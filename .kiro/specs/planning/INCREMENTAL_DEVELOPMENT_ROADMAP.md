# Incremental Development Roadmap

## Overview

This roadmap defines the incremental development approach for FMI/OS, focusing on building a working kernel incrementally with architecture-first progression and boot mode expansion per architecture.

## Core Principles

1. **Always Working System**: Each stage produces a bootable, testable kernel
2. **Architecture-First**: Complete one architecture before moving to the next
3. **Boot Mode Progression**: Implement boot modes incrementally per architecture
4. **Minimal Start**: Begin with absolute minimum "Hello World" functionality
5. **Incremental Features**: Add one feature at a time while maintaining working system

## Development Stages

### Stage 1: Minimal x86_64 Hello World (PVH ELF Only)
**Goal**: Bootable x86_64 kernel that prints "Hello World" via PVH ELF boot protocol

**Deliverables**:
- Minimal x86_64 bootstrap code for PVH ELF entry
- Basic console output (serial port direct write)
- Simple kernel main that prints message and halts
- Build system supporting x86_64 target
- QEMU testing with PVH ELF boot

**Specs Involved**:
- `bootstrap` - PVH ELF x86_64 bootstrap only
- `build-system` - Basic autoconf/automake for x86_64
- `console` - Direct serial output only
- `testing-framework` - Basic QEMU smoke tests

**Success Criteria**:
- Kernel boots in QEMU with `-kernel` flag
- Prints "Hello World from FMI/OS" to serial console
- Cleanly halts after message

### Stage 2: x86_64 Multiboot2 Support
**Goal**: Add Multiboot2 boot protocol while maintaining PVH ELF support

**Deliverables**:
- Multiboot2 header and entry point
- Multiboot2 information parsing
- Boot protocol detection and unified initialization
- ISO generation with GRUB for Multiboot2 testing

**Specs Involved**:
- `bootstrap` - Add Multiboot2 support to existing PVH code
- `build-system` - Add ISO generation and GRUB configuration

**Success Criteria**:
- Kernel boots via both PVH ELF and Multiboot2
- Same "Hello World" output regardless of boot method
- QEMU testing with both `-kernel` and `-cdrom` methods

### Stage 3: x86_64 UEFI Support
**Goal**: Add UEFI boot protocol support

**Deliverables**:
- UEFI application entry point
- UEFI system table and boot services integration
- UEFI memory map parsing
- UEFI testing in QEMU with OVMF

**Specs Involved**:
- `bootstrap` - Add UEFI support to existing boot protocols
- `build-system` - Add UEFI application generation

**Success Criteria**:
- Kernel boots via PVH ELF, Multiboot2, and UEFI
- Consistent behavior across all boot methods
- UEFI-specific information displayed alongside hello message

### Stage 4: x86_64 Feature Expansion
**Goal**: Add core kernel features while maintaining all boot protocols

**Deliverables**:
- Enhanced console system with multiple devices
- Basic memory management (slab allocator)
- Simple process/thread framework
- KFS namespace basics
- Expanded system information display

**Specs Involved**:
- `console` - Multi-device console support
- `memory` - Basic slab allocator
- `threads-processes` - Minimal thread support
- `kfs-namespace` - Basic namespace structure

**Success Criteria**:
- All x86_64 boot protocols working with enhanced features
- Rich system information display
- Basic kernel services operational

### Stage 5: ARM64 Architecture Port
**Goal**: Port complete x86_64 functionality to ARM64

**Deliverables**:
- ARM64 bootstrap code
- ARM64 U-Boot support (primary)
- ARM64 UEFI support
- ARM64 console drivers
- ARM64 memory management
- Complete feature parity with x86_64

**Specs Involved**:
- All specs updated for ARM64 architecture support
- `bootstrap` - ARM64 bootstrap implementation
- `build-system` - ARM64 cross-compilation support

**Success Criteria**:
- ARM64 kernel boots via U-Boot and UEFI
- Feature parity with x86_64 implementation
- Consistent behavior across architectures

### Stage 6: RISC-V Architecture Port
**Goal**: Port complete functionality to RISC-V

**Deliverables**:
- RISC-V bootstrap code
- RISC-V U-Boot support (primary)
- RISC-V UEFI support
- RISC-V console drivers
- RISC-V memory management
- Complete three-architecture support

**Specs Involved**:
- All specs updated for RISC-V architecture support
- `bootstrap` - RISC-V bootstrap implementation
- `build-system` - RISC-V cross-compilation support

**Success Criteria**:
- RISC-V kernel boots via U-Boot and UEFI
- Feature parity across all three architectures
- Comprehensive multi-architecture testing

## Implementation Guidelines

### Stage Progression Rules

1. **Complete Before Advancing**: Each stage must be 100% complete and tested before moving to next
2. **Maintain Working State**: Never break existing functionality when adding new features
3. **Test Everything**: Each stage requires comprehensive testing of all supported configurations
4. **Document Progress**: Each stage completion documented with working examples

### Architecture Progression Rules

1. **x86_64 First**: Complete all x86_64 boot modes before starting ARM64
2. **ARM64 Second**: Complete all ARM64 boot modes before starting RISC-V
3. **RISC-V Last**: Complete final architecture with full feature parity
4. **No Parallel Architecture Work**: Focus on one architecture at a time

### Boot Mode Progression Rules

1. **PVH ELF First**: Simplest virtualization boot protocol
2. **Multiboot2 Second**: Standard bootloader protocol
3. **UEFI Third**: Modern firmware protocol
4. **U-Boot for ARM64/RISC-V**: Embedded/development board protocol

### Testing Requirements

1. **QEMU Smoke Tests**: Every stage must pass QEMU smoke tests
2. **Regression Testing**: New stages must not break previous functionality
3. **Cross-Architecture Testing**: Later stages test all architectures
4. **Boot Protocol Testing**: All supported boot protocols tested per architecture

## Spec Coordination

### Bootstrap Spec Stages
- Stage 1: PVH ELF x86_64 only
- Stage 2: Add Multiboot2 to x86_64
- Stage 3: Add UEFI to x86_64
- Stage 5: ARM64 U-Boot + UEFI
- Stage 6: RISC-V U-Boot + UEFI

### Build System Spec Stages
- Stage 1: Basic x86_64 autoconf/automake
- Stage 2: Add ISO generation for Multiboot2
- Stage 3: Add UEFI application generation
- Stage 5: Add ARM64 cross-compilation
- Stage 6: Add RISC-V cross-compilation

### Console Spec Stages
- Stage 1: Direct serial output only
- Stage 4: Multi-device console system
- Stage 5: ARM64 console drivers
- Stage 6: RISC-V console drivers

### Memory Spec Stages
- Stage 4: Basic slab allocator
- Stage 5: ARM64 memory management
- Stage 6: RISC-V memory management

### Other Specs
- Stages 4-6: Incremental implementation as features are needed
- Always maintain working system during feature addition

## Success Metrics

### Stage 1 Success
- [ ] Kernel boots in QEMU via PVH ELF
- [ ] Prints "Hello World from FMI/OS"
- [ ] Shows basic system information
- [ ] Cleanly halts

### Stage 2 Success
- [ ] All Stage 1 functionality maintained
- [ ] Kernel boots via Multiboot2 (GRUB/ISO)
- [ ] Boot protocol detection working
- [ ] Consistent output regardless of boot method

### Stage 3 Success
- [ ] All previous functionality maintained
- [ ] Kernel boots via UEFI (OVMF in QEMU)
- [ ] UEFI-specific information displayed
- [ ] Three boot protocols working on x86_64

### Stage 4 Success
- [ ] All boot protocols maintained
- [ ] Enhanced console system operational
- [ ] Basic memory management working
- [ ] Simple kernel services running
- [ ] Rich system information display

### Stage 5 Success
- [ ] Complete ARM64 port functional
- [ ] ARM64 U-Boot and UEFI boot working
- [ ] Feature parity with x86_64
- [ ] Cross-architecture testing passing

### Stage 6 Success
- [ ] Complete RISC-V port functional
- [ ] RISC-V U-Boot and UEFI boot working
- [ ] Three-architecture feature parity
- [ ] Comprehensive multi-architecture testing

This incremental approach ensures we always have a working, testable system while building complexity gradually and maintaining quality throughout development.
