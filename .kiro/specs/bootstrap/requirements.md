# Requirements Document

## Introduction

The Bootstrap subsystem handles the critical early kernel initialization phase, including architecture-specific boot protocol support, 32-bit to 64-bit mode transitions, and hardware detection. This subsystem ensures the kernel can boot reliably across multiple architectures and bootloader configurations.

## Glossary

- **Bootstrap**: The early kernel initialization code that sets up the execution environment
- **Multiboot_Protocol**: Standard boot protocol used by GRUB and other bootloaders
- **PVH_ELF**: Paravirtualized Hardware ELF boot protocol for virtualization environments
- **Mode_Transition**: The process of switching from 32-bit to 64-bit execution mode
- **Boot_Protocol_Detection**: Mechanism to identify which bootloader protocol was used
- **Memory_Detection**: Process of discovering available system memory from bootloader
- **Command_Line_Parsing**: Extraction and processing of kernel boot parameters

## Requirements

### Requirement 1: Multi-Architecture Boot Protocol Support

**User Story:** As a system administrator, I want the kernel to support multiple boot protocols across different architectures, so that it can be deployed on various hardware platforms and virtualization environments.

#### Acceptance Criteria

1. THE Bootstrap SHALL support Multiboot v2 protocol for x86_64 architecture
2. THE Bootstrap SHALL support PVH ELF boot protocol for virtualization environments
3. THE Bootstrap SHALL support U-Boot protocol for ARM64 and RISC-V architectures
4. THE Bootstrap SHALL support UEFI boot protocol for modern hardware platforms
5. THE Bootstrap SHALL detect which boot protocol was used during startup
6. THE Bootstrap SHALL expose detected boot protocol through `//kern/boot/protocol` in the KFS
7. THE Build_System SHALL enforce architecture-specific boot protocol restrictions
8. WHEN building for ARM64 or RISC-V, THE Build_System SHALL require U-Boot or UEFI protocol support

### Requirement 2: 32-bit to 64-bit Mode Transition

**User Story:** As a kernel developer, I want reliable 32-bit to 64-bit mode transitions, so that the kernel can start from bootloaders that may boot into 32-bit mode and establish proper 64-bit execution environment.

#### Acceptance Criteria

1. THE Bootstrap SHALL perform CPU feature detection using CPUID instructions
2. THE Bootstrap SHALL verify long mode and PAE support before transition
3. THE Bootstrap SHALL detect current CPU mode (32-bit or 64-bit) at entry
4. WHEN booted in 32-bit mode, THE Bootstrap SHALL create minimal page table structure (PML4→PDP→PD with 2MB pages)
5. WHEN booted in 32-bit mode, THE Bootstrap SHALL set up temporary GDT with 64-bit code and data segments
6. WHEN booted in 32-bit mode, THE Bootstrap SHALL enable PAE (Physical Address Extension) in CR4 register
7. WHEN booted in 32-bit mode, THE Bootstrap SHALL enable long mode via LME bit in EFER MSR
8. WHEN booted in 32-bit mode, THE Bootstrap SHALL activate paging via PG bit in CR0 register
9. WHEN booted in 32-bit mode, THE Bootstrap SHALL perform far jump to 64-bit code segment to complete transition
10. THE Bootstrap SHALL validate successful 64-bit mode operation and output debug markers to serial console

### Requirement 3: Bootloader Memory Detection

**User Story:** As a system administrator, I want automatic memory detection from bootloaders, so that the kernel adapts to different hardware configurations without manual intervention.

#### Acceptance Criteria

1. THE Memory_Detection SHALL support UEFI memory maps
2. THE Memory_Detection SHALL support Multiboot v2 memory maps
3. THE Memory_Detection SHALL support U-Boot memory detection for ARM64/RISC-V
4. THE Memory_Detection SHALL parse memory regions (available, reserved, bad, ACPI)
5. THE Memory_Detection SHALL provide unified memory_map_t interface
6. THE Memory_Detection SHALL calculate total usable memory and highest address
7. THE Memory_Detection SHALL handle memory holes and reserved regions
8. THE Memory_Detection SHALL support up to 128 memory regions
9. THE Memory_Detection SHALL provide thread-safe access using spinlocks
10. THE Memory_Detection SHALL initialize before page allocator setup

### Requirement 4: Boot Command Line Processing

**User Story:** As a system administrator, I want kernel boot parameters to be captured and parsed, so that I can configure kernel behavior including console selection and debugging options.

#### Acceptance Criteria

1. THE Command_Line_Parser SHALL capture parameters from UEFI bootloaders
2. THE Command_Line_Parser SHALL capture parameters from Multiboot v2 bootloaders
3. THE Command_Line_Parser SHALL capture parameters from U-Boot bootloaders
4. THE Command_Line_Parser SHALL copy command line data into kernel memory
5. THE Command_Line_Parser SHALL provide cmdline_get_param() function
6. THE Command_Line_Parser SHALL support console parameter parsing
7. THE Command_Line_Parser SHALL expose complete command line via `//kern/cmdline`
8. THE Command_Line_Parser SHALL handle parameters up to 4096 characters
9. THE Command_Line_Parser SHALL parse key=value pairs and boolean flags
10. THE Command_Line_Parser SHALL provide thread-safe parameter access

### Requirement 5: Architecture-Specific Bootstrap Implementation

**User Story:** As a kernel developer, I want architecture-specific bootstrap code that handles platform differences, so that the kernel can boot optimally on each supported architecture.

#### Acceptance Criteria

1. THE Bootstrap SHALL be located in `arch/*/bootstrap.S` for each architecture
2. THE x86_64 Bootstrap SHALL handle Multiboot v2 and PVH ELF protocols
3. THE x86_64 Bootstrap SHALL support both 32-bit and 64-bit entry points for these protocols
4. THE ARM64 Bootstrap SHALL handle U-Boot and UEFI protocols
5. THE RISC-V Bootstrap SHALL handle U-Boot and UEFI protocols
6. THE Bootstrap SHALL call generic kernel main entry point after initialization
7. THE Bootstrap SHALL set up architecture-specific stack and registers
8. THE Bootstrap SHALL enable architecture-specific CPU features
9. THE Bootstrap SHALL handle architecture-specific interrupt setup
10. THE Bootstrap SHALL provide debug output capabilities and validate hardware compatibility before proceeding

### Requirement 6: Minimalized Kernel Integration

**User Story:** As a kernel developer, I want the bootstrap to integrate directly with a minimalized main kernel, so that I can verify bootstrap functionality using the actual kernel infrastructure without complex subsystem dependencies.

#### Acceptance Criteria

1. THE Bootstrap SHALL integrate with minimalized main kernel for x86_64, ARM64, and RISC-V architectures
2. THE Minimalized_Kernel SHALL use simplified console output via arch_putchar() function
3. THE Minimalized_Kernel SHALL display detected CPU type and architecture information
4. THE Minimalized_Kernel SHALL display total memory detected from bootloader
5. THE Minimalized_Kernel SHALL display boot protocol used (Multiboot v2, PVH, U-Boot, UEFI)
6. THE Minimalized_Kernel SHALL display command line parameters received from bootloader
7. THE Minimalized_Kernel SHALL work with all architecture-specific bootstrap code
8. THE Minimalized_Kernel SHALL output information to serial console for automated capture
9. THE Minimalized_Kernel SHALL perform clean shutdown after displaying information
10. THE QEMU_Tests SHALL validate expected output patterns for each architecture