# Design Document

## Overview

The Bootstrap subsystem provides the critical early kernel initialization infrastructure that bridges the gap between bootloader handoff and kernel main execution. It handles multi-architecture boot protocol support, CPU mode transitions, hardware detection, and establishes the foundation for all subsequent kernel operations.

**SIMPLIFIED ARCHITECTURE**: Instead of a separate test kernel, the bootstrap integrates directly with a minimalized main kernel that uses simplified console output and avoids complex subsystem dependencies during early boot.

## Architecture

### Boot Flow Architecture

```
Bootloader → Bootstrap Entry → Mode Transition → Boot-Mode Main → Kernel Main
    ↓              ↓               ↓                ↓              ↓
  Protocol      CPU Setup      64-bit Mode      mb2_main()      kmain()
  Detection     Stack Init     Page Tables      pvh_main()
          GDT Setup      Feature Check    uefi_main()
```

### Simplified Console Architecture

```
kprintf() → arch_putchar() → Serial Port
  ↓            ↓              ↓
Format      Direct I/O    Hardware
String      No Buffering  Output
```

### Boot-Mode Specific Entry Points

Each boot protocol has its own main function to handle protocol-specific setup:

- **mb2_main()**: Handles Multiboot v2 specific initialization
- **pvh_main()**: Handles PVH ELF specific initialization
- **uefi_main()**: Handles UEFI specific initialization
- **uboot_main()**: Handles U-Boot specific initialization

All boot-mode mains eventually call the unified `kmain()` after completing their setup.

### Multi-Protocol Support

The bootstrap system supports multiple boot protocols through architecture-specific entry points that assume 32-bit mode entry:

- **x86_64**: Multiboot v2 (assumes 32-bit entry), PVH ELF (32-bit entry), UEFI (64-bit entry)
- **ARM64**: U-Boot, UEFI (64-bit native)
- **RISC-V**: U-Boot, UEFI (64-bit native)

### Architecture Separation

```
arch/x86_64/bootstrap.S    # x86_64-specific bootstrap with mode transition
arch/x86_64/mb2_main.c     # Multiboot v2 specific setup
arch/x86_64/pvh_main.c     # PVH ELF specific setup
arch/x86_64/uefi_main.c    # UEFI specific setup
arch/arm64/bootstrap.S     # ARM64-specific bootstrap (native 64-bit)
arch/arm64/uboot_main.c    # U-Boot specific setup
arch/arm64/uefi_main.c     # UEFI specific setup
arch/riscv64/bootstrap.S   # RISC-V-specific bootstrap (native 64-bit)
arch/riscv64/uboot_main.c  # U-Boot specific setup
arch/riscv64/uefi_main.c   # UEFI specific setup
```

## Components and Interfaces

### Simplified Console Interface

```c
// Architecture-provided simple console output
void arch_putchar(char c);

// Simplified kprintf that uses arch_putchar
int kprintf(const char *format, ...);

// No complex console subsystem dependencies during bootstrap
// No buffering, no multiplexing, direct hardware output
```

### Boot-Mode Specific Main Functions

```c
// Boot-mode specific entry points called from bootstrap.S
void mb2_main(uint32_t magic, void *info);    // Multiboot v2
void pvh_main(void *start_info);              // PVH ELF
void uefi_main(void *system_table);           // UEFI
void uboot_main(void *fdt);                   // U-Boot

// All boot-mode mains eventually call this
void kmain(boot_info_t *boot_info);
```

### Boot Protocol Detection

```c
enum boot_protocol {
  BOOT_PROTOCOL_MULTIBOOT_V2,
  BOOT_PROTOCOL_PVH_ELF,
  BOOT_PROTOCOL_UBOOT,
  BOOT_PROTOCOL_UEFI
};

struct boot_info_s {
  enum boot_protocol protocol;
  void *bootloader_data;
  char *command_line;
  struct memory_map_s *memory_map;
  uint32_t cpu_features;
} boot_info_t;
```

