# Autoconf/Automake Build System Guidelines

This document establishes the requirements and guidelines for the FMI/OS autoconf/automake-based build system with proper architecture separation.

## Core Principles

1. **Standard GNU Build System**: Use autoconf/automake for configuration management
2. **Architecture Separation**: Clean separation between generic and architecture-specific code
3. **Feature Configuration**: Support for enabling/disabling kernel features at configure time
4. **Cross-Compilation**: Seamless support for multiple target architectures

## Directory Structure Requirements

### Top-Level Organization
```
fmios/
├── configure.ac       # Autoconf configuration script
├── Makefile.am        # Top-level automake makefile
├── include/           # Generic headers (architecture-neutral)
│   └── arch -> ../arch/x86_64/include  # Symlink managed by configure
├── lib/               # Generic libraries (architecture-neutral)
│   ├── arch -> ../arch/x86_64/lib      # Symlink managed by configure
│   └── c/             # Kernel C library (klibc)
├── drivers/           # Generic drivers (architecture-neutral)
│   ├── console.c      # Generic console management
│   └── debug.c        # Debug driver
├── kernel/            # Generic kernel code (architecture-neutral)
│   ├── main.c         # Main kernel entry point and subsystem initializers
│   ├── kobj.c         # Kernel object management (runtime functions only)
│   ├── memory.c       # Memory management (runtime functions only)
│   ├── slab.c         # Slab allocator (runtime functions only)
│   ├── process.c      # Process management (runtime functions only)
│   ├── thread.c       # Thread management (runtime functions only)
│   ├── namespace.c    # Namespace filesystem (runtime functions only)
│   └── rfork.c        # Process creation (runtime functions only)
├── arch/              # Architecture-specific code
│   ├── x86_64/
│   │   ├── Makefile.am    # Architecture-specific build rules
│   │   ├── bootstrap.S    # Architecture bootstrap (calls kernel/main.c)
│   │   ├── include/       # x86_64-specific headers
│   │   ├── lib/           # x86_64-specific libraries
│   │   │   └── page.c     # Architecture-specific page allocator
│   │   └── drivers/       # x86_64-specific drivers (serial, video)
│   ├── arm64/
│   └── riscv64/
├── tests/             # Test suite
│   └── Makefile.am    # Test build rules
└── docs/              # Documentation
```

### Architecture Separation Rules

#### Generic Code (include/, lib/, kernel/, drivers/)
- **MUST** be architecture-neutral
- **MUST NOT** contain inline assembly (`__asm__`, `asm`)
- **MUST NOT** depend on byte order or endianness
- **MUST NOT** use architecture-specific types (e.g., `__int128` on x86_64)
- **MUST NOT** use architecture-specific compiler intrinsics
- **MUST** use portable C constructs only
- **NOTE**: Generic drivers provide interfaces only; hardware-specific implementations are in arch-specific drivers
- **NOTE**: Kernel subsystems call architecture-specific functions via well-defined interfaces

#### Architecture-Specific Code (arch/*/include/, arch/*/lib/, arch/*/drivers/)
- **MAY** contain inline assembly
- **MAY** depend on endianness and byte order
- **MAY** use architecture-specific types and constants
- **MAY** use compiler intrinsics specific to the target architecture
- **MUST** provide consistent interfaces to generic code
- **MUST** implement required architecture-specific functions

## Configure Script Requirements

### Target Architecture Selection
```bash
# Configure for specific architectures
./configure --target=x86_64-fmios
./configure --target=aarch64-fmios  
./configure --target=riscv64-fmios
```

### Console Driver Configuration
```bash
# Enable/disable console drivers
./configure --enable-serial-console    # Enable serial console (default: yes)
./configure --disable-serial-console   # Disable serial console
./configure --enable-video-console     # Enable VGA video console (default: yes)
./configure --disable-video-console    # Disable video console
./configure --enable-debug-console     # Enable debug console (default: yes)
./configure --disable-debug-console    # Disable debug console
```

