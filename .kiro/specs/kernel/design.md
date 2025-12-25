# Design Document: FMI/OS (Flexible Microkernel Infrastructure Operating System)

## Overview

FMI/OS is a Plan 9-inspired microkernel operating system that combines the elegance of Plan 9's "everything is a file" philosophy with modern container support, multi-architecture capabilities, and robust security through capability-based access control. The system centers around a unified `//` namespace where all services register themselves, creating a coherent and discoverable system interface.

The architecture maintains microkernel principles by keeping complex services in user space while providing essential kernel services including POSIX thread/process management, memory management, IPC, and the core `//` namespace filesystem. The system supports OCI-compliant containers through its `rfork()` mechanism and provides comprehensive networking with Plan 9-style distributed computing capabilities.

## Test-Driven Development Architecture

FMI/OS is designed and implemented using comprehensive Test-Driven Development (TDD) methodology where tests define expected behavior before any implementation code is written. This approach ensures:

### TDD Design Principles

1. **Specification-First Design**: All system components are specified through tests that define exact expected behavior
2. **Incremental Development**: Each component is developed in test-implementation pairs, ensuring working functionality at each step
3. **Quality Assurance**: All code is written to satisfy well-defined tests, ensuring correctness from the start
4. **Regression Prevention**: Comprehensive test coverage prevents future changes from breaking existing functionality
5. **Living Documentation**: Tests serve as executable documentation of system behavior and requirements

### TDD Implementation Strategy

**Phase-Based Development**: The system is developed in phases where each phase consists of:
1. **Test Definition**: Comprehensive tests are written first for all components in the phase
2. **Implementation**: Code is written specifically to make the tests pass
3. **Validation**: All tests must pass before proceeding to the next phase
4. **Integration**: Components are integrated and tested together

**Multi-Architecture TDD**: Tests are written for all target architectures (x86_64, ARM64, RISC-V) before implementation, ensuring consistent behavior across platforms.

**Dual Testing Environment**: The system uses both unit testing (with testing klibc) and QEMU smoke testing (with kernel klibc) to validate functionality in both simulated and actual kernel environments.

## Architecture

### Core Kernel Components

**Standard Headers Usage:**
The kernel uses a centralized `ktypes.h` header that includes all necessary compiler-provided freestanding headers:
- `stddef.h` - Provides `size_t`, `ptrdiff_t`, `NULL`, and `offsetof`
- `stdint.h` - Provides standard integer types (`uint8_t`, `int32_t`, etc.)
- `stdbool.h` - Provides `bool`, `true`, and `false`
- `stdarg.h` - Provides variadic argument support (`va_list`, `va_start`, etc.)
- `stdatomic.h` - Provides standard atomic types and operations

All kernel source files must include `ktypes.h` instead of directly including these standard headers. This approach provides centralized type management while ensuring compatibility with standard C and maintaining freestanding environment requirements.

The FMI/OS kernel provides essential services while maintaining microkernel principles:

```
┌─────────────────────────────────────────────────────────────┐
│                    FMI/OS Kernel                           │
├─────────────────────────────────────────────────────────────┤
│  Thread Manager    │  Process Manager  │  Process Groups   │
├─────────────────────────────────────────────────────────────┤
│  Memory Manager    │     IPC System    │    Scheduler      │
├─────────────────────────────────────────────────────────────┤
│           // Namespace Filesystem & //proc, //kern         │
├─────────────────────────────────────────────────────────────┤
│  Capability System │ Hardware Abstraction │ Signal Handling │
└─────────────────────────────────────────────────────────────┘
```

**Concurrent Safety Design:**
- All kernel data structures use unified `kobj_t` header with integrated locking
- Object pools provide efficient allocation/deallocation without frequent memory operations
- Read-write locks in kernel objects for structures with frequent reads and occasional writes
- Atomic reference counting for safe memory management and cleanup
- Lock ordering hierarchy to prevent deadlocks
- Slab allocator integration for optimal memory utilization
- Objects returned to pools rather than freed to main memory for performance

**Thread-Safe Data Structure Implementation:**
- **Lists**: `list_t` provides thread-safe doubly-linked lists with read-write locks, automatic element counting, and kobj owner integration
- **Hash Tables**: `hash_table_t` provides thread-safe hash tables with per-bucket locking for fine-grained concurrency
- **B-Trees**: `btree_t` provides thread-safe B-tree operations with tree-level locking and kobj pool integration for node allocation
- **Lock Hierarchy**: Strict ordering prevents deadlocks: global subsystem locks → owner object locks → data structure locks
- **Error Handling**: All operations return error codes for proper failure handling and resource cleanup
**Kernel Responsibilities:**
- Thread management and scheduling (threads are the fundamental unit)
- Process management (thread groups with shared memory/namespace)
- Process group management for POSIX job control
- Virtual memory management and protection
- Inter-process communication through filesystem operations
- `//` namespace filesystem with service registration
- Capability-based security enforcement
- Hardware abstraction and interrupt handling
- POSIX signal handling and job control

**Initialization Memory Reclamation Design:**

The kernel appears designed with a sophisticated memory reclamation system for initialization code that follows TDD principles:

```
TDD-Driven Kernel Memory Layout:
┌─────────────────────────────────────┐
│  .init.text (Reclaimable)          │
│  __init_start symbol               │  ← Marks beginning of reclaimable region
│  - Subsystem initializers          │  ← mem_init(), proc_init(), etc. (tested first)
│  - Bootstrap and setup code        │  ← bootstrap.S, bootstrap.c (tested first)
│  - Boot sequence code              │  ← boot.s, boot.c (tested first, if defined)
│  - One-time initialization data    │  ← Linker script grouped (tested first)
│  - Early hardware setup            │  ← Hardware abstraction (tested first)
│  __init_end symbol                 │  ← Marks end of reclaimable region
├─────────────────────────────────────┤
│  .text (Permanent)                 │  ← Runtime kernel code (tested first)
│  - Runtime functions               │  ← Memory management (tested first)
│  - System call handlers            │  ← Process management (tested first)
│  - Interrupt handlers              │  ← IPC operations (tested first)
├─────────────────────────────────────┤
│  .data/.bss (Permanent)            │  ← Runtime data structures (tested first)
│  - Global variables                │  ← Kernel objects (tested first)
│  - Data structures                 │  ← Statistics counters (tested first)
└─────────────────────────────────────┘

TDD Memory Reclamation Process:
1. Write tests defining expected boot behavior and memory reclamation
2. Boot → Execute bootstrap code (using .init.text, tested first)
3. Initialize console subsystem first (highest priority, tested first)
4. Initialize minimum required dependencies (memory subsystem, tested first)
5. Initialize remaining subsystems with console output available (tested first)
6. Transfer control to scheduler (tested first)
7. Free memory between __init_start and __init_end (tested first)
8. Validate reclaimed pages available for runtime allocation (tested first)
```

**TDD Console Initialization Priority:**
- **Tests First**: Console subsystem tests written before implementation defining initialization priority
- **Highest Priority**: Console subsystem initialized first among all kernel subsystems (validated by tests)
- **Dependency Handling**: Tests define that if console depends on memory subsystem, initialize memory first, then console immediately
- **Early Debug Output**: Tests validate console available for capturing initialization messages from all subsequent subsystems
- **Fallback Mechanisms**: Tests define alternative console methods attempted if primary console initialization fails

**TDD Linker Script Integration:**
- **Test-Driven Sections**: Tests written first defining expected memory layout and section organization
- **Section Attributes**: Tests validate initialization functions use `__attribute__((section(".init.text")))`
- **Bootstrap Inclusion**: Tests validate bootstrap sources (bootstrap.S, bootstrap.c, boot.s, boot.c) placed in `.init.text` when defined
- **Symbol Boundaries**: Tests validate linker provides `__init_start` at the beginning and `__init_end` at the end of `.init.text` section
- **Memory Segregation**: Tests validate strict separation between initialization and runtime code
- **Runtime Safety**: Tests validate no runtime functions placed in reclaimable sections

