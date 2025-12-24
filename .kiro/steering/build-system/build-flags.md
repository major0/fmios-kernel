---
inclusion: always
---

# Build System Compiler Flags

This document specifies the compiler flags that must be used for different build targets in the FMI/OS project with the new autoconf/automake-based build system.

## Autoconf/Automake Integration

All compiler and linker flags are managed through the autoconf/automake system. The configure script detects the target architecture and sets appropriate flags automatically.

## Kernel Build Flags

When building kernel code (any code in `kernel/` directory), the following flags MUST be configured by autoconf/automake:

### Required Flags (Set by configure.ac)
- `-ffreestanding` - Indicates that the kernel is a freestanding environment without standard library
- `-nostdlib` - Do not link against standard library
- `-fno-builtin` - Disable built-in function optimizations that assume standard library
- `-fno-stack-protector` - Disable stack protection (kernel manages its own stack protection)
- `-fPIC` - Generate Position Independent Code for all kernel modules
- `-fno-plt` - Disable Procedure Linkage Table for direct calls (optional optimization)

**Speculative Execution Attack Mitigations:**
- `-mretpoline` - Enable retpoline mitigations for indirect branches (x86_64)
- `-mindirect-branch=thunk-extern` - Use external thunks for indirect branches (x86_64)
- `-mfunction-return=thunk-extern` - Use external thunks for function returns (x86_64)
- `-mindirect-branch-register` - Restrict indirect branch targets to registers (x86_64)
- `-fcf-protection=full` - Enable Intel CET (Control Flow Enforcement Technology) when available
- `-fstack-protector-strong` - Enable stack canary protection for functions with local arrays

**Bounds Violation and Off-by-One Detection:**
- `-fsanitize=bounds` - Enable array bounds checking (debug builds)
- `-Warray-bounds` - Warn about array bounds violations at compile time
- `-Wstringop-overflow` - Warn about string operation buffer overflows
- `-fstack-clash-protection` - Protect against stack clash attacks
- `-D_FORTIFY_SOURCE=2` - Enhanced bounds checking for standard library functions (debug builds)
- `-Wformat-overflow` - Warn about format string buffer overflows
- `-Wformat-truncation` - Warn about format string truncation issues

**Position Independent Code (PIC) Requirements:**
- Production kernel code MUST be compiled with `-fPIC` to support loading at any address
- Debug kernel code MUST be compiled with `-fno-pic` for easier debugging and development
- All kernel sources MUST support both PIC and non-PIC compilation modes depending on debugging configuration
- Enables KASLR (Kernel Address Space Layout Randomization) for security in production builds
- Supports flexible bootloader integration and memory layouts in production
- Assembly code MUST use constraints that work in both PIC and non-PIC modes (e.g., "Nd" for port I/O)
- No absolute addresses allowed in kernel code or data structures regardless of PIC mode

**Note:** The `-nostdinc` flag is NOT used, allowing access to compiler-provided headers like `stddef.h`, `stdint.h`, `stdbool.h`, `stdatomic.h`, and `string.h` which are safe in freestanding environments. The kernel MUST use these standard functions and types instead of custom implementations.

**Standard Library Function Usage:**
- **REQUIRED**: Use compiler-provided functions from `<string.h>`: `memset`, `memcpy`, `memcmp`, `memmove`, `strlen`
- **REQUIRED**: Use safe standard functions: `strncpy`, `strncat`, `snprintf`, `vsnprintf`
- **PROHIBITED**: Unsafe functions: `strcpy`, `strcat`, `sprintf`, `vsprintf`, `gets`
- **PROHIBITED**: Custom implementations of standard library functions (e.g., `kstring.c`, `kmemory.c`)
- **PROHIBITED**: Creating kernel-specific wrappers for standard memory/string functions

### Architecture-Specific Flags (Set by configure.ac based on --target)

#### x86_64 Flags
- `-mcmodel=kernel` - Use kernel code model
- `-mno-red-zone` - Disable red zone optimization (required for kernel)
- `-mno-mmx -mno-sse -mno-sse2` - Disable floating point instructions in kernel