# Development and debugging features
./configure --enable-debug             # Enable debug builds (default: no)
./configure --enable-valgrind          # Enable Valgrind integration (default: no)
./configure --enable-qemu-testing      # Enable QEMU smoke tests (default: yes)
```

### Build Configuration
```bash
# Cross-compilation support
./configure --host=x86_64-linux-gnu --target=x86_64-fmios
./configure --host=x86_64-linux-gnu --target=aarch64-fmios
./configure --host=x86_64-linux-gnu --target=riscv64-fmios

# Build type selection
./configure --enable-debug             # Debug build with -O0 -g
./configure --disable-debug            # Release build with -O2
```

## Makefile.am Structure

### Top-Level Makefile.am
```makefile
# Subdirectories to build
SUBDIRS = lib drivers arch/$(target_arch) kernel tests

# Architecture symlink management
all-local:
	$(LN_S) -f ../arch/$(target_arch)/include include/arch
	$(LN_S) -f ../arch/$(target_arch)/lib lib/arch

clean-local:
	rm -f include/arch lib/arch

# Distribution files
EXTRA_DIST = docs/ scripts/

# Configure-generated files
BUILT_SOURCES = include/arch lib/arch
```

### Architecture-Specific Makefile.am (arch/x86_64/Makefile.am)
```makefile
# Architecture-specific sources
arch_SOURCES = \
	bootstrap.S \
	entry.S \
	interrupts.c \
	mmu.c \
	context.c

# Architecture-specific drivers
driver_SOURCES = \
	drivers/serial_console.c \
	drivers/video_console.c

# Build libarch.a for this architecture
noinst_LIBRARIES = libarch.a
libarch_a_SOURCES = $(arch_SOURCES) $(driver_SOURCES)

# Architecture-specific compiler flags
AM_CFLAGS = -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2