### Memory Detection Interface

```c
struct memory_region_s {
  uint64_t base_addr;
  uint64_t length;
  enum memory_type type;
} memory_region_t;

struct memory_map_s {
  memory_region_t regions[128];
  size_t region_count;
  uint64_t total_memory;
  uint64_t usable_memory;
} memory_map_t;

// Unified interface for all bootloader types
int memory_detect_from_bootloader(boot_info_t *boot_info, memory_map_t *map);
bool memory_region_is_usable(const memory_region_t *region);
```

### Command Line Processing

```c
struct cmdline_parser_s {
  char *cmdline_buffer;
  size_t buffer_size;
  spinlock_t parser_lock;
} cmdline_parser_t;

// Command line parameter access
const char *cmdline_get_param(const char *param_name);
bool cmdline_has_flag(const char *flag_name);
int cmdline_get_int_param(const char *param_name, int default_value);
```

## Data Models

### Boot Information Structure

The bootstrap subsystem maintains comprehensive boot information accessible through the KFS:

```c
struct bootstrap_state_s {
  boot_info_t boot_info;
  memory_map_t memory_map;
  cmdline_parser_t cmdline_parser;
  uint32_t cpu_features;
  bool mode_transition_complete;
  spinlock_t state_lock;
} bootstrap_state_t;
```

### CPU Feature Detection