### Service Architecture

User-space services register in the `//` namespace and manage their own hierarchical interfaces:

```
// Namespace Structure:
//proc/           - Process information (kernel-provided)
//kern/           - Kernel information (kernel-provided)
//scsi/           - SCSI subsystem (unified block device interface)
  ├── host0/     - SCSI host adapter 0
  ├── 0:0:0:0/ - SCSI device (host:channel:target:lun)
    ├── block  - Block device interface
    ├── part1  - Partition 1
    ├── part2  - Partition 2
    ├── model  - Device model (SCSI INQUIRY data)
    ├── vendor - Vendor information (SCSI INQUIRY data)
    ├── serial - Serial number (SCSI INQUIRY data)
    ├── wwn    - World Wide Name (if available)
    ├── scsi_id - SCSI identifier string
    └── primary -> part1  - Symlink to primary partition
  └── 0:0:1:0/ - Second SCSI device
//block/          - Legacy block device compatibility layer
  ├── sda -> ../scsi/host0/0:0:0:0/block  - Compatibility symlink
  └── sdb -> ../scsi/host0/0:0:1:0/block  - Compatibility symlink
//net/device/     - Network device services
  ├── eth0/      - Ethernet interface
  ├── stats    - Interface statistics
  ├── config   - Configuration interface
  └── link     - Link status
  └── wlan0/     - Wireless interface
  ├── stats    - Interface statistics
  ├── config   - Configuration interface
  └── signal   - Signal strength
//net/ipv4/       - IPv4 protocol endpoints
  ├── 192.168.1.100/
  ├── 80/      - HTTP server endpoint
  ├── 443/     - HTTPS server endpoint
  └── 22/      - SSH server endpoint
  └── 0.0.0.0/   - Wildcard address
  └── 53/      - DNS server endpoint
//net/ipv6/       - IPv6 protocol endpoints
  ├── ::1/       - Loopback address
  └── 22/      - SSH server endpoint
  └── ::/        - Wildcard address
  └── 53/      - DNS server endpoint
//net/unix/       - Unix domain sockets
  ├── stream/    - Stream sockets
  └── dgram/     - Datagram sockets
//rdma/           - RDMA device services
  ├── mlx5_0/    - Mellanox ConnectX RDMA device
  ├── verbs    - RDMA verbs interface
  ├── stats    - Performance statistics
  └── config   - Device configuration
  └── roce0/     - RoCE (RDMA over Converged Ethernet) device
//tty/            - Terminal and character stream services
  ├── console    - System console (character stream)
  ├── tty1       - Virtual terminal 1 (character stream)
  ├── tty2       - Virtual terminal 2 (character stream)
  └── pts/       - Pseudo-terminal slaves
  ├── 0        - PTY slave 0 (character stream)
  └── 1        - PTY slave 1 (character stream)
//input/          - Input device services
  ├── keyboard0  - USB keyboard (character stream)
  ├── mouse0     - USB mouse (character stream)
  ├── touchpad0  - Touchpad device (character stream)
  └── event/     - Raw input events
  ├── kbd      - Keyboard events
  └── mouse    - Mouse events
//usb/            - USB device services
  ├── bus0/      - USB bus 0
  ├── 001      - USB device 001 (keyboard)
  ├── 002      - USB device 002 (mouse)
  └── hub      - USB hub information
  └── bus1/      - USB bus 1
//swap/           - Swap services
//fs/             - Filesystem services
```

### Multi-Architecture Support

The system is designed for multi-architecture deployment:

```
Target Architectures:
├── x86_64       - Primary development platform
├── ARM64        - Mobile and embedded systems
└── RISC-V       - Open hardware platforms

Build System:
├── Cross-compilation support
├── Architecture-specific optimizations
└── Shared kernel core with arch-specific HAL

**Forward Declaration Prohibition Design:**

The build system appears designed to enforce code quality through strict header usage requirements:

```
Code Quality Enforcement:
┌─────────────────────────────────────┐
│  Source File Analysis               │
│  - Scan for custom forward decls   │
│  - Check against existing headers   │
│  - Generate compiler warnings/errors│
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Header Dependency Validation      │
│  - Verify proper #include usage    │
│  - Prevent duplicate declarations   │
│  - Ensure maintainable interfaces  │
└─────────────────────────────────────┘
```

**Forward Declaration Prevention Strategy:**
- **Interface Centralization**: All function and type declarations must exist in appropriate header files
- **Duplicate Prevention**: Custom forward declarations create maintenance issues when interfaces change
- **Compiler Integration**: Build system uses compiler warnings/errors to detect custom forward declarations
- **Header Enforcement**: Source files must include proper headers instead of declaring interfaces locally
- **Maintainability**: Single source of truth for all interface declarations prevents inconsistencies
```

## Components and Interfaces

### 1. Kernel Core

**Thread and Process Manager:**
- Implements POSIX threading model: threads → processes → process groups
- Threads are the fundamental scheduling unit with thread IDs (tids)
- Processes are collections of threads sharing memory space and namespace
- **Process tree stored as B+ tree** for efficient PID lookups and range queries
- Process groups provide POSIX job control and signal handling
- Provides `rfork()` system call for namespace and thread/process customization
- All processes are container-native with namespace, capability, and resource limit support
- OCI-compliant containers created through `rfork()` with appropriate namespace and capability restrictions
- **Multi-policy scheduler:** Supports SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH, SCHED_IDLE, and SCHED_DEADLINE
- **Real-time scheduling:** Deterministic real-time scheduling with priority inheritance and priority ceiling protocols
- **IPC-aware scheduling:** Threads performing blocking IPC operations yield remaining quantum to target process
- Container isolation achieved through existing process isolation mechanisms

**Memory Manager:**
- **3-Level Page Table Allocator**: Architecture-specific page allocator with L1 (1GB regions), L2 (16MB regions), and L3 (256KB regions with bitmaps) for efficient memory management
- **Dynamic Memory Detection**: Bootloader-agnostic memory detection supporting UEFI, Multiboot, and U-Boot with automatic page table sizing based on available memory
- **Bitmap-Based Page Tracking**: 2-bit page states (FREE, ALLOCATED, INVALID, RESERVED) with O(1) single page allocation using find_first_zero() operations
- **Bad Memory Handling**: Support for marking memory regions as invalid/unallocatable for handling bad memory and memory holes
- Virtual memory with per-process address spaces
- Memory protection between processes and kernel
- Page mapping support for zero-copy IPC operations
- Integration with userland swap services

## TDD Component Architecture

### Test-Driven Kernel Components

The FMI/OS kernel components are designed and implemented using comprehensive TDD methodology where tests define expected behavior before any implementation:

**TDD Thread and Process Manager:**
- Tests written first defining POSIX threading model behavior: threads → processes → process groups
- Tests validate threads as fundamental scheduling unit with thread IDs (tids)
- Tests define processes as collections of threads sharing memory space and namespace
- **TDD Process Tree**: Tests written first for B+ tree operations before implementation, validating efficient PID lookups and range queries
- Tests define process groups providing POSIX job control and signal handling
- Tests validate `rfork()` system call behavior for namespace and thread/process customization
- Tests ensure all processes are container-native with namespace, capability, and resource limit support
- Tests validate OCI-compliant container creation through `rfork()` with appropriate restrictions
- **TDD Multi-policy Scheduler**: Tests written first defining SCHED_NORMAL, SCHED_FIFO, SCHED_RR, SCHED_BATCH, SCHED_IDLE, and SCHED_DEADLINE behavior
- **TDD Real-time Scheduling**: Tests define deterministic real-time scheduling with priority inheritance and priority ceiling protocols
- **TDD IPC-aware Scheduling**: Tests validate threads performing blocking IPC operations yield remaining quantum to target process