# Install architecture-specific headers
archincludedir = $(includedir)/arch
archinclude_HEADERS = include/*.h
```

## Compiler and Linker Flag Management

### Configure.ac Flag Configuration
```autoconf
# Architecture-specific compiler flags
case "$target_arch" in
    x86_64)
        ARCH_CFLAGS="-mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2"
        ARCH_LDFLAGS="-z max-page-size=0x1000"
        ;;
    aarch64)
        ARCH_CFLAGS="-mcpu=cortex-a57 -mgeneral-regs-only"
        ARCH_LDFLAGS=""
        ;;
    riscv64)
        ARCH_CFLAGS="-march=rv64imac -mabi=lp64"
        ARCH_LDFLAGS=""
        ;;
esac

# Kernel build flags with Position Independent Code, Speculative Execution Protection, and Bounds Checking
KERNEL_CFLAGS="-std=c11 -Wall -Wextra -Werror -ffreestanding -fno-stack-protector"
KERNEL_CFLAGS="$KERNEL_CFLAGS -nostdlib -fno-builtin -fPIC -fno-plt"
KERNEL_CFLAGS="$KERNEL_CFLAGS -mretpoline -mindirect-branch=thunk-extern -mfunction-return=thunk-extern"
KERNEL_CFLAGS="$KERNEL_CFLAGS -mindirect-branch-register -fcf-protection=full -fstack-protector-strong"
KERNEL_CFLAGS="$KERNEL_CFLAGS -Warray-bounds -Wstringop-overflow -fstack-clash-protection"
KERNEL_CFLAGS="$KERNEL_CFLAGS -Wformat-overflow -Wformat-truncation"
KERNEL_CFLAGS="$KERNEL_CFLAGS -Iinclude -Ilib/c $ARCH_CFLAGS"

# Debug build additional flags for enhanced bounds checking
if test "x$enable_debug" = "xyes"; then
    KERNEL_CFLAGS="$KERNEL_CFLAGS -fsanitize=bounds -D_FORTIFY_SOURCE=2"
fi

KERNEL_LDFLAGS="-nostdlib -static -Llib/c -Llib/arch $ARCH_LDFLAGS"

# Test build flags
TEST_CFLAGS="-std=c11 -Wall -Wextra -Werror -O0 -g -D_POSIX_C_SOURCE=200809L"
TEST_CFLAGS="$TEST_CFLAGS -Iinclude -Ilib/c -Itests/unity/src"
TEST_LDFLAGS="-Ltests/klibc"
```

### Conditional Compilation
```autoconf
# Console driver configuration
AM_CONDITIONAL([ENABLE_SERIAL_CONSOLE], [test "x$enable_serial_console" = "xyes"])
AM_CONDITIONAL([ENABLE_VIDEO_CONSOLE], [test "x$enable_video_console" = "xyes"])
AM_CONDITIONAL([ENABLE_DEBUG_CONSOLE], [test "x$enable_debug_console" = "xyes"])
```

## Symlink Management

### Automatic Symlink Creation
The configure script and Makefiles must manage architecture symlinks:

```bash
# In configure.ac
AC_CONFIG_COMMANDS([arch-symlinks], [
    rm -f include/arch lib/arch
    ln -sf ../arch/$target_arch/include include/arch
    ln -sf ../arch/$target_arch/lib lib/arch
], [target_arch=$target_arch])
```

### Symlink Cleanup
```makefile
# In top-level Makefile.am
clean-local:
	rm -f include/arch lib/arch

distclean-local:
	rm -f include/arch lib/arch
```

## Code Organization Rules

### Main Kernel Entry Point

The main kernel entry point and subsystem initializers are located in `kernel/main.c`:
- Architecture-specific bootstrap code (e.g., `arch/x86_64/bootstrap.S`) performs early initialization
- Bootstrap sets up minimal environment (stack, basic CPU state)
- Bootstrap calls `kmain()` function in `kernel/main.c`
- **All subsystem initializers are defined in `kernel/main.c`** (not in their respective source files):
  - Memory manager initialization (`mem_init()` defined in `kernel/main.c`)
  - Scheduler initialization (`sched_init()` defined in `kernel/main.c`)
  - IPC system initialization (`ipc_init()` defined in `kernel/main.c`)
  - Process management initialization (`proc_init()` defined in `kernel/main.c`)
  - Thread management initialization (`thread_init()` defined in `kernel/main.c`)
  - Namespace filesystem initialization (`namespace_init()` defined in `kernel/main.c`)
- Individual subsystem source files contain runtime functions only, not initializers
- This allows custom linker scripts to group initializers with the entry point
- Initializer code can be freed once control is handed off to the scheduler

### Linker Script Integration

The custom linker script can group initialization code:
```ld
.init : {
    *kernel/main.o(.text)     /* Entry point and initializers */
    *kernel/main.o(.rodata)   /* Initialization data */
} > RAM