```c
struct cpu_features_s {
  bool long_mode_supported;
  bool pae_supported;
  bool pse_supported;
  bool nx_supported;
  bool smep_supported;
  bool smap_supported;
  uint32_t max_physical_addr_bits;
  uint32_t max_linear_addr_bits;
} cpu_features_t;
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Boot Protocol Detection Correctness
*For any* valid bootloader handoff, the bootstrap system should correctly identify the boot protocol and extract all required information without corruption.
**Validates: Requirements 1.6, 1.7**

### Property 2: Mode Transition Atomicity
*For any* x86_64 system with long mode support, when booted in 32-bit mode, the 32-bit to 64-bit mode transition should complete atomically without intermediate invalid states.
**Validates: Requirements 2.9, 2.10**

### Property 3: Memory Map Consistency
*For any* bootloader-provided memory map, the unified memory_map_t interface should preserve all region information and maintain consistency across different bootloader formats.
**Validates: Requirements 3.5, 3.6**

### Property 4: Command Line Parameter Preservation
*For any* valid command line string, all parameters should be preserved and accessible through the cmdline_get_param() interface without data loss.
**Validates: Requirements 4.4, 4.5**

### Property 5: CPU Feature Detection Reliability
*For any* supported CPU, feature detection should accurately report capabilities and never report unsupported features as available.
**Validates: Requirements 2.1, 2.2**

### Property 6: QEMU Smoke Test Validation
*For any* supported architecture, the minimal test kernel should successfully boot, display environment information, and shutdown cleanly when executed in QEMU.
**Validates: Requirements 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8, 6.9, 6.10**

### Property 7: Test Kernel Architecture Independence
*For any* architecture-specific bootstrap code, the minimal test kernel should work without modification and display consistent information format across all platforms.
**Validates: Requirements 6.7, 6.8**

## Error Handling

### Boot Protocol Errors
- Invalid magic numbers → Clear error message and halt
- Unsupported protocol version → Fallback to supported version or halt
- Corrupted bootloader data → Validation and graceful degradation

### Mode Transition Errors
- Unsupported CPU features → Clear error message identifying missing features
- Page table setup failure → Debug output and halt with error code
- GDT setup failure → Immediate halt with diagnostic information
- Mode detection failure → Clear indication of current vs required mode

### Memory Detection Errors
- Invalid memory map → Use conservative defaults and log warnings
- Memory region overlap → Resolve conflicts using bootloader priority
- Insufficient memory → Halt with minimum memory requirement message

### Command Line Errors
- Oversized command line → Truncate with warning
- Invalid parameter format → Skip invalid parameters and continue
- Buffer allocation failure → Use static fallback buffer

## Testing Strategy

### QEMU Smoke Testing (Mandatory Testing Method)

**CRITICAL REQUIREMENT: All bootstrap code testing MUST be performed exclusively through QEMU smoke testing using a custom minimal test kernel. Traditional unit testing is not applicable for bootstrap code.**

See `bootstrap-testing-methodology.md` for complete testing requirements and methodology.

The bootstrap subsystem cannot be effectively unit tested due to its hardware-dependent nature and early execution context. Instead, comprehensive QEMU smoke testing provides validation across all supported architectures using a purpose-built minimal test kernel.

#### Minimal Test Kernel Requirements

The custom test kernel MUST:
- Use only serial console output for automated capture
- Dynamically discover system information through bootstrap-provided data
- Validate discovered information against QEMU parameters
- Display comprehensive system information including memory, CPU, architecture, and boot protocol
- Perform clean shutdown after validation
- Work consistently across all supported architectures

#### Environment Information Discovery

The test kernel MUST dynamically discover and display:
- **Architecture**: Target architecture (x86_64, arm64, riscv64)
- **CPU Information**: Type, model, core count, features
- **Memory Information**: Total memory, usable regions, memory map
- **Boot Protocol**: Detected protocol and bootloader data
- **Command Line**: Parameters passed from bootloader
- **Hardware**: Any additional hardware information from bootloader

#### Environment Validation

The test kernel MUST validate that discovered information matches QEMU parameters:
- Memory size matches `-m` parameter
- CPU core count matches `-smp` parameter
- Architecture matches expected target
- Command line matches `-append` parameter

#### Minimal Test Kernel Architecture

```c
// tests/bootstrap/test_kernel.c - Generic test kernel for all architectures
void test_kernel_main(boot_info_t *boot_info) {
  // Initialize minimal serial console output only
  serial_console_init();

  // Display comprehensive system information discovered by bootstrap
  kprintf("=== Bootstrap Test Kernel ===\n");
  kprintf("Architecture: %s\n", get_architecture_name());
  kprintf("CPU Type: %s\n", get_cpu_type_string());
  kprintf("CPU Cores: %d\n", get_cpu_core_count());
  kprintf("CPU Features: %s\n", get_cpu_features_string());

  // Display memory information from bootstrap
  kprintf("Total Memory: %lu MB\n", boot_info->memory_map.total_memory / (1024*1024));
  kprintf("Usable Memory: %lu MB\n", boot_info->memory_map.usable_memory / (1024*1024));
  kprintf("Memory Regions: %zu\n", boot_info->memory_map.region_count);

  // Display boot protocol information
  kprintf("Boot Protocol: %s\n", get_boot_protocol_name(boot_info->protocol));

  // Display command line parameters
  if (boot_info->command_line) {
    kprintf("Command Line: %s\n", boot_info->command_line);
  }

  // Validate environment against QEMU parameters
  kprintf("=== Environment Validation ===\n");
  validate_memory_against_qemu_params(boot_info);
  validate_cpu_count_against_qemu_params(boot_info);
  validate_architecture_against_expected();
  validate_command_line_against_qemu_params(boot_info);

  // Display success and shutdown
  kprintf("=== Bootstrap Test PASSED ===\n");
  kprintf("Shutting down...\n");

  // Perform clean shutdown
  test_kernel_shutdown();
}
```

#### QEMU Test Configuration

```bash
# x86_64 QEMU smoke test with environment validation
qemu-system-x86_64 \
  -kernel tests/bootstrap/test-kernel-x86_64 \
  -serial mon:stdio \
  -nographic \
  -m 512M \
  -smp 2 \
  -append "console=ttyS0 test=bootstrap arch=x86_64"