**x86_64 PIC/Non-PIC Requirements:**
- Assembly code MUST use constraints that work in both modes: `"Nd"` for port numbers (not absolute addresses)
- Global variables accessed via: `mov symbol(%rip), %rax` (PIC) or `mov $symbol, %rax` (non-PIC) - compiler handles this
- Function calls use: `call symbol@PLT` (PIC) or `call symbol` (non-PIC) - compiler handles this
- Inline assembly should use register constraints that work in both modes
- Port I/O operations use immediate or register constraints: `"Nd"` constraint works in both modes

**x86_64 Speculative Execution Mitigations:**
- Assembly code MUST use `lfence` after indirect branches: `call *%rax; lfence`
- Memory barriers required before sensitive operations: `mfence`
- No indirect jumps without retpoline protection: `jmp __x86_indirect_thunk_rax`
- Branch prediction barriers: `lfence` after conditional branches accessing sensitive data
- Serializing instructions (`cpuid`, `lfence`) before privileged operations

#### ARM64 Flags  
- `-mcpu=cortex-a57` - Target Cortex-A57 processor
- `-mgeneral-regs-only` - Use general-purpose registers only (no NEON/FP)

**ARM64 PIC Requirements:**
- Assembly code MUST use PC-relative addressing: `adrp x0, symbol` + `add x0, x0, :lo12:symbol`
- Global variables accessed via PC-relative loads
- Function calls use: `bl symbol` (branch and link with relative addressing)
- No absolute addressing in assembly code

**ARM64 Speculative Execution Mitigations:**
- Assembly code MUST use speculation barriers: `dsb sy; isb` after indirect branches
- Memory barriers before sensitive operations: `dmb sy`
- No indirect branches without proper CSV2 checks when available
- Branch prediction barriers: `isb` after conditional branches accessing sensitive data
- Instruction synchronization barriers before privileged operations

#### RISC-V Flags
- `-march=rv64imac` - Target RV64IMAC instruction set
- `-mabi=lp64` - Use LP64 ABI

**RISC-V PIC Requirements:**
- Assembly code MUST use PC-relative addressing with `auipc` instruction
- Global variables accessed via: `auipc reg, %pcrel_hi(symbol)` + `addi reg, reg, %pcrel_lo(symbol)`
- Function calls use: `call symbol` (PC-relative call)
- No absolute addressing in assembly code

**RISC-V Speculative Execution Mitigations:**
- Assembly code MUST use fence instructions: `fence.i` after indirect branches
- Memory barriers before sensitive operations: `fence rw,rw`
- Instruction fence after code modifications: `fence.i`
- No speculative execution patterns in assembly code

### Include Paths for Kernel (Set by Makefile.am)
- `-Iinclude` - Generic headers
- `-Ilib/c` - Kernel libc headers  
- `-Iarch/$(target_arch)/include` - Architecture-specific headers (via symlink)

### Linker Paths for Kernel (Set by Makefile.am)
- `-Llib/c` - Link against kernel libc implementation
- `-Llib/arch` - Link against architecture-specific libraries (via symlink)

## Test Build Flags

When building test code (any code in `tests/` directory), the following flags MUST be configured by autoconf/automake:

### Prohibited Flags
- DO NOT use `-ffreestanding` - Tests run in hosted environment
- DO NOT use `-nostdlib` - Tests need standard library
- DO NOT use `-nostdinc` - Tests use standard headers

### Required Flags (Set by configure.ac)
- `-D_POSIX_C_SOURCE=200809L` - Enable POSIX features for testing
- Standard warning flags: `-Wall -Wextra -Werror`
- Debug flags: `-O0 -g` for debugging

### Include Paths for Tests (Set by Makefile.am)
- `-Iinclude` - Access to generic headers
- `-Ilib/c` - Use kernel klibc header (single source of truth)
- `-Itests/unity/src` - Unity test framework
- `-Itests` - Test-specific headers

### Linker Paths for Tests (Set by Makefile.am)
- `-Ltests/klibc` - Link against testing klibc implementation

## Configure Script Example

### Basic Configuration
```bash
# Configure for x86_64 with default options
./configure --target=x86_64-fmios

# Configure for ARM64 with serial console only
./configure --target=aarch64-fmios --disable-video-console

# Configure for RISC-V with debug build
./configure --target=riscv64-fmios --enable-debug
```