.text : {
    *(.text)                  /* Runtime kernel code */
} > RAM
```

After scheduler startup, the `.init` section can be freed to reclaim memory.

### Moving Architecture-Specific Code

#### Serial Console Driver (arch/x86_64/drivers/serial_console.c)
- Move inline assembly (`outb`, `inb` functions)
- Move x86_64-specific port addresses
- Keep generic console interface in generic code

#### Video Console Driver (arch/x86_64/drivers/video_console.c)  
- Move VGA memory mapping (0xB8000)
- Move x86_64-specific video initialization
- Keep generic text rendering in generic code

#### Memory Management
- Move page table manipulation to arch-specific code
- Move MMU configuration to arch-specific code
- Keep generic memory allocation in generic code

### Generic Drivers (drivers/)

Generic drivers provide architecture-neutral interfaces:
- Console management (`drivers/console.c`) - manages console driver registration and multiplexing
- Debug driver (`drivers/debug.c`) - generic debug output interface
- **NOTE**: Hardware-specific implementations are in `arch/*/drivers/`
- **NOTE**: SCSI and RDMA are userspace services, not kernel drivers

## Testing Integration

### Architecture-Specific Testing
```makefile
# In tests/Makefile.am
check-local:
if ENABLE_QEMU_TESTING
	$(MAKE) qemu-test-$(target_arch)
endif

qemu-test-x86_64:
	qemu-system-x86_64 -kernel kernel/fmi-kernel -serial mon:stdio -nographic

qemu-test-aarch64:
	qemu-system-aarch64 -machine virt -kernel kernel/fmi-kernel -serial mon:stdio -nographic

qemu-test-riscv64:
	qemu-system-riscv64 -machine virt -kernel kernel/fmi-kernel -serial mon:stdio -nographic
```

## Code Quality Enforcement

The build system MUST enforce code quality standards to prevent maintenance issues and ensure consistency across the codebase.

### Forward Declaration Prohibition

**CRITICAL REQUIREMENT: The build system MUST detect and prevent forward declarations.**

#### Build-Time Validation
```makefile
# In configure.ac - Add forward declaration checking
AC_ARG_ENABLE([strict-headers],
    AS_HELP_STRING([--enable-strict-headers], [Enable strict header validation (default: yes)]),
    [enable_strict_headers=$enableval],
    [enable_strict_headers=yes])

AM_CONDITIONAL([ENABLE_STRICT_HEADERS], [test "x$enable_strict_headers" = "xyes"])
```

#### Makefile Integration
```makefile
# In top-level Makefile.am
if ENABLE_STRICT_HEADERS
check-headers:
	@echo "Checking for forward declarations..."
	@$(top_srcdir)/scripts/check-forward-declarations.sh $(top_srcdir)
	@echo "Header validation passed"

all-local: check-headers
endif

# In individual Makefile.am files
if ENABLE_STRICT_HEADERS
$(OBJECTS): check-headers
endif
```

#### Validation Script Requirements
The `scripts/check-forward-declarations.sh` script MUST:

1. **Scan all source files** (`.c` and `.h`) for forward declarations
2. **Identify prohibited patterns**:
   - `struct name;` declarations
   - Function declarations that exist in headers
   - Type declarations that duplicate header definitions
3. **Cross-reference with headers** to detect duplicates
4. **Generate clear error messages** indicating:
   - Which file contains the forward declaration
   - Which header should be included instead
   - Line number and context of the violation
5. **Return non-zero exit code** to fail the build when violations are found

#### Error Message Format
```bash
ERROR: Forward declaration found in kernel/memory.c:15
  Found: struct process_s;
  Solution: #include "process.h"
  
ERROR: Duplicate function declaration in tests/test_memory.c:23
  Found: int memory_init(void);
  Solution: #include "memory.h"
```

#### Integration with CI/CD
```makefile
# Ensure header validation runs in continuous integration
check: check-headers
	@echo "All code quality checks passed"

# Make header validation mandatory for release builds
dist-hook: check-headers
	@echo "Distribution package validated"
```

### Header Dependency Validation

The build system SHOULD also validate:

1. **Circular Dependencies**: Detect and report circular header includes
2. **Missing Includes**: Identify when headers use types without including their definitions
3. **Unused Includes**: Flag headers that are included but not used
4. **Include Order**: Enforce consistent include ordering within files

### Enforcement Benefits

1. **Maintainability**: Single source of truth for all declarations
2. **Consistency**: Prevents inconsistent declarations across files
3. **Build Reliability**: Catches interface mismatches at build time
4. **Developer Productivity**: Clear error messages guide developers to correct solutions
5. **Code Quality**: Enforces clean, well-structured code organization

## Implementation Checklist

- [ ] Create configure.ac with architecture detection and feature options
- [ ] Create top-level Makefile.am with subdirectory management
- [ ] Create architecture-specific Makefile.am files
- [ ] Move architecture-specific code to arch/ directories
- [ ] Update all include paths to use autoconf/automake flags
- [ ] Implement symlink management in build system
- [ ] Add conditional compilation for optional features
- [ ] Update test system for new directory structure
- [ ] **Implement forward declaration checking script**
- [ ] **Add header validation to build system**
- [ ] **Enable strict header validation by default**
- [ ] Verify cross-compilation works for all target architectures
- [ ] Test all configure options and feature combinations

This autoconf/automake-based build system provides a clean, maintainable, and standards-compliant approach to building FMI/OS across multiple architectures with proper feature configuration and code quality enforcement.