# ARM64 QEMU smoke test with environment validation
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a57 \
  -kernel tests/bootstrap/test-kernel-arm64 \
  -serial mon:stdio \
  -nographic \
  -m 512M \
  -smp 2 \
  -append "console=ttyS0 test=bootstrap arch=arm64"

# RISC-V QEMU smoke test with environment validation
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

#### Automated Test Validation

```bash
# Expected output patterns for validation (updated for comprehensive testing)
EXPECTED_PATTERNS=(
  "=== Bootstrap Test Kernel ==="
  "Architecture: (x86_64|arm64|riscv64)"
  "CPU Type: .*"
  "CPU Cores: [0-9]+"
  "CPU Features: .*"
  "Total Memory: [0-9]+ MB"
  "Usable Memory: [0-9]+ MB"
  "Memory Regions: [0-9]+"
  "Boot Protocol: (Multiboot v2|PVH|U-Boot|UEFI)"
  "Command Line: .*"
  "=== Environment Validation ==="
  "Memory: PASS.*"
  "CPU Cores: PASS.*"
  "Architecture: PASS.*"
  "Command Line: PASS.*"
  "=== Bootstrap Test PASSED ==="
  "Shutting down..."
)

# Enhanced validation script with environment correlation
validate_bootstrap_test_output() {
  local output_file="$1"
  local arch="$2"
  local memory_mb="$3"
  local cpu_cores="$4"

  # Validate expected patterns
  for pattern in "${EXPECTED_PATTERNS[@]}"; do
    if ! grep -q "$pattern" "$output_file"; then
      echo "ERROR: Missing expected pattern: $pattern"
      return 1
    fi
  done

  # Validate environment correlation
  if ! grep -q "Total Memory: ${memory_mb} MB" "$output_file"; then
    echo "ERROR: Memory mismatch - expected ${memory_mb} MB"
    return 1
  fi

  if ! grep -q "CPU Cores: ${cpu_cores}" "$output_file"; then
    echo "ERROR: CPU core mismatch - expected ${cpu_cores} cores"
    return 1
  fi

  if ! grep -q "Architecture: ${arch}" "$output_file"; then
    echo "ERROR: Architecture mismatch - expected ${arch}"
    return 1
  fi

  echo "SUCCESS: Bootstrap test passed for $arch with environment validation"
  return 0
}
```

### Property-Based Testing (Limited Scope)

Property-based testing is limited to components that can be tested in isolation:

#### Testable Components
- Memory map parsing functions (with mock bootloader data)
- Command line parameter parsing (with test strings)
- Boot protocol detection logic (with simulated data structures)
- CPU feature detection (with mock CPUID responses)

#### Non-Testable Components
- Actual mode transitions (requires real hardware/virtualization)
- Bootloader handoff (requires actual bootloader)
- Hardware initialization (requires physical hardware)
- Architecture-specific assembly code (requires execution environment)

### Integration Testing Strategy

```makefile
# Integration testing through QEMU with environment validation
check-bootstrap: test-kernel-x86_64 test-kernel-arm64 test-kernel-riscv64
  @echo "Running bootstrap QEMU smoke tests with environment validation..."
  ./scripts/qemu-bootstrap-test.sh x86_64 512 2
  ./scripts/qemu-bootstrap-test.sh arm64 512 2
  ./scripts/qemu-bootstrap-test.sh riscv64 512 2
  @echo "All bootstrap tests passed with environment validation"

# Build minimal test kernels for each architecture
test-kernel-x86_64: tests/bootstrap/test_kernel.c arch/x86_64/bootstrap.S
  $(CC) $(TEST_KERNEL_CFLAGS) -o $@ $^

test-kernel-arm64: tests/bootstrap/test_kernel.c arch/arm64/bootstrap.S
  $(AARCH64_CC) $(TEST_KERNEL_CFLAGS) -o $@ $^

test-kernel-riscv64: tests/bootstrap/test_kernel.c arch/riscv64/bootstrap.S
  $(RISCV64_CC) $(TEST_KERNEL_CFLAGS) -o $@ $^
```