**TDD Memory Manager:**
- **TDD 3-Level Page Table Allocator**: Tests written first defining L1 (1GB regions), L2 (16MB regions), and L3 (256KB regions with bitmaps) behavior
- **TDD Dynamic Memory Detection**: Tests define bootloader-agnostic memory detection supporting UEFI, Multiboot, and U-Boot with automatic page table sizing
- **TDD Bitmap-Based Page Tracking**: Tests validate 2-bit page states (FREE, ALLOCATED, INVALID, RESERVED) with O(1) single page allocation using find_first_zero() operations
- **TDD Bad Memory Handling**: Tests define support for marking memory regions as invalid/unallocatable for handling bad memory and memory holes
- Tests validate virtual memory with per-process address spaces
- Tests ensure memory protection between processes and kernel
- Tests validate page mapping support for zero-copy IPC operations
- Tests define integration with userland swap services

**TDD Memory Detection Architecture:**

The memory detection system follows TDD principles with tests defining bootloader-agnostic memory discovery:

```
TDD Bootloader Memory Detection Flow:
┌─────────────────────────────────────┐
│  Bootstrap Code (Tested First)      │
│  - Tests define boot protocol detection │  ← UEFI, Multiboot, U-Boot
│  - Tests validate bootloader data preservation │  ← Memory maps, system tables
│  - Tests define memory_detect_init() behavior │
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Memory Detection System (Tested First) │
│  - Tests define bootloader memory map parsing │  ← Protocol-specific parsing
│  - Tests validate unified format conversion │  ← memory_region_t structures
│  - Tests define totals and statistics calculation │  ← Total, usable, highest address
│  - Tests validate thread-safe access │  ← Spinlock protection
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Page Allocator Initialization (Tested First) │
│  - Tests define required table size calculation │  ← Based on total memory
│  - Tests validate 3-level page structure creation │  ← L1/L2/L3 entries
│  - Tests define bitmap initialization per region │  ← 2-bit page states
│  - Tests validate bad region marking as invalid │  ← Permanent exclusion
└─────────────────────────────────────┘
```

**TDD Memory Region Types** (defined by tests first):
- `MEMORY_AVAILABLE` - Tests validate usable for kernel allocation
- `MEMORY_RESERVED` - Tests define reserved by system/firmware behavior
- `MEMORY_ACPI_RECLAIMABLE` - Tests validate ACPI tables (reclaimable after parsing)
- `MEMORY_ACPI_NVS` - Tests define ACPI non-volatile storage behavior
- `MEMORY_BAD` - Tests validate defective memory region handling
- `MEMORY_HOLE` - Tests define memory hole behavior in address space

**TDD Dynamic Page Table Sizing** (calculated by tests first):
Tests define page allocator structure size calculation based on detected memory:
- L3 entries needed = total_pages / 64 (pages per L3) - validated by tests
- L2 entries needed = L3_count / 64 (L3 entries per L2) - validated by tests
- L1 entries needed = L2_count / 64 (L2 entries per L1) - validated by tests
- Maximum support: 64 L1 entries (up to 64GB memory) - validated by tests

**TDD Boot Command Line Parameter System:**

The boot command line parameter system follows TDD with tests defining parameter capture and parsing:

```
TDD Boot Parameter Processing Flow:
┌─────────────────────────────────────┐
│  Bootstrap Code (Tested First)      │
│  - Tests define boot protocol detection │  ← UEFI, Multiboot, U-Boot
│  - Tests validate command line location │  ← Protocol-specific locations
│  - Tests define cmdline_capture_init() behavior │
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Command Line Capture System (Tested First) │
│  - Tests validate cmdline copy to kernel memory │  ← Safe from bootloader changes
│  - Tests define key=value parameter parsing │  ← console=ttyS0,115200
│  - Tests validate boolean flag handling │  ← debug, quiet, verbose
│  - Tests define thread-safe access │  ← Spinlock protection
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Parameter Access Interface (Tested First) │
│  - Tests validate cmdline_get_param() lookups │  ← By parameter name
│  - Tests define console parameter parsing │  ← For console selection
│  - Tests validate KFS exposure via //kern/cmdline │  ← Runtime inspection
│  - Tests define integration with subsystems │  ← Console, logging, etc.
└─────────────────────────────────────┘
```

**TDD Command Line Parameter Types** (defined by tests first):
- `console=ttyS0,115200` - Tests validate console device and baud rate specification
- `debug` - Tests define debug output and verbose logging enablement
- `quiet` - Tests validate suppression of non-essential kernel messages
- `root=/dev/sda1` - Tests define root filesystem specification
- `init=/sbin/init` - Tests validate initial process specification
- `mem=512M` - Tests define memory limit override behavior

**TDD Parameter Parsing Features** (validated by tests first):
- Key-value pairs: `parameter=value` - tests validate parsing
- Boolean flags: `debug`, `quiet`, `verbose` - tests validate recognition
- Multiple values: `console=ttyS0 console=tty0` - tests validate handling
- Quoted values: `init="/custom/init script"` - tests validate parsing
- Maximum length: 4096 characters total - tests validate limits

**TDD PVH ELF Boot Protocol Architecture:**

The PVH ELF boot protocol follows TDD principles with tests defining virtualization-optimized boot sequences and 32-bit to 64-bit mode transitions:

```
TDD PVH ELF Boot Flow:
┌─────────────────────────────────────┐
│  ELF Header Generation (Tested First) │
│  - Tests define ELF64 executable structure │  ← Proper ELF header format
│  - Tests validate XEN_ELFNOTE_PHYS32_ENTRY │  ← 32-bit entry point note
│  - Tests validate XEN_ELFNOTE_ENTRY note │  ← 64-bit entry point note
│  - Tests define note section placement │  ← ELF notes section
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  32-bit Entry Point (Tested First)  │
│  - Tests define PVH start info detection │  ← EBX register validation
│  - Tests validate CPU feature detection │  ← CPUID for long mode, PAE
│  - Tests define page table setup │  ← PML4, PDP, PD with 2MB pages
│  - Tests validate GDT setup │  ← 64-bit code/data segments
│  - Tests define mode transition sequence │  ← PAE, EFER, paging enable
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  64-bit Entry Point (Tested First)  │
│  - Tests validate existing 64-bit mode │  ← EFER MSR validation
│  - Tests define PVH start info parsing │  ← Memory map extraction
│  - Tests validate debug marker output │  ← Serial console communication
│  - Tests define boot info structure setup │  ← Kernel parameter passing
└─────────────────────────────────────┘
```

**TDD PVH Boot Protocol Features** (validated by tests first):
- **Dual Entry Points**: Tests validate both 32-bit (`_pvh_start32`) and 64-bit (`_pvh_start64`) entry points
- **ELF Note Integration**: Tests define proper XEN_ELFNOTE placement in ELF notes section
- **Mode Transition**: Tests validate complete 32-bit to 64-bit transition including CPU feature detection
- **Page Table Setup**: Tests define minimal page table structure (PML4→PDP→PD with 2MB pages)
- **GDT Configuration**: Tests validate temporary GDT with 64-bit segments for mode transition
- **PVH Start Info**: Tests define parsing of hypervisor-provided boot information structure
- **Hypervisor Compatibility**: Tests validate compatibility with Xen hypervisor and QEMU PVH boot
- **Debug Integration**: Tests define serial console output during boot process for debugging

