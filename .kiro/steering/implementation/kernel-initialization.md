# Kernel Initialization Design

This document establishes the design principles for kernel initialization and memory management in FMI/OS.

## Core Principles

1. **Centralized Initialization**: All subsystem initializers are defined in `kernel/main.c`
2. **Linker Script Integration**: Custom linker scripts can group initialization code for memory reclamation
3. **Memory Efficiency**: Initialization code can be freed after scheduler startup
4. **Clean Separation**: Runtime functions are separate from initialization code
5. **Subsystem Organization**: Each kernel subsystem follows the established organization pattern (see `kernel-subsystem-organization.md`)

## Initialization Architecture

### Entry Point Flow

```
arch/x86_64/bootstrap.S
    ↓ (early hardware setup)
kernel/main.c:kmain()
    ↓ (subsystem initialization)
kernel/main.c:start_scheduler()
    ↓ (hand off control)
[initialization code can be freed]
```

### Code Organization

#### kernel/main.c
Contains:
- `kmain()` - Main kernel entry point
- All subsystem initializers:
  - `sched_init()` - Scheduler initialization
  - `proc_init()` - Process management initialization
  - `mem_init()` - Memory manager initialization
  - `ns_init()` - Namespace filesystem initialization
  - `ipc_init()` - IPC system initialization
- `start_scheduler()` - Transfer control to scheduler
- **NOTE**: These functions are defined in `main.c`, not in their respective subsystem files

#### Subsystem Files (kernel/*.c)
Contains:
- Runtime functions only (no initializers)
- Global resource management for the subsystem
- Filesystem interface implementations
- Data structures and algorithms
- Public APIs for other subsystems
- **NOTE**: No `*_init()` functions in these files - initialization is centralized in `main.c`

### Linker Script Integration

The custom linker script groups initialization code for memory reclamation:

```ld
/* Linker script example */
SECTIONS {
    /* Initialization code - can be freed after boot */
    .init : ALIGN(4K) {
        __init_start = .;
        *kernel/main.o(.text)      /* Entry point and initializers */
        *kernel/main.o(.rodata)    /* Initialization constants */
        *kernel/main.o(.data)      /* Initialization data */
        __init_end = .;
    } > RAM

    /* Runtime kernel code - permanent */
    .text : ALIGN(4K) {
        __text_start = .;
        *(.text)                   /* All other kernel code */
        __text_end = .;
    } > RAM

    .rodata : ALIGN(4K) {
        *(.rodata)                 /* Runtime constants */
    } > RAM

    .data : ALIGN(4K) {
        *(.data)                   /* Runtime data */
    } > RAM
}
```

### Memory Reclamation

After scheduler startup:

```c
// In kernel/main.c after start_scheduler()
void free_init_memory(void) {
    extern char __init_start[], __init_end[];
    size_t init_size = __init_end - __init_start;

    // Mark initialization pages as free
    free_pages(__init_start, init_size);

    klogf(KLOG_INFO, "Freed %zu bytes of initialization code", init_size);
}
```

## Implementation Guidelines

### Subsystem Initializer Pattern

Each initializer in `kernel/main.c` follows this pattern:

```c
// In kernel/main.c
static int memory_init(void) {
    // Initialize memory management subsystem
    // Call functions from kernel/memory.c as needed
    return memory_subsystem_setup();  // Implemented in kernel/memory.c
}

static int process_init(void) {
    // Initialize process management subsystem
    // Call functions from kernel/process.c as needed
    return process_subsystem_setup(); // Implemented in kernel/process.c
}
```

### Subsystem Runtime Functions

Runtime functions in subsystem files provide the actual implementation:

```c
// In kernel/memory.c
int memory_subsystem_setup(void) {
    // Actual memory manager setup code
    // This function is called by memory_init() in main.c
    return 0;
}

void* kmalloc(size_t size) {
    // Runtime memory allocation
    // This stays in memory after initialization
}
```

### Error Handling

Initialization errors are handled centrally in `main.c`:

```c
// In kernel/main.c
void kmain(void) {
    if (memory_init() != 0) {
        kpanic("Memory initialization failed");
    }

    if (process_init() != 0) {
        kpanic("Process initialization failed");
    }

    // Continue with other initializers...

    start_scheduler();

    // This point should never be reached
    kpanic("Scheduler returned unexpectedly");
}
```

## Benefits

### Memory Efficiency
- Initialization code is freed after boot
- Reduces kernel memory footprint
- More memory available for runtime operations

### Clean Architecture
- Clear separation between initialization and runtime
- Centralized error handling
- Consistent initialization order

### Debugging Support
- All initialization code in one place
- Easy to add debug output for boot process
- Simplified boot failure diagnosis

### Linker Script Flexibility
- Custom memory layouts possible
- Support for different memory configurations
- Easy to adjust for different architectures

## Build System Integration

### Makefile.am Configuration

The kernel Makefile.am ensures proper compilation:

```makefile
# In kernel/Makefile.am
fmi_kernel_SOURCES = \
    main.c \
    memory.c \
    process.c \
    thread.c \
    namespace.c

# Ensure main.c is compiled with initialization flags
main.o: main.c
    $(CC) $(KERNEL_CFLAGS) -DKERNEL_INIT -c $< -o $@
```

### Architecture Integration

Architecture-specific bootstrap code calls into `main.c`:

```assembly
# In arch/x86_64/bootstrap.S
.global _start
_start:
    # Set up stack
    mov $kernel_stack_top, %rsp

    # Call main kernel entry point
    call kmain

    # Should never return
    hlt
```

This design provides a clean, efficient, and maintainable approach to kernel initialization while enabling memory reclamation and supporting custom linker scripts.