### Cross-Compilation
```bash
# Cross-compile from x86_64 host to ARM64 target
./configure --host=x86_64-linux-gnu --target=aarch64-fmios

# Cross-compile with specific toolchain
./configure --target=riscv64-fmios CC=riscv64-linux-gnu-gcc
```

## Makefile.am Structure

### Kernel Makefile.am
```makefile
# Kernel-specific flags (set by configure.ac)
AM_CFLAGS = $(KERNEL_CFLAGS) $(ARCH_CFLAGS)
AM_LDFLAGS = $(KERNEL_LDFLAGS) $(ARCH_LDFLAGS)

# Include paths
AM_CPPFLAGS = -Iinclude -Ilib/c -Iarch/$(target_arch)/include

# Linker library paths  
LDADD = -Llib/c -Llib/arch
```

### Test Makefile.am
```makefile
# Test-specific flags (set by configure.ac)
AM_CFLAGS = $(TEST_CFLAGS)
AM_LDFLAGS = $(TEST_LDFLAGS)

# Include paths for tests
AM_CPPFLAGS = -Iinclude -Ilib/c -Itests/unity/src -Itests

# Linker library paths for tests
LDADD = -Ltests/klibc -lunity
```

## Rationale

### Why autoconf/automake for Flag Management
The autoconf/automake system provides:
- **Standardized Configuration**: Uses GNU standard configuration interface
- **Cross-Compilation Support**: Automatic toolchain detection and configuration
- **Feature Detection**: Automatic detection of compiler capabilities
- **Maintainability**: Centralized flag management in configure.ac

### Why `-ffreestanding` for Kernel
The kernel operates in a freestanding environment where:
- No standard library is available
- The kernel provides its own implementations of basic functions
- Compiler cannot assume standard library behavior
- Memory layout and calling conventions are kernel-controlled

### Why NOT `-ffreestanding` for Tests
Tests run in a hosted environment where:
- Standard library is available and needed
- Tests use standard I/O, memory allocation, threading
- Testing framework requires standard library functions
- Debugging tools expect standard environment

## Directory Structure Impact

### New Structure
```
fmios/
├── include/           # Generic headers (-Iinclude)
│   └── arch -> ../arch/x86_64/include  # Architecture symlink
├── lib/               # Generic libraries
│   ├── arch -> ../arch/x86_64/lib      # Architecture symlink  
│   └── c/             # Kernel C library (-Ilib/c, -Llib/c)
├── arch/x86_64/       # Architecture-specific code
│   ├── include/       # x86_64 headers (via symlink)
│   └── lib/           # x86_64 libraries (via symlink)
└── tests/
    └── klibc/         # Testing klibc (-Ltests/klibc)
```

### Flag Configuration
```makefile
# Automatically configured by autoconf/automake
KERNEL_CFLAGS = -ffreestanding -nostdlib -fno-builtin $(ARCH_CFLAGS)
KERNEL_LDFLAGS = -nostdlib -static $(ARCH_LDFLAGS)
KERNEL_CPPFLAGS = -Iinclude -Ilib/c -Iarch/$(target_arch)/include

TEST_CFLAGS = -Wall -Wextra -Werror -O0 -g -D_POSIX_C_SOURCE=200809L
TEST_LDFLAGS = 
TEST_CPPFLAGS = -Iinclude -Ilib/c -Itests/unity/src -Itests
```

## Implementation Notes

### Dual Klibc Implementation Strategy
- **Single Header Approach**: The kernel klibc header (`lib/c/klibc.h`) serves as the single source of truth
- **Build System Selection**: Autoconf/automake selects appropriate library path via `-L` flags
- **Interface Parity**: Both implementations must provide identical interfaces

### Architecture Symlink Management
- **Automatic Creation**: Configure script creates symlinks based on `--target` option
- **Clean Targets**: Makefiles remove symlinks during clean operations
- **Distribution**: Symlinks are not included in distribution tarballs

This autoconf/automake-based approach ensures clean, maintainable, and portable builds across all supported architectures while providing proper feature configuration and cross-compilation support.