**TDD 32-bit to 64-bit Mode Transition Sequence** (defined by tests first):
1. **CPU Feature Detection**: Tests validate CPUID checks for long mode and PAE support
2. **Page Table Creation**: Tests define minimal paging structure setup for 64-bit mode
3. **GDT Setup**: Tests validate creation and loading of 64-bit Global Descriptor Table
4. **PAE Enable**: Tests define Physical Address Extension enabling in CR4 register
5. **Long Mode Enable**: Tests validate LME bit setting in EFER MSR (Extended Feature Enable Register)
6. **Paging Enable**: Tests define PG bit setting in CR0 register to activate paging
7. **Far Jump**: Tests validate far jump to 64-bit code segment to complete transition
8. **Mode Validation**: Tests define verification that long mode is active and properly configured

**TDD Boot Protocol Integration:**

The boot protocol support follows TDD principles with tests defining comprehensive bootloader compatibility and proper mode transitions:

```
TDD Boot Protocol Flow:
┌─────────────────────────────────────┐
│  Header Generation (Tested First)   │
│  - Tests validate Multiboot v2 header │  ← Magic 0x36d76289, tags
│  - Tests validate PVH ELF headers    │  ← ELF notes and entry points
│  - Tests define header placement     │  ← v2: first 32KB, PVH: ELF sections
│  - Tests validate linker integration │  ← Proper section placement
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Boot Protocol Detection (Tested First) │
│  - Tests define magic number validation │  ← EAX register checks
│  - Tests validate info structure access │  ← EBX register validation
│  - Tests define protocol selection logic │  ← Multiboot v2 vs PVH ELF
│  - Tests validate debug marker output │  ← Boot progress indication
└─────────────────────────────────────┘
  ↓
┌─────────────────────────────────────┐
│  Mode Transition (Tested First)     │
│  - Tests define 32-bit entry handling │  ← PVH ELF and Multiboot v2 32-bit
│  - Tests validate 64-bit mode validation │  ← Multiboot v2 64-bit path
│  - Tests define transition sequence │  ← Complete mode switch
│  - Tests validate final kernel entry │  ← Common entry point
└─────────────────────────────────────┘
```

**TDD Boot Protocol Features** (validated by tests first):
- **Multi-Protocol Support**: Tests validate Multiboot v2 and PVH ELF protocol support for x86_64
- **Architecture-Specific Protocols**: Tests validate UEFI and U-Boot support for ARM64 and RISC-V
- **Mode Transition Handling**: Tests define proper 32-bit to 64-bit transitions when needed
- **Header Validation**: Tests define proper magic numbers and structure validation
- **Protocol Detection**: Tests validate runtime detection of which protocol was used
- **Info Structure Parsing**: Tests validate extraction of memory maps and boot parameters
- **Debug Integration**: Tests define serial console output for boot progress tracking
- **Bootloader Compatibility**: Tests validate compatibility with GRUB, UEFI, and U-Boot configurations
- **Build System Validation**: Tests validate that configure script provides appropriate protocol support per architecture

**IPC System:**
- Filesystem-based communication model
- All IPC operations use standard filesystem calls (read, write, open, close)
- `page_map()` function for efficient memory sharing
- inotify/dnotify-style asynchronous notifications
- Security through filesystem-style permissions
- **IPC Scheduling Optimization:** When a process performs blocking I/O to another process, it yields its remaining CPU quantum to the target process for improved responsiveness
- **RDMA Support:** Remote Direct Memory Access for high-performance network communication with zero-copy semantics
- **Fast Userland Interfaces:** Kernel functions mapped into userland process space for ultra-low latency IPC operations, similar to Linux futexes

### 2. Namespace Filesystem (`//`)

**Core Functionality:**
- Complete filesystem implementation supporting standard operations including symlinks and character streams
- Hierarchical service organization with symlink support for flexible service aliasing
- Character stream support for terminals, pipes, and other streaming I/O
- Service registration and discovery
- Dependency notification system
- Access control through capability system

**VFS-IPC Integration Architecture:**

The namespace filesystem appears designed with a unified request routing system that seamlessly integrates kernel-level VFS interfaces with userland service IPC:

```
Application Request (open/read/write/close)
  ↓
VFS Request Router
  ↓
Path Analysis & Handler Lookup
  ↓
┌─────────────────────┬─────────────────────┐
│   Kernel Handler    │   Userland Handler  │
│   (//kern/*, etc.)  │   (//scsi/*, etc.)  │
│                     │                     │
│   Direct VFS Call   │   IPC Forwarding    │
│   ↓                 │   ↓                 │
│   Kernel Subsystem  │   Userland Service  │
│   Response          │   Response          │
└─────────────────────┴─────────────────────┘
  ↓
Unified Response to Application
```

**Request Routing Logic:**
- **Path Registry**: The VFS router maintains a dynamic registry mapping path prefixes to handlers
- **Kernel Paths**: Paths like `//kern/mem/stats/physical` route directly to kernel subsystem VFS interfaces
- **Userland Paths**: Paths like `//scsi/host0/0:0:0:0/block` route through IPC to registered userland services
- **Transparent Operation**: Applications use identical filesystem APIs regardless of handler location
- **Dynamic Registration**: Services can register/deregister paths, updating the routing table
- **Error Handling**: Service failures result in standard filesystem error codes (ENOENT, EIO)

**IPC Filesystem Operation Forwarding:**
- **Parameter Marshalling**: Filesystem operation parameters (path, flags, data) are marshalled for IPC transmission
- **Response Handling**: Userland service responses are unmarshalled and returned as standard filesystem results
- **Access Control**: Capability checks occur at the VFS router level before forwarding to handlers
- **Atomic Operations**: Multi-step operations can coordinate between kernel and userland handlers

**Service Registration Process:**
```
1. Service starts and requests namespace registration
2. Kernel allocates namespace path (e.g., //scsi/host0/0:0:0:0 for SCSI devices)
3. Service populates its hierarchy with interfaces and metadata
4. Other services can discover and depend on registered services
5. Kernel provides notifications when dependencies become available
```

**Enhanced VFS Statistics Interface:**

The kernel provides fine-grained statistics through structured sub-directories with a consistent header/data format that appears designed for programmatic parsing and forward compatibility:

```
//kern/<subsystem>/stats/<category>
```

**Statistics Format Design:**
- **Header/Data Structure**: Each statistics file uses a two-line format where the first line contains field names and the second line contains corresponding values
- **Forward Compatibility**: New fields can be added to the end without breaking existing parsers
- **Atomic Consistency**: Statistics appear updated atomically to prevent torn reads during concurrent access
- **Hierarchical Organization**: Statistics seem organized by subsystem and category for logical grouping

**Memory Subsystem Statistics:**
```
//kern/mem/stats/physical    - total free used cached
//kern/mem/stats/virtual     - total allocated reserved
//kern/mem/stats/system      - allocations faults swapping
//kern/mem/stats/pools       - pools utilization fragmentation
```

**Process Subsystem Statistics:**
```
//kern/proc/stats/counts     - total running sleeping zombie
//kern/proc/stats/creation   - forks exits failures
//kern/proc/stats/scheduling - switches preemptions
```

**Thread Subsystem Statistics:**
```
//kern/thread/stats/counts     - total running blocked
//kern/thread/stats/scheduling - quantum priority_changes
```

**IPC Subsystem Statistics:**
```
//kern/ipc/stats/messages - sent received queued
//kern/ipc/stats/channels - active created destroyed
```

**Namespace Subsystem Statistics:**
```
//kern/ns/stats/entries    - total services directories
//kern/ns/stats/operations - lookups registrations bindings
```

This design appears to enable userland tooling to parse statistics programmatically while allowing the kernel to add new fields without breaking existing monitoring tools.

### 3. User-Space Services

**SCSI Subsystem and Block Device Services:**
- All block devices are accessed through the unified SCSI interface in `//scsi/` namespace
- SCSI host adapters register as `//scsi/hostN/` where N is the host number
- Individual devices appear as `//scsi/hostN/H:C:T:L/` following SCSI addressing (Host:Channel:Target:LUN)
- Each SCSI device provides comprehensive metadata from SCSI INQUIRY commands:
  - `model` - Device model string from SCSI INQUIRY
  - `vendor` - Vendor identification from SCSI INQUIRY
  - `serial` - Unit serial number from SCSI INQUIRY
  - `wwn` - World Wide Name for devices that support it
  - `scsi_id` - Complete SCSI identifier string
- Legacy ATA/SATA devices are presented through SCSI translation layer (SAT - SCSI-ATA Translation)
- NVMe devices are presented through SCSI translation for unified interface
- Partitions appear as `partN` entries within each device directory
- Legacy `//block/` namespace provides compatibility symlinks to SCSI devices (e.g., `sda -> ../scsi/host0/0:0:0:0/block`)
- Support for various storage protocols unified under SCSI interface

**Network Services:**
- TCP/IP stack implementation in user space
- Custom network protocol for IPC-to-network mapping
- 9P and FUSE interfaces for distributed computing
- **RDMA Services:** High-performance RDMA support for InfiniBand, RoCE, and iWARP
- RDMA verbs interface accessible through `//rdma/` namespace
- Zero-copy networking with direct memory access capabilities

**Filesystem Services:**
- Various filesystem implementations (ext4, NTFS, etc.)
- Mount through namespace binding operations
- Integration with SCSI subsystem dependencies

**Terminal and Character Stream Services:**
- Register in `//tty/` namespace
- Provide console, virtual terminals, and pseudo-terminals
- Support POSIX terminal semantics and control
- Integration with job control and process groups

**USB and Input Device Services:**
- USB host controller drivers register in `//usb/` namespace
- USB device enumeration and management
- HID (Human Interface Device) support for keyboards, mice, and touchpads
- Input devices register in `//input/` namespace with both cooked and raw event interfaces
- Support for common USB device classes (HID, mass storage, network adapters)
- Hot-plug support with automatic service registration/deregistration
**Swap Service (`swapd`):**
- Registration in `//swap/` namespace
- Kernel integration for memory page swapping

### 4. Development and Testing Infrastructure

**Service Libraries:**
- Multi-language support for service development
- D-Bus-like functionality integrated into `//` namespace
- Consistent metadata management
- Templates for common service types

**Testing and Monitoring Tools:**
- Asynchronous operation testing in `//` namespace
- Real-time service interaction monitoring
- Load testing and failure simulation
- Integration with capability system for authorized monitoring

**Development and Debugging Infrastructure:**
- QEMU integration for virtual machine deployment and testing
- GDB server support within QEMU for kernel debugging
- Kernel symbol table generation for source-level debugging
- Remote debugging capabilities for step-through kernel debugging
- Automated test deployment in virtualized environments

## Data Models

### Process Model

```c
struct kobj_s {
  enum kobj_type type;          // THREAD, PROCESS, PGROUP, SERVICE, etc.
  atomic_t ref_count;           // Reference counting
  struct spinlock_s *obj_lock;       // Object-level locking
  struct kobj_pool *pool;       // Pool this object belongs to
  uint32_t magic;               // Magic number for validation
  struct list_head pool_list;   // List entry for pool management
};
typedef struct kobj_s kobj_t;

struct thread_s {
  kobj_t kobj;                  // Kernel object header
  tid_t tid;                    // Thread ID
  struct process_s *process;    // Parent process
  struct thread_context *ctx;  // CPU context and registers
  struct thread_state state;   // Running, blocked, etc.
  int sched_policy;            // SCHED_NORMAL, SCHED_FIFO, SCHED_RR, etc.
  int priority;                // Thread priority (0-99 for RT, nice for normal)
  int nice_value;              // Nice value for SCHED_NORMAL threads
  struct sched_deadline_params *dl_params; // Deadline scheduling parameters
  uint64_t runtime;            // CPU time consumed
  uint64_t deadline;           // Absolute deadline (for SCHED_DEADLINE)
  uint64_t period;             // Period (for SCHED_DEADLINE)
};
typedef struct thread_s thread_t;

struct process_s {
  kobj_t kobj;                  // Kernel object header
  pid_t pid;                    // Process ID
  struct process_group_s *pgrp; // Parent process group
  struct capability_set *caps;
  struct namespace *ns;
  struct memory_space *mm;
  struct thread_list *threads; // List of threads in this process
  struct service_registrations *services;
  struct resource_limits *limits; // Resource limits for container support
};
typedef struct process_s process_t;

struct process_group_s {
  kobj_t kobj;                  // Kernel object header
  pgid_t pgid;                  // Process group ID
  struct session_s *session;   // Parent session (if any)
  struct process_list *processes; // List of processes in this group
  struct capability_set *group_caps; // Group-level capabilities
};
typedef struct process_group_s process_group_t;
```

### Process Tree Management

```c
struct process_btree_node_s {
  kobj_t kobj;                  // Kernel object header
  bool is_leaf;                 // True if this is a leaf node
  int key_count;                // Number of keys in this node
  pid_t keys[BTREE_ORDER - 1];  // Process IDs (keys)
  union {
    process_t *processes[BTREE_ORDER]; // Leaf: pointers to processes
    struct process_btree_node_s *children[BTREE_ORDER]; // Internal: child nodes
  };
  struct process_btree_node_s *parent; // Parent node
  struct process_btree_node_s *next;   // Next leaf (for range queries)
};
typedef struct process_btree_node_s process_btree_node_t;

struct process_btree_s {
  process_btree_node_t *root;   // Root of the B+ tree
  int height;                   // Height of the tree
  size_t process_count;         // Total number of processes
  struct spinlock_s *tree_lock;      // Protects tree structure
};
typedef struct process_btree_s process_btree_t;
```

```c
struct kobj_pool {
  enum kobj_type type;          // Type of objects in this pool
  size_t obj_size;              // Size of each object
  size_t pool_size;             // Total number of objects in pool
  size_t free_count;            // Number of free objects
  struct list_head free_list;   // List of free objects
  struct list_head active_list; // List of active objects
  struct spinlock pool_lock;    // Protects pool operations
  void *pool_memory;            // Pre-allocated memory for objects
  struct slab_cache *slab;      // Slab allocator for this object type
};

enum kobj_type {
  KOBJ_THREAD = 1,
  KOBJ_PROCESS = 2,
  KOBJ_PGROUP = 3,
  KOBJ_SERVICE = 4,
  KOBJ_NAMESPACE = 5,
  KOBJ_RDMA_MR = 6,
  KOBJ_RDMA_QP = 7,
  KOBJ_CHARSTREAM = 8,
  KOBJ_SCSI_HOST = 9,
  KOBJ_SCSI_DEVICE = 10,
  KOBJ_PARTITION = 11
};
```

```c
struct sched_deadline_params {
  uint64_t runtime;             // Maximum runtime per period
  uint64_t deadline;            // Relative deadline
  uint64_t period;              // Period length
  uint64_t flags;               // Scheduling flags
};

struct scheduler_stats {
  uint64_t context_switches;    // Total context switches
  uint64_t preemptions;         // Preemption count
  uint64_t deadline_misses;     // Deadline scheduling misses
  uint64_t rt_throttling;       // Real-time throttling events
  struct per_cpu_stats *cpu_stats; // Per-CPU scheduling statistics
};

enum sched_policy {
  SCHED_NORMAL = 0,             // Standard time-sharing
  SCHED_FIFO = 1,              // First-in, first-out real-time
  SCHED_RR = 2,                // Round-robin real-time
  SCHED_BATCH = 3,             // Batch processing
  SCHED_IDLE = 5,              // Very low priority
  SCHED_DEADLINE = 6           // Deadline scheduling
};
```

### SCSI Subsystem Data Models

```c
struct scsi_host_s {
  kobj_t kobj;                  // Kernel object header
  uint32_t host_no;             // SCSI host number
  char *host_name;              // Host adapter name
  struct scsi_host_template *hostt; // Host template operations
  struct capability_set *caps;  // Host capabilities
  struct device_list *devices;  // List of SCSI devices on this host
  struct spinlock_s *host_lock;      // Protects host state
  atomic_t ref_count;           // Reference counting
};
typedef struct scsi_host_s scsi_host_t;

struct scsi_device_s {
  kobj_t kobj;                  // Kernel object header
  scsi_host_t *host;            // Parent SCSI host
  uint32_t channel;             // SCSI channel number
  uint32_t id;                  // SCSI target ID
  uint32_t lun;                 // SCSI logical unit number
  char scsi_address[16];        // "H:C:T:L" address string

  // SCSI INQUIRY data
  char vendor[9];               // Vendor identification (8 chars + null)
  char model[17];               // Product identification (16 chars + null)
  char revision[5];             // Product revision (4 chars + null)
  char serial[256];             // Unit serial number (variable length)
  uint64_t wwn;                 // World Wide Name (if available)

  // Device characteristics
  enum scsi_device_type type;   // Direct access, sequential, etc.
  uint32_t sector_size;         // Logical block size
  uint64_t capacity;            // Device capacity in blocks
  bool removable;               // Removable media device
  bool write_protected;         // Write protection status

  // Translation layer info (for ATA/NVMe devices)
  enum device_protocol protocol; // SCSI_NATIVE, SCSI_ATA_SAT, SCSI_NVME_TRANS
  void *protocol_data;          // Protocol-specific data

  struct partition_table *partitions; // Partition information
  struct capability_set *access_caps;  // Access control capabilities
  struct spinlock_s *device_lock;    // Protects device state
  atomic_t ref_count;           // Reference counting
};
typedef struct scsi_device_s scsi_device_t;

enum scsi_device_type {
  SCSI_TYPE_DISK = 0x00,        // Direct-access block device
  SCSI_TYPE_TAPE = 0x01,        // Sequential-access device
  SCSI_TYPE_PRINTER = 0x02,     // Printer device
  SCSI_TYPE_PROCESSOR = 0x03,   // Processor device
  SCSI_TYPE_WORM = 0x04,        // Write-once read-multiple device
  SCSI_TYPE_ROM = 0x05,         // CD-ROM device
  SCSI_TYPE_SCANNER = 0x06,     // Scanner device
  SCSI_TYPE_MOD = 0x07,         // Optical memory device
  SCSI_TYPE_MEDIUM_CHANGER = 0x08, // Medium changer device
  SCSI_TYPE_COMM = 0x09,        // Communications device
  SCSI_TYPE_ENCLOSURE = 0x0d,   // Enclosure services device
  SCSI_TYPE_RBC = 0x0e,         // Simplified direct-access device
  SCSI_TYPE_OSD = 0x11,         // Object-based storage device
  SCSI_TYPE_NO_LUN = 0x7f       // Unknown or no device type
};

enum device_protocol {
  SCSI_NATIVE,                  // Native SCSI device
  SCSI_ATA_SAT,                 // ATA device via SCSI-ATA Translation
  SCSI_NVME_TRANS               // NVMe device via SCSI translation
};

struct partition_entry_s {
  kobj_t kobj;                  // Kernel object header
  uint32_t partition_number;    // Partition number (1-based)
  uint64_t start_sector;        // Starting sector
  uint64_t sector_count;        // Number of sectors
  uint8_t partition_type;       // Partition type code
  char *filesystem_type;        // Detected filesystem type
  bool bootable;                // Bootable flag
  struct capability_set *access_caps; // Partition access control
  struct spinlock_s *partition_lock; // Protects partition state
  atomic_t ref_count;           // Reference counting
};
typedef struct partition_entry_s partition_entry_t;

struct partition_table_s {
  enum partition_table_type type; // MBR, GPT, etc.
  uint32_t partition_count;     // Number of partitions
  partition_entry_t **partitions; // Array of partition entries
  struct spinlock_s *table_lock;     // Protects partition table
  atomic_t ref_count;           // Reference counting
};
typedef struct partition_table_s partition_table_t;

enum partition_table_type {
  PARTITION_TABLE_MBR,          // Master Boot Record
  PARTITION_TABLE_GPT,          // GUID Partition Table
  PARTITION_TABLE_BSD,          // BSD disklabel
  PARTITION_TABLE_SUN,          // Sun disklabel
  PARTITION_TABLE_NONE          // No partition table
};
```

### Service Registration

```c
struct service_entry_s {
  kobj_t kobj;                  // Kernel object header
  char *path;                   // e.g., "//scsi/host0/0:0:0:0"
  struct capability_set *caps;  // Service capabilities
  struct file_operations *ops;  // Filesystem operations
  struct metadata *meta;        // Service metadata
  struct dependency_list *deps; // Service dependencies
  struct spinlock_s *service_lock;   // Protects service entry
  atomic_t ref_count;           // Reference counting for safe cleanup
};
typedef struct service_entry_s service_entry_t;
```

### RDMA Memory Management

```c
struct rdma_memory_region_s {
  void *virtual_addr;           // Virtual address of memory region
  uint64_t physical_addr;       // Physical address for DMA
  size_t length;                // Size of memory region
  uint32_t lkey;                // Local key for access
  uint32_t rkey;                // Remote key for RDMA operations
  struct capability_set *access_caps; // Access control capabilities
  struct spinlock_s *region_lock;    // Protects memory region
  atomic_t ref_count;           // Reference counting
};
typedef struct rdma_memory_region_s rdma_memory_region_t;

struct rdma_queue_pair_s {
  uint32_t qp_num;              // Queue pair number
  enum qp_type type;            // RC, UC, UD queue pair types
  struct rdma_device *device;  // Associated RDMA device
  struct work_queue *send_wq;   // Send work queue
  struct work_queue *recv_wq;   // Receive work queue
  struct completion_queue *send_cq; // Send completion queue
  struct completion_queue *recv_cq; // Receive completion queue
  struct spinlock_s *qp_lock;        // Protects queue pair state
  atomic_t ref_count;           // Reference counting
};
typedef struct rdma_queue_pair_s rdma_queue_pair_t;
```

```c
struct rdma_memory_region {
  void *virtual_addr;            // Virtual address of memory region
  uint64_t physical_addr;        // Physical address for DMA
  size_t length;                 // Size of memory region
  uint32_t lkey;                 // Local key for access
  uint32_t rkey;                 // Remote key for RDMA operations
  struct capability_set *access_caps; // Access control capabilities
  struct spinlock_s *region_lock;     // Protects memory region
  atomic_t ref_count;           // Reference counting
};

struct rdma_queue_pair {
  uint32_t qp_num;              // Queue pair number
  enum qp_type type;            // RC, UC, UD queue pair types
  struct rdma_device *device;   // Associated RDMA device
  struct work_queue *send_wq;   // Send work queue
  struct work_queue *recv_wq;   // Receive work queue
  struct completion_queue *send_cq; // Send completion queue
  struct completion_queue *recv_cq; // Receive completion queue
  struct spinlock_s *qp_lock;        // Protects queue pair state
  atomic_t ref_count;           // Reference counting
};
```

```c
struct namespace_entry_s {
  char *name;
  enum entry_type type;          // FILE, DIRECTORY, SERVICE, SYMLINK, CHARSTREAM
  service_entry_t *service;      // If type == SERVICE
  char *symlink_target;          // If type == SYMLINK
  struct char_stream_s *stream;  // If type == CHARSTREAM
  struct capability_set *access_caps;
  struct notification_list *waiters;
  struct spinlock_s *entry_lock;      // Protects namespace entry
  atomic_t ref_count;           // Reference counting
};
typedef struct namespace_entry_s namespace_entry_t;

struct char_stream_s {
  struct stream_operations *ops; // Stream-specific operations (read, write, ioctl)
  struct buffer_queue *input_queue;  // Input buffer for character streams
  struct buffer_queue *output_queue; // Output buffer for character streams
  struct wait_queue *readers;    // Processes waiting to read
  struct wait_queue *writers;    // Processes waiting to write
  struct termios *tty_settings;  // Terminal settings (if applicable)
  struct spinlock_s *stream_lock;     // Protects stream state
  atomic_t ref_count;           // Reference counting
};
typedef struct char_stream_s char_stream_t;
```

### Fast Userland IPC Interface

```c
struct fast_ipc_region_s {
  kobj_t kobj;                  // Kernel object header
  void *userland_addr;          // Mapped address in userland
  void *kernel_addr;            // Kernel address of shared region
  size_t region_size;           // Size of mapped region
  struct capability_set *access_caps; // Access control
  struct fast_ipc_ops *ops;     // Fast IPC operations
};
typedef struct fast_ipc_region_s fast_ipc_region_t;

struct fast_ipc_ops_s {
  int (*fast_send)(void *msg, size_t len, pid_t target);
  int (*fast_recv)(void *buf, size_t len, pid_t *sender);
  int (*fast_notify)(pid_t target, uint32_t event);
  int (*fast_wait)(uint32_t *events, int timeout);
};
typedef struct fast_ipc_ops_s fast_ipc_ops_t;

// Fast userland mutex operations (similar to Linux futex)
struct fast_mutex_s {
  atomic_t value;               // Mutex state (0=unlocked, 1=locked)
  struct wait_queue *waiters;   // Queue of waiting threads
  pid_t owner;                  // Current owner process
  uint32_t flags;               // Mutex flags and options
};
typedef struct fast_mutex_s fast_mutex_t;
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

Let me analyze the acceptance criteria to determine which ones can be tested as properties.

<function_calls>
<invoke name="prework">
<parameter name="featureName">microkernel-os

Based on the prework analysis, I can now define the key correctness properties for FMI/OS:

### Property 1: Namespace Filesystem Operations
*For any* valid filesystem operation (read, write, open, close, ls, readlink, symlink, etc.) on the `//` namespace, the operation should complete successfully and return correct results according to standard filesystem semantics.
**Validates: Requirements 2.1, 2.2, 7.3, 7.4**

### Property 2: Service Registration and Discovery
*For any* service that registers itself in the `//` namespace, the service should become discoverable through standard filesystem operations and appear in the correct hierarchical location.
**Validates: Requirements 2.4, 2.6**

### Property 3: Service Binding Operations
*For any* valid service in the `//` namespace, binding/mounting the service into the `/` root namespace should succeed and make the service accessible through the mount point.
**Validates: Requirements 2.5**

### Property 4: Dependency Notification System
*For any* service waiting for a namespace entry, when that entry becomes available, the waiting service should receive notification reliably and promptly.
**Validates: Requirements 2.8, 2.9**

### Property 5: IPC Filesystem Model
*For any* inter-process communication operation, using standard filesystem calls (read, write, open, close) should successfully transmit data between processes.
**Validates: Requirements 8.1**

### Property 6: Memory Page Mapping
*For any* valid `page_map()` operation between processes, data should be transferred correctly without kernel copying, and memory should remain properly isolated.
**Validates: Requirements 8.3, 8.4**

### Property 7: Asynchronous Notifications
*For any* process using inotify/dnotify-style notifications, notification events should be delivered correctly when the monitored conditions occur.
**Validates: Requirements 8.2**

### Property 8: Access Control Enforcement
*For any* operation on namespace entries, IPC, or system information, unauthorized access attempts should be rejected according to the capability system rules.
**Validates: Requirements 2.7, 7.7, 8.5**

### Property 9: Process and Kernel Information Access
*For any* request for process information via `//proc/` or kernel information via `//kern/`, the correct data should be returned through standard filesystem operations.
**Validates: Requirements 7.1, 7.2, 7.5, 7.6**

### Property 28: Enhanced VFS Statistics Interface
*For any* statistics file in the `//kern/<subsystem>/stats/<category>` hierarchy, the file should return data in header/data format (field names on first line, values on second line) and support atomic reads for consistent snapshots.
**Validates: Requirements 7A.1, 7A.2, 7A.3, 7A.10, 7A.13, 7A.14**

### Property 29: VFS-IPC Request Routing
*For any* filesystem operation on the `//` namespace, the VFS router should correctly route the request to either kernel subsystem handlers or userland service handlers based on the path, and return consistent results regardless of handler location.
**Validates: Requirements 6A.1, 6A.2, 6A.3, 6A.4, 6A.5, 6A.9**

### Property 30: Initialization Memory Reclamation
*For any* kernel boot sequence, all subsystem initializers and bootstrap code should be grouped in the reclaimable memory segment, successfully initialize all subsystems, and the initialization memory should be freed after scheduler startup without affecting runtime operations.
**Validates: Requirements 17.44, 17.45, 17.46, 17.47, 17.48, 17.49, 17.50, 17.51, 17.52**

### Property 31: Console Initialization Priority
*For any* kernel boot sequence, the console subsystem should initialize with the highest reasonable priority (after only essential dependencies like memory management), and console output should be available for capturing debug messages from all subsequent subsystem initializations.
**Validates: Requirements 3A.1, 3A.2, 3A.3, 3A.4, 3A.5, 3A.6, 3A.7**

### Property 10: POSIX Thread and Process Compliance
*For any* POSIX thread or process operation, the behavior should conform to POSIX standards for threading and process management.
**Validates: Requirements 10.1, 10.2**

### Property 11: Rfork Namespace Customization
*For any* `rfork()` operation with namespace customization, the child process should inherit the correctly modified namespace and restricted capabilities.
**Validates: Requirements 10.3, 10.4**

### Property 12: OCI Container Compliance
*For any* container operation implemented through the `rfork()` system, the behavior should conform to OCI specifications for container runtime operations.
**Validates: Requirements 10.5, 10.6**

### Property 13: Memory Protection and Management
*For any* memory operation, processes should be properly isolated from each other and the kernel, and memory allocation/deallocation should work correctly.
**Validates: Requirements 1.2**

### Property 32: 3-Level Page Allocator Operations
*For any* page allocation or deallocation operation using the 3-level page table allocator, the operation should complete with O(1) performance for single pages, maintain correct bitmap states, and properly handle bad memory regions marked as invalid.
**Validates: Requirements 9B.1, 9B.2, 9B.3, 9B.4, 9B.5**

### Property 33: Dynamic Memory Detection and Sizing
*For any* bootloader memory configuration (UEFI, Multiboot, U-Boot), the memory detection system should correctly parse the memory map, calculate appropriate page table structure sizes, and provide unified access to memory region information regardless of bootloader type.
**Validates: Requirements 9C.1, 9C.2, 9C.3, 9C.4, 9C.5, 9C.6**

### Property 34: Boot Command Line Parameter Processing
*For any* bootloader command line configuration (UEFI, Multiboot, U-Boot), the command line parameter system should correctly capture, parse, and provide access to kernel parameters including console selection, debug flags, and key=value pairs.
**Validates: Requirements 9D.1, 9D.2, 9D.3, 9D.4, 9D.5, 9D.6, 9D.7, 9D.12**

### Property 35: PVH ELF Boot Protocol Support
*For any* PVH ELF boot scenario (32-bit or 64-bit entry), the kernel should correctly detect the PVH boot protocol, perform necessary mode transitions, parse PVH start info structure, and successfully initialize all kernel subsystems with proper debug output throughout the boot process.
**Validates: Requirements 19B.1, 19B.2, 19B.3, 19B.4, 19B.5, 19B.6, 19B.7, 19B.8, 19B.16, 19B.17, 19B.18, 19B.21, 19B.22, 19B.23**

### Property 14: Service Hierarchy Organization
*For any* registered service, it should appear in the correct hierarchical location within the `//` namespace and maintain its own filesystem structure.
**Validates: Requirements 2.3**

### Property 15: Core Kernel Services
*For any* core kernel service (IPC, memory management, namespace filesystem), the service should be available and function correctly.
**Validates: Requirements 1.1, 1.3, 1.5**

### Property 36: Architecture-Specific Boot Protocol Support
*For any* build configuration targeting x86_64 architecture, the build system should provide Multiboot v2 and PVH ELF protocol support, ensuring appropriate 64-bit compatible boot protocols are available for the target architecture.
**Validates: Requirements 22.1, 22.2, 22.3, 22.4, 22.5, 22.7, 22.9**

## Error Handling

### Kernel Error Handling

**Memory Management Errors:**
- Out of memory conditions trigger swap service activation
- Invalid memory access attempts result in process termination
- Memory leaks are prevented through automatic cleanup on process termination

**IPC Errors:**
- Invalid filesystem operations on IPC channels return appropriate error codes
- Broken IPC connections are detected and cleaned up automatically
- Security violations in IPC operations are logged and blocked

**Service Registration Errors:**
- Duplicate service registrations are rejected with clear error messages
- Invalid namespace paths are rejected during registration
- Service crashes trigger automatic cleanup of namespace entries

### User-Space Service Error Handling

**Service Isolation:**
- Service crashes are contained and don't affect other services or the kernel
- Failed services can be restarted without system reboot
- Service dependencies are tracked and dependent services are notified of failures

**Network Service Errors:**
- Network protocol errors are handled gracefully with appropriate error responses
- Connection failures trigger automatic retry mechanisms
- Invalid network requests are rejected with proper error codes

**Filesystem Service Errors:**
- Filesystem corruption is detected and reported
- I/O errors are propagated to applications with appropriate error codes
- Mount failures are handled gracefully with rollback mechanisms

## Testing Strategy

### Comprehensive TDD Testing Framework

FMI/OS employs a multi-layered Test-Driven Development approach with both unit testing and property-based testing for comprehensive coverage:

#### Test-First Development Process

**Unit Tests:**
- Written before implementation for each component and function
- Test specific examples and edge cases for each component
- Validate integration points between kernel and user-space services
- Test error conditions and recovery mechanisms
- Focus on concrete scenarios and boundary conditions

**Property-Based Tests:**
- Written before implementation to verify universal properties across all possible inputs
- Test system behavior under randomized conditions
- Validate correctness properties defined in this document
- Ensure comprehensive input coverage through automated generation

**QEMU Smoke Tests:**
- Written before implementation to test kernel components in actual kernel environment
- Validate console output, memory management, and synchronization primitives in kernel mode
- Test incremental functionality as each component is implemented
- Ensure kernel boots, runs test code, and shuts down cleanly

#### TDD Testing Categories

**Architecture-Specific TDD:**
- Tests written first for each target architecture (x86_64, ARM64, RISC-V)
- Implementation follows to satisfy architecture-specific requirements
- Ensures consistent behavior across all supported platforms

**Console Driver TDD:**
- Tests written first for each console driver type (serial, video, debug)
- Implementation follows to satisfy unified console interface requirements
- Validates proper character transmission and formatting across driver types

**Memory Management TDD:**
- Tests written first for slab allocator, page allocator, and memory detection
- Implementation follows to satisfy O(1) performance and thread safety requirements
- Validates proper memory allocation, deallocation, and error handling

**Synchronization Primitives TDD:**
- Tests written first for spinlocks, read-write locks, and atomic operations
- Implementation follows to satisfy thread safety and performance requirements
- Validates proper lock ordering and deadlock prevention

#### Dual Testing Environment Strategy

**Testing Klibc Environment:**
- Provides instrumentation and debugging capabilities for unit testing
- Wraps standard libc calls for testing kernel code in userspace
- Enables comprehensive testing of kernel logic without kernel environment

**Kernel Klibc Environment:**
- Provides actual kernel implementation for QEMU smoke testing
- Tests kernel components in their actual runtime environment
- Validates integration with hardware and bootloader interfaces

### Property-Based Testing Configuration

**Testing Framework:** The system uses QuickCheck-style property-based testing
- Minimum 100 iterations per property test
- Each property test references its corresponding design document property
- Test tag format: **Feature: microkernel-os, Property {number}: {property_text}**

**Key TDD Testing Areas:**

1. **Namespace Operations TDD:**
  - Tests written first for all filesystem operations on `//` namespace
  - Implementation follows to satisfy standard filesystem semantics
  - Validates concurrent access patterns and race conditions

2. **Service Registration TDD:**
  - Tests written first for service registration and discovery scenarios
  - Implementation follows to satisfy hierarchical namespace requirements
  - Validates service discovery and dependency resolution

3. **IPC TDD:**
  - Tests written first for all IPC communication patterns
  - Implementation follows to satisfy filesystem-based IPC requirements
  - Validates data integrity across process boundaries and security enforcement

4. **Container TDD:**
  - Tests written first for all container configurations and operations
  - Implementation follows to satisfy OCI compliance requirements
  - Validates namespace isolation and capability restrictions

5. **Memory Management TDD:**
  - Tests written first for all memory allocation patterns and scenarios
  - Implementation follows to satisfy memory protection and isolation requirements
  - Validates page mapping operations for correctness and performance

### Integration Testing

**System-Level TDD:**
- Tests written first for complete system integration scenarios
- Boot testing across multiple architectures with expected behavior defined
- Service startup and dependency resolution testing with defined outcomes
- Network protocol integration testing with specified interfaces
- Container orchestration testing with OCI compliance validation

**QEMU-Based TDD:**
- Tests written first for automated kernel deployment scenarios in QEMU virtual machines
- GDB server integration testing with defined debugging capabilities
- Step-through debugging validation with source-level visibility requirements
- Breakpoint and watchpoint testing with specified kernel development workflows
- Multi-architecture testing validation with consistent behavior across x86_64, ARM64, RISC-V

**Performance TDD:**
- Tests written first defining performance requirements and benchmarks
- IPC performance benchmarking with specified latency and throughput targets
- Memory management performance testing with defined allocation/deallocation speeds
- Service registration and discovery performance with specified lookup times
- Container startup and operation performance with defined resource usage limits

**Security TDD:**
- Tests written first defining security requirements and threat models
- Capability system enforcement testing with specified access control scenarios
- Namespace isolation verification with defined boundary testing
- Container security boundary testing with specified isolation requirements
- Network security protocol testing with defined attack resistance

The TDD testing strategy ensures that FMI/OS maintains correctness, security, and performance across all supported platforms and use cases while providing comprehensive validation of the system's Plan 9-inspired architecture and modern container capabilities. Every component is developed with tests defining expected behavior first, followed by implementation to satisfy those requirements.
