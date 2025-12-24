# Requirements Document

## Introduction

FMI/OS (Flexible Microkernel Infrastructure Operating System) is a multi-component microkernel operating system designed with modularity, security, and flexibility as core principles. The system follows a microkernel architecture where the kernel provides minimal core services while user-space servers handle higher-level functionality like file systems, device drivers, and network protocols.

## Test-Driven Development Methodology

FMI/OS follows comprehensive Test-Driven Development as defined in the project steering documents. All kernel subsystems must comply with the TDD methodology for consistent quality and reliability.

## Glossary

- **FMI_OS**: The Flexible Microkernel Infrastructure Operating System kernel
- **Kernel**: The core kernel providing IPC, memory management, scheduling, and POSIX thread/process management
- **KFS**: The Kernel Filesystem (KFS) accessed via `//` paths, providing a complete filesystem where services register in a hierarchical structure accessible via standard filesystem operations
- **Dependency_Notification**: Kernel mechanism allowing services to wait for and be notified when specific KFS entries become available
- **Proc_Information**: Kernel mechanism for exposing process data through the `//proc/` path in the KFS, similar to Plan 9 and Linux `/proc`
- **Kern_Information**: Kernel mechanism for exposing kernel-specific data through the `//kern/` path in the KFS, separate from process information
- **Root_Namespace**: The `/` namespace where services can be bound/mounted for access
- **Service**: A user-space process providing system functionality (to be implemented in separate userland project)
- **Driver**: A user-space process that manages hardware devices (to be implemented in separate userland project)
- **IPC_System**: Inter-Process Communication system using filesystem-based operations and memory page mapping
- **Page_Map**: Kernel function for mapping memory pages between processes for efficient data transfer
- **Memory_Manager**: Component responsible for virtual memory management and protection
- **Thread_Manager**: Kernel component managing POSIX threads and processes with `rfork()` functionality
- **Rfork_System**: Plan 9-style process creation allowing namespace customization, capability restriction, and OCI-compliant container implementation
- **Container_Implementation**: OCI-compliant container runtime built on top of the `rfork()` system
- **Build_System**: Tools and scripts for compiling, linking, testing, and generating kernel images with Valgrind integration
- **Target_Architecture**: Supported CPU architectures (x86_64, ARM64, RISC-V) for multi-platform deployment
- **Boot_Protocol_Support**: Kernel support for being loaded by standard bootloaders (U-Boot, UEFI, GRUB multiboot) without including its own bootloader
- **Capability_System**: Security model based on capabilities for fine-grained access control without root privileges
- **Klibc**: Kernel-specific implementation of essential C library functions (kprintf, kmalloc, etc.)
- **Testing_Klibc**: Alternative klibc implementation for unit testing that wraps standard libc calls

## Requirements

### Requirement 1: Low-Level Primitives and Foundation

**User Story:** As a kernel developer, I want all low-level primitives to be fully implemented and tested before any higher-level functionality is attempted, so that the kernel has a solid foundation and all components can rely on properly functioning basic operations.

#### TDD Approach

This requirement follows strict Test-Driven Development where tests for each primitive are written first, defining expected behavior, followed by implementation to satisfy those tests.

#### Acceptance Criteria

1. THE Kernel SHALL implement and test all read/write locks using standard C atomic operations before using them in higher-level components
2. THE Kernel SHALL use standard C atomic operations (stdatomic.h or compiler intrinsics) for all synchronization primitives instead of implementing custom atomic operations
3. THE Kernel SHALL implement and test all list operations before using them in higher-level components
4. THE Kernel SHALL implement and test all B-tree operations before using them in higher-level components
5. THE Kernel SHALL implement and test all memory management primitives before using them in higher-level components
6. THE Kernel SHALL implement and test all synchronization primitives (spinlocks, read-write locks) using standard atomic operations before using them in higher-level components
7. WHEN implementing any higher-level functionality, THE Kernel SHALL only use previously tested and verified low-level primitives
8. THE Kernel SHALL provide thread-safe variants of all data structures (lists, hash tables, B-trees) with kobj integration
9. THE Kernel SHALL use unified fmi_kobj headers for all kernel data structures to provide type safety, reference counting, and lock integration
10. THE Kernel SHALL implement proper lock hierarchy and ordering to prevent deadlocks in concurrent operations

#### TDD Test Requirements

1. ALL synchronization primitives SHALL have comprehensive unit tests written before implementation
2. ALL data structure operations SHALL have property-based tests validating correctness across random inputs
3. ALL memory management functions SHALL have tests for allocation, deallocation, and error conditions
4. ALL lock operations SHALL have concurrency tests validating thread safety
5. ALL integration points SHALL have tests validating proper interaction between primitives

### Requirement 1A: Kernel Object Safety and Thread-Safe Data Structures

**User Story:** As a kernel developer, I want all kernel data structures to be thread-safe and integrated with the kernel object system, so that concurrent operations are safe and memory management is automatic.

#### TDD Approach

All kernel data structures follow Test-Driven Development with comprehensive concurrency tests written before implementation to validate thread safety and proper integration with the kernel object system as defined in the project steering documents.

#### Acceptance Criteria

1. THE Console_Subsystem SHALL use safe_list for console driver registration and management
2. THE Memory_Manager SHALL use safe_list for VMA management and free page tracking
3. THE Process_Manager SHALL use safe_btree for process PID lookup and management
4. THE Thread_Manager SHALL integrate with kobj system for thread object management
5. THE IPC_System SHALL use safe_list for IPC channel management
6. THE Namespace_Manager SHALL use safe_btree for namespace tree structure

#### TDD Test Requirements

1. ALL safe data structures SHALL have concurrency tests validating thread safety under high contention
2. ALL kobj integration SHALL have tests validating reference counting and memory management
3. ALL lock hierarchy enforcement SHALL have tests preventing deadlock scenarios
4. ALL error conditions SHALL have tests validating proper cleanup and resource management
5. ALL subsystem integrations SHALL have tests validating correct usage of safe data structures

### Requirement 2: Kernel Library Architecture and Dual Implementation Strategy

**User Story:** As a kernel developer, I want a clear separation between core primitives, kernel libc functions, and testing infrastructure, so that the kernel can be self-contained while still being testable with standard tooling.

#### TDD Approach

The dual klibc implementation strategy enables comprehensive testing where the testing klibc provides instrumentation and debugging capabilities while maintaining interface parity with the kernel klibc. Tests are written first for both implementations to ensure behavioral consistency.

#### Acceptance Criteria

1. THE Kernel SHALL provide a `kernel/lib/` directory containing two distinct library categories:
   - Core primitives library (synchronization, data structures, utilities)
   - Kernel libc (klibc) located in `kernel/lib/c/` providing kernel-specific implementations of standard functions
2. THE Core primitives library SHALL include synchronization primitives organized in separate source files:
   - `spinlock.c` - Spinlock implementation using standard atomics
   - NOTE: Read-write locks, mutexes and semaphores are NOT part of the core primitives library
   - NOTE: Read-write locks, mutexes and semaphores will be implemented as part of the thread management system
3. THE Kernel libc SHALL be located in `kernel/lib/c/` and provide kernel-specific implementations:
   - `kprintf.c` - Kernel printf implementation for console output
   - `klogf.c` - Kernel logging function with log levels and timestamps
   - `kmem.c` - General-purpose memory operations (kmalloc, kfree, krealloc, kcalloc) that provide wrapper functions calling the kernel slab allocator
   - `khalt.c` - System halt and panic functions (kpanic, kabort)
   - All other standard functions (string, memory, math, conversion) are provided by compiler
4. THE Kernel SHALL compile as a standalone binary using only its own klibc implementation from `kernel/lib/c/`
5. THE Testing framework SHALL provide an alternative klibc implementation in `tests/klibc/` that:
   - Wraps standard libc calls for testing purposes
   - Provides the same interface as the kernel klibc
   - Enables unit testing of kernel code in userspace
   - Implements exactly the same functions as the kernel klibc (no more, no less)
6. THE Build system SHALL use different linker library paths to select the appropriate klibc:
   - Kernel builds: Link against `kernel/lib/c/`
   - Test builds: Link against `tests/klibc/`
7. THE Kernel klibc SHALL NOT depend on any external libraries or system calls
8. THE Testing klibc SHALL provide debugging and instrumentation capabilities not available in the kernel version
9. BOTH kernel and testing klibc implementations SHALL implement exactly the same interface:
   - Neither implementation may have extra functions beyond those declared in the shared header
   - Neither implementation may omit functions declared in the shared header
   - All function signatures MUST match exactly between implementations

#### TDD Test Requirements

1. ALL klibc functions SHALL have unit tests written before implementation for both kernel and testing versions
2. ALL interface parity SHALL be validated through automated tests comparing function signatures and behavior
3. ALL kernel klibc functions SHALL have tests validating freestanding environment compatibility
4. ALL testing klibc functions SHALL have tests validating proper wrapping of standard libc calls
5. ALL build system configurations SHALL have tests validating correct library selection

### Requirement 2A: Testing Klibc Validation

**User Story:** As a kernel developer, I want comprehensive unit tests for the testing version of klibc, so that I can ensure the testing infrastructure provides accurate simulation of kernel behavior and maintains interface parity with the kernel klibc.

#### Acceptance Criteria

1. THE Testing framework SHALL include comprehensive unit tests for all testing klibc functions in `tests/klibc/`
2. THE Testing klibc unit tests SHALL verify that each function correctly wraps the corresponding standard libc function
3. THE Testing klibc unit tests SHALL validate that all function signatures match the kernel klibc interface exactly
4. THE Testing klibc unit tests SHALL test error handling and edge cases for each wrapped function
5. THE Testing klibc unit tests SHALL verify that debugging and instrumentation features work correctly
6. THE Testing klibc unit tests SHALL ensure memory allocation functions (kmalloc, kfree, krealloc, kcalloc) provide proper tracking and debugging capabilities
7. THE Testing klibc unit tests SHALL validate that kprintf and klogf functions produce expected output formats
8. THE Testing klibc unit tests SHALL verify that all functions handle NULL pointers and invalid parameters gracefully
9. THE Testing framework SHALL run testing klibc unit tests as part of the standard test suite to ensure testing infrastructure reliability

### Requirement 3: Console and Output Management

**User Story:** As a system administrator, I want the kernel to support both serial and video outputs with selectable console configuration, so that I can choose the appropriate output method for different deployment scenarios.

#### TDD Approach

Console drivers follow Test-Driven Development with tests written first for each driver type (serial, video, debug) across all architectures, followed by implementation to satisfy the tests.

#### Acceptance Criteria

1. THE Kernel SHALL support serial output for console operations
2. THE Kernel SHALL support video output for console operations
3. THE Kernel SHALL allow console selection as a boot parameter
4. THE Kernel SHALL provide a unified console interface that abstracts the underlying output method
5. THE Kernel SHALL default to a sensible console output method when no boot parameter is specified

#### TDD Test Requirements

1. ALL console drivers SHALL have unit tests written before implementation for each architecture
2. ALL console interfaces SHALL have tests validating unified abstraction across driver types
3. ALL boot parameter parsing SHALL have tests validating correct console selection
4. ALL console output SHALL have tests validating correct character transmission and formatting
5. ALL fallback mechanisms SHALL have tests validating proper default console selection

### Requirement 3A: Console Initialization Priority

**User Story:** As a kernel developer, I want the console subsystem to initialize as early as possible during kernel startup, so that I can capture debug output and error messages from all other subsystem initializations.

#### Acceptance Criteria

1. THE Console subsystem SHALL take the highest reasonable priority for initialization among all kernel subsystems
2. THE Kernel SHALL initialize the console subsystem first unless the console depends on another subsystem (such as the memory subsystem)
3. WHEN the console subsystem depends on another subsystem, THE Kernel SHALL initialize the minimum required dependencies first, then immediately initialize the console
4. THE Kernel SHALL make every effort to enable console output as early as possible in the boot sequence
5. THE Console initialization SHALL occur before any other subsystem that produces debug output or error messages
6. THE Kernel SHALL ensure that console drivers are available for capturing initialization messages from all subsequent subsystem initializations
7. WHEN console initialization fails, THE Kernel SHALL attempt to fall back to alternative console methods before proceeding with other subsystem initializations

### Requirement 4: Kernel Logging System

**User Story:** As a kernel developer and system administrator, I want a comprehensive logging system with log levels and timestamps, so that I can debug issues and monitor kernel behavior effectively.

#### Acceptance Criteria

1. THE Kernel SHALL implement a `klogf()` function that wraps `kprintf()`
2. THE `klogf()` function SHALL support multiple log levels (DEBUG, INFO, WARN, ERROR, CRITICAL)
3. THE `klogf()` function SHALL produce output similar to a standard syslog logger
4. THE Kernel SHALL use the number of jiffies since boot as timestamps in `klogf()` output
5. THE Testing framework SHALL provide a special version of `klogf()` that uses current time as timestamps
6. THE `klogf()` function SHALL format log messages with timestamp, log level, and message content
7. THE Kernel SHALL provide mechanisms to filter log output by log level

### Requirement 5: Core Kernel Architecture

**User Story:** As a system architect, I want a kernel that provides essential services including POSIX thread/process management and core namespace filesystems, so that the system maintains microkernel principles while efficiently managing processes and providing fundamental system interfaces.

#### Acceptance Criteria

1. THE Kernel SHALL provide inter-process communication (IPC) services
2. THE Kernel SHALL provide memory management and protection services
3. THE Kernel SHALL provide POSIX thread and process management in kernel space
4. THE Kernel SHALL provide interrupt handling and low-level hardware abstraction
5. THE Kernel SHALL provide core KFS (Kernel Filesystem) accessible via `//` paths and `//proc/` process information
6. THE Kernel SHALL maintain a single kernel type that handles all core services
7. THE Kernel SHALL NOT include user-level file systems, network protocol stacks, or device driver functionality in kernel space

### Requirement 6: Kernel Filesystem (KFS)

**User Story:** As a system developer, I want a full filesystem implementation for the KFS with dependency notification, so that I can use standard filesystem commands to inspect and manage registered services.

#### Acceptance Criteria

1. THE Kernel SHALL provide a complete KFS (Kernel Filesystem) accessible via `//` paths supporting standard filesystem operations
2. THE KFS SHALL support filesystem commands such as `ls //` to inspect registered services
3. THE KFS SHALL organize registered services in a filesystem-like hierarchy
4. WHEN a Service registers itself, THE Kernel SHALL make it available in the KFS
5. THE Kernel SHALL support binding/mounting services from KFS `//` paths into the `/` root namespace
6. THE Kernel SHALL provide mechanisms for service discovery through standard filesystem operations on KFS
7. THE Kernel SHALL enforce access control for KFS operations
8. THE Kernel SHALL provide a notification mechanism for services to wait for specific KFS entries
9. WHEN a service requests notification for a KFS path, THE Kernel SHALL notify the service when that path becomes available

### Requirement 6A: VFS-IPC Integration

**User Story:** As a system developer, I want the KFS to seamlessly integrate with the IPC system, so that filesystem operations are automatically routed to either kernel subsystems or userland services without requiring different APIs.

#### Acceptance Criteria

1. THE Kernel SHALL provide a unified VFS request router that handles all filesystem operations on the KFS
2. WHEN a filesystem operation (open, close, read, write, readdir, stat) targets a kernel subsystem path (e.g., `//kern/mem/stats/physical`), THE VFS_Router SHALL handle the request directly through kernel-level VFS interfaces
3. WHEN a filesystem operation targets a userland service path (e.g., `//scsi/host0/0:0:0:0/block`), THE VFS_Router SHALL forward the request through the IPC system to the appropriate userland process
4. THE VFS_Router SHALL maintain a registry of path prefixes and their corresponding handlers (kernel subsystem or userland service)
5. THE VFS_Router SHALL provide transparent request forwarding such that applications cannot distinguish between kernel and userland implementations
6. THE IPC_System SHALL support filesystem operation forwarding with proper parameter marshalling and response handling
7. THE VFS_Router SHALL handle service registration and deregistration by updating the path-to-handler mapping
8. WHEN a userland service crashes or becomes unavailable, THE VFS_Router SHALL return appropriate filesystem error codes (e.g., ENOENT, EIO) for requests to that service's paths
9. THE VFS_Router SHALL enforce access control policies consistently across both kernel and userland handlers
10. THE VFS_Router SHALL support atomic operations where both kernel and userland handlers can participate in the same filesystem transaction

### Requirement 7: Kernel Information Exposure

**User Story:** As a system administrator and developer, I want the kernel to expose both process and kernel-specific information through the KFS, so that I can monitor and manage both processes and kernel state through familiar filesystem operations.

#### Acceptance Criteria

1. THE Kernel SHALL expose process information through the `//proc/` path in the KFS
2. THE Kernel SHALL expose kernel-specific data through the `//kern/` path in the KFS
3. THE Kernel SHALL provide process data accessible via standard filesystem operations (read, write)
4. THE Kernel SHALL provide kernel configuration and status data via `//kern/` filesystem interface
5. THE Kernel SHALL allow process control and monitoring through the `//proc/` filesystem interface
6. THE Kernel SHALL provide system-wide process and resource information via `//proc/`
7. THE Kernel SHALL integrate both process and kernel information access with the capability system for access control

### Requirement 7A: Enhanced VFS Statistics Interface

**User Story:** As a system administrator and monitoring tool developer, I want fine-grained statistics through structured sub-directories with header/data format, so that I can build robust monitoring tools that are forward-compatible with kernel changes and can parse statistics programmatically.

#### Acceptance Criteria

1. THE Kernel SHALL implement the standardized KFS statistics interface as defined in the project steering documents
2. THE Memory subsystem SHALL provide statistics following the standard format
3. THE Process subsystem SHALL provide statistics following the standard format
4. THE Thread subsystem SHALL provide statistics following the standard format
5. THE IPC subsystem SHALL provide statistics following the standard format
6. THE Namespace subsystem SHALL provide statistics following the standard format

### Requirement 8: Inter-Process Communication

**User Story:** As a system developer, I want Plan 9-style filesystem-based IPC with async notifications and memory mapping, so that processes can communicate efficiently through familiar filesystem operations.

#### Acceptance Criteria

1. THE IPC_System SHALL model all inter-process communication as filesystem operations (read, write, open, close)
2. THE IPC_System SHALL support inotify/dnotify-style asynchronous message notifications
3. THE IPC_System SHALL provide `page_map()` functionality to map memory pages between processes
4. WHEN processes use `page_map()`, THE IPC_System SHALL enable fast user-to-user copy operations without kernel copying
5. THE IPC_System SHALL enforce security policies through filesystem-style permissions and access control

### Requirement 9: Memory Management

**User Story:** As a system administrator, I want robust memory protection and management, so that processes cannot interfere with each other or the kernel.

#### Acceptance Criteria

1. THE Memory_Manager SHALL provide virtual memory mapping for each process
2. THE Memory_Manager SHALL enforce memory protection boundaries between processes
3. THE Memory_Manager SHALL prevent user processes from accessing kernel memory directly
4. WHEN a process requests memory allocation, THE Memory_Manager SHALL provide isolated memory regions
5. WHEN a process terminates, THE Memory_Manager SHALL reclaim all allocated memory

### Requirement 9A: Kernel Memory Allocation (Slab Allocator)

**User Story:** As a kernel developer, I want efficient kernel memory allocation with bitmap-based tracking and O(1) performance, so that kernel subsystems can allocate memory quickly and reliably with minimal fragmentation.

#### TDD Approach

The slab allocator follows Test-Driven Development with comprehensive tests written first for allocation patterns, performance characteristics, and error conditions, followed by implementation to satisfy those tests.

#### Acceptance Criteria

1. THE Slab_Allocator SHALL provide _kmalloc() and _kfree() internal functions for kernel memory allocation
2. THE Klibc SHALL provide kmalloc() and kfree() wrapper functions that call the slab allocator internal functions
3. THE Slab_Allocator SHALL use power-of-2 size classes (16, 32, 64, 128, 256, 512, 1024, 2048, 4096 bytes)
4. THE Slab_Allocator SHALL maintain separate slab caches for each size class with bitmap-based free object tracking
5. THE Slab_Allocator SHALL use find_first_set() and find_first_zero() operations for O(1) allocation and deallocation performance
6. THE Slab_Allocator SHALL maintain "available" and "full" lists per slab pool for efficient cache management
7. THE Slab_Allocator SHALL track object count and total size per slab pool for statistics and debugging
8. THE Slab_Allocator SHALL protect all slab operations with spinlocks for thread-safe concurrent access
9. THE Slab_Allocator SHALL use architecture-specific page allocation as its backend memory source
10. THE Slab_Allocator SHALL organize slabs into full, partial, and empty lists for efficient allocation
11. THE Slab_Allocator SHALL handle large allocations (>4KB) by forwarding directly to the page allocator
12. THE Slab_Allocator SHALL provide allocation statistics through the `//kern/slab/` filesystem interface in the KFS
13. THE Slab_Allocator SHALL initialize during kernel startup before other subsystems that require memory allocation
14. THE Slab_Allocator SHALL use bitmap arrays to track free/allocated objects within each slab for O(1) lookup performance
15. THE Slab_Allocator SHALL implement find_first_set() and find_first_zero() using architecture-optimized bit manipulation instructions when available
16. THE Slab_Allocator SHALL maintain per-pool statistics including allocation count, deallocation count, current usage, peak usage, and fragmentation metrics
17. THE Slab_Allocator SHALL provide debugging capabilities including allocation tracking, double-free detection, and memory corruption detection when enabled

#### TDD Test Requirements

1. ALL slab allocation functions SHALL have unit tests written before implementation validating O(1) performance
2. ALL size class management SHALL have tests validating proper power-of-2 allocation and bitmap tracking
3. ALL concurrency operations SHALL have tests validating thread-safe allocation under high contention
4. ALL error conditions SHALL have tests validating proper handling of out-of-memory and corruption scenarios
5. ALL statistics interfaces SHALL have tests validating accurate reporting through the KFS filesystem
6. ALL debugging features SHALL have tests validating detection of double-free and corruption conditions

### Requirement 9B: Architecture-Specific Page Allocator

**User Story:** As a kernel developer, I want a sophisticated page allocator that dynamically adapts to available memory and handles bad memory regions, so that the kernel can efficiently manage physical memory across different hardware configurations.

#### Acceptance Criteria

1. THE Architecture-specific page allocator SHALL provide pages_alloc() and pages_free() functions using <topic>_<verb>() naming convention
2. THE Architecture-specific page allocator SHALL implement O(1) allocation and deallocation for single pages whenever possible
3. THE Architecture-specific page allocator SHALL implement a 3-level page-table based memory management structure
4. THE Architecture-specific page allocator SHALL be located in `arch/*/lib/page.c` with interface in `arch/*/include/page.h`
5. THE Page_Allocator SHALL dynamically size its page table structure based on maximum available memory reported by the bootloader
6. THE Page_Allocator SHALL integrate with architecture-specific bootloader memory detection (UEFI, Multiboot, U-Boot)
7. THE Page_Allocator SHALL use bitmap-based page tracking with 2-bit page states (FREE, ALLOCATED, INVALID, RESERVED)
8. THE Page_Allocator SHALL provide pages_mark_invalid() function to mark bad memory regions as unallocatable
9. THE Page_Allocator SHALL handle memory holes and reserved regions automatically based on bootloader memory map
10. THE Page_Allocator SHALL use find_first_zero() operations on bitmaps for efficient free page location
11. THE Page_Allocator SHALL support marking pages as invalid and unfreeable for disabling bad memory
12. THE Page_Allocator SHALL provide thread-safe operations using spinlocks for concurrent access
13. THE Page_Allocator SHALL organize memory in a 3-level hierarchy: L1 (1GB regions), L2 (16MB regions), L3 (256KB regions with bitmaps)
14. THE Page_Allocator SHALL automatically detect and adapt to memory configurations from different bootloaders

### Requirement 9C: Bootloader Memory Detection

**User Story:** As a system administrator, I want the kernel to automatically detect and adapt to available memory from different bootloaders, so that the system works correctly across various hardware platforms and boot environments.

#### Acceptance Criteria

1. THE Memory_Detection_System SHALL support memory detection from UEFI bootloaders
2. THE Memory_Detection_System SHALL support memory detection from Multiboot bootloaders  
3. THE Memory_Detection_System SHALL support memory detection from U-Boot bootloaders (ARM64/RISC-V)
4. THE Memory_Detection_System SHALL parse bootloader memory maps to identify available, reserved, bad, and ACPI memory regions
5. THE Memory_Detection_System SHALL provide a unified memory_map_t interface regardless of bootloader type
6. THE Memory_Detection_System SHALL calculate total usable memory and highest memory address from bootloader data
7. THE Memory_Detection_System SHALL be located in `arch/*/lib/memory_detect.c` with interface in `arch/*/include/memory_detect.h`
8. THE Memory_Detection_System SHALL initialize during kernel bootstrap before page allocator initialization
9. THE Memory_Detection_System SHALL provide memory_region_is_usable() function to determine if regions can be used for allocation
10. THE Memory_Detection_System SHALL handle different memory region types (MEMORY_AVAILABLE, MEMORY_RESERVED, MEMORY_BAD, MEMORY_HOLE)
11. THE Memory_Detection_System SHALL provide thread-safe access to memory map information using spinlocks
12. THE Memory_Detection_System SHALL support up to 128 memory regions to handle complex memory layouts

### Requirement 9D: Boot Command Line Parameter Handling

**User Story:** As a system administrator, I want the kernel to capture and parse boot command line parameters from bootloaders, so that I can configure kernel behavior including console selection and other runtime options.

#### Acceptance Criteria

1. THE Boot_Parameter_System SHALL capture command line parameters from UEFI bootloaders
2. THE Boot_Parameter_System SHALL capture command line parameters from Multiboot bootloaders
3. THE Boot_Parameter_System SHALL capture command line parameters from U-Boot bootloaders (ARM64/RISC-V)
4. THE Boot_Parameter_System SHALL copy command line data into kernel memory during bootstrap
5. THE Boot_Parameter_System SHALL provide cmdline_get_param() function to retrieve specific parameters by name
6. THE Boot_Parameter_System SHALL support console parameter parsing (e.g., console=ttyS0,115200)
7. THE Boot_Parameter_System SHALL expose complete command line via `//kern/cmdline` in the KFS
8. THE Boot_Parameter_System SHALL be located in `arch/*/lib/cmdline.c` with interface in `arch/*/include/cmdline.h`
9. THE Boot_Parameter_System SHALL initialize during kernel bootstrap after memory detection
10. THE Boot_Parameter_System SHALL provide thread-safe access to command line parameters using spinlocks
11. THE Boot_Parameter_System SHALL handle command line parameters up to 4096 characters in length
12. THE Boot_Parameter_System SHALL parse key=value parameter pairs and boolean flags

### Requirement 10: POSIX Thread and Process Management

**User Story:** As an application developer, I want POSIX thread and process support with Plan 9-style `rfork()` functionality and OCI-compliant container support, so that I can create processes with customized namespaces and run containerized applications.

#### Acceptance Criteria

1. THE Thread_Manager SHALL implement POSIX thread creation, scheduling, and synchronization
2. THE Thread_Manager SHALL implement POSIX process creation, management, and termination
3. THE Thread_Manager SHALL provide thread-aware mutexes with owner tracking and priority inheritance
4. THE Thread_Manager SHALL provide semaphores with proper resource counting and thread blocking/wakeup mechanisms
5. THE Thread_Manager SHALL provide condition variables for thread signaling and coordination
6. THE Thread_Manager SHALL integrate mutex and semaphore operations with the scheduler for proper blocking behavior
7. THE Thread_Manager SHALL provide `rfork()`-style process creation with namespace customization
7. THE Thread_Manager SHALL provide `rfork()`-style process creation with namespace customization
8. WHEN creating a child process with `rfork()`, THE Thread_Manager SHALL allow setting up and restricting capabilities of the current namespace before inheritance
9. THE Rfork_System SHALL serve as the primary mechanism for implementing OCI-compliant containers
10. THE Container_Implementation SHALL provide full OCI compliance for container runtime operations
11. THE Thread_Manager SHALL maintain all thread and process state information in kernel space
12. THE Thread_Manager SHALL provide preemptive multitasking for threads and processes
13. THE Thread_Manager SHALL support priority-based scheduling with multiple priority levels

### Requirement 11: User-Space System Services

**User Story:** As a system designer, I want system services and drivers to run in user space with self-managed namespace hierarchies, so that services can organize their interfaces and metadata in intuitive filesystem structures.

#### Acceptance Criteria

1. THE FMI_OS SHALL support file system services implemented as Service processes
2. THE FMI_OS SHALL support device Driver processes running in user space
3. THE FMI_OS SHALL support network protocol services implemented as Service processes
4. ALL Service processes SHALL register themselves in the KFS upon startup
5. EACH Service SHALL be responsible for managing its own filesystem hierarchy within the KFS
6. WHEN a SCSI device service registers, IT SHALL provide entries for partitions (part1, part2) and comprehensive SCSI metadata (vendor, model, serial, WWN, scsi_id) following SCSI INQUIRY standards
7. THE FMI_OS SHALL provide dependency notification mechanisms for services to wait for required resources
8. WHEN a Service or Driver crashes, THE FMI_OS SHALL isolate the failure and allow service restart
9. WHEN a filesystem service waits for a SCSI device, THE Kernel SHALL notify the service when the device becomes available

### Requirement 12: SCSI Subsystem

**User Story:** As a system administrator, I want a unified SCSI interface for all block devices including legacy ATA and modern NVMe devices, so that I have consistent device management and comprehensive metadata access regardless of the underlying storage protocol.

#### Acceptance Criteria

1. THE FMI_OS SHALL provide a unified SCSI subsystem accessible through the `//scsi/` path in the KFS
2. THE SCSI subsystem SHALL register host adapters as `//scsi/hostN/` where N is the host number
3. THE SCSI subsystem SHALL address individual devices using standard SCSI addressing format `H:C:T:L` (Host:Channel:Target:LUN)
4. WHEN a SCSI device is detected, THE system SHALL populate complete SCSI INQUIRY metadata including vendor, model, revision, serial number, and World Wide Name when available
5. THE SCSI subsystem SHALL provide SCSI-ATA Translation (SAT) layer for legacy ATA/SATA devices
6. THE SCSI subsystem SHALL provide SCSI translation layer for NVMe devices
7. THE system SHALL maintain a legacy compatibility layer in `//block/` path within the KFS with symlinks to SCSI devices
8. THE SCSI subsystem SHALL support all standard SCSI device types including direct-access, sequential-access, and optical devices
9. THE SCSI subsystem SHALL provide partition detection and management for direct-access devices
10. THE SCSI subsystem SHALL integrate with the capability system for fine-grained access control

### Requirement 13: Network Protocol Support

**User Story:** As a network administrator, I want comprehensive TCP/IP networking with IPC-to-network protocol mapping, so that the system can participate in modern networks while maintaining Plan 9-style distributed computing capabilities.

#### Acceptance Criteria

1. THE FMI_OS SHALL support TCP/IP networking with both IPv4 and IPv6 protocols
2. THE FMI_OS SHALL implement a userland networking service that runs as a Service process
3. THE Network_Service SHALL transparently map the IPC system to a network protocol with matching interfaces
4. THE Network_Protocol SHALL be designed in the spirit of 9P to interface seamlessly with the IPC system
5. THE Network_Service SHALL enable remote access to local services through the network protocol

### Requirement 14: Distributed Computing Interfaces

**User Story:** As a distributed systems developer, I want 9P and FUSE interfaces to the IPC system, so that I can integrate FMI/OS with existing distributed computing and filesystem ecosystems.

#### Acceptance Criteria

1. THE FMI_OS SHALL provide a 9P interface to the IPC system for Plan 9 compatibility
2. THE FMI_OS SHALL provide a FUSE wrapper interface to the IPC system for Linux compatibility
3. THE 9P_Interface SHALL allow remote Plan 9 systems to access FMI/OS services
4. THE FUSE_Interface SHALL allow Linux systems to mount FMI/OS services as filesystems
5. THE Distributed_Interfaces SHALL maintain security and capability restrictions when accessed remotely

### Requirement 15: Service Development Libraries

**User Story:** As a service developer, I want helper libraries with D-Bus-like functionality integrated into the KFS, so that I can easily implement inter-service communication and maintain consistent metadata within my service's KFS entry.

#### Acceptance Criteria

1. THE FMI_OS SHALL provide helper libraries for service interaction with the KFS
2. THE Service_Libraries SHALL integrate D-Bus-like functionality within the KFS system
3. THE Service_Libraries SHALL facilitate consistent metadata management within service KFS entries
4. THE Service_Libraries SHALL provide standardized interfaces for common service operations (registration, metadata publishing, dependency waiting, inter-service messaging)
5. THE Service_Libraries SHALL support multiple programming languages used for service development
6. THE Service_Libraries SHALL include templates and examples for common service types (SCSI devices, network interfaces, filesystems)
7. THE Service_Libraries SHALL enforce consistent naming conventions and metadata formats across services

### Requirement 16: Service Testing and Monitoring Tools

**User Story:** As a system administrator and developer, I want comprehensive tooling for testing and monitoring asynchronous operations in the `//` namespace, so that I can debug service interactions and monitor system behavior.

#### Acceptance Criteria

1. THE FMI_OS SHALL provide tooling for testing asynchronous input/output operations in the `//` namespace
2. THE Monitoring_Tools SHALL monitor inter-service communication through the `//` namespace
3. THE Testing_Tools SHALL simulate service interactions and dependency scenarios
4. THE Monitoring_Tools SHALL provide real-time visibility into service registration, deregistration, and communication
5. THE Testing_Tools SHALL validate service behavior under various load and failure conditions
6. THE Monitoring_Tools SHALL integrate with the capability system to provide authorized access to service monitoring data

**User Story:** As a developer, I want comprehensive multi-architecture build tools with integrated testing framework, so that I can efficiently develop, test, and maintain the operating system for multiple target architectures.

#### Acceptance Criteria

1. THE Build_System SHALL compile the kernel from source code for multiple target architectures
2. THE Build_System SHALL compile and link user-space services and applications for multiple architectures
3. THE Build_System SHALL generate bootable system images for commonly used architectures (x86_64, ARM64, RISC-V)
4. THE Build_System SHALL support cross-compilation for all target architectures
5. THE Build_System SHALL provide dependency management for system components across architectures
6. THE Build_System SHALL organize source code to support multi-architecture builds with shared and architecture-specific components
7. THE Build_System SHALL include a comprehensive unit testing framework for kernel and userland components
8. THE Build_System SHALL integrate Valgrind testing for memory error detection and performance analysis

### Requirement 17: Build System and Development Tools

**User Story:** As a developer, I want a comprehensive autoconf/automake-based build system with integrated testing framework and dual klibc support, so that I can efficiently develop, test, and maintain the operating system for multiple target architectures with proper configuration management.

#### TDD Approach

The build system itself follows Test-Driven Development principles as defined in the project steering documents.

#### Acceptance Criteria

1. THE Build_System SHALL implement all requirements defined in the build system steering documents
2. THE Build_System SHALL support multi-architecture compilation (x86_64, ARM64, RISC-V)
3. THE Build_System SHALL provide dual klibc implementation strategy
4. THE Build_System SHALL integrate QEMU-based smoke testing
5. THE Build_System SHALL enforce code quality standards and forward declaration prohibition
6. THE Build_System SHALL support memory reclamation through linker script integration

#### TDD Test Requirements

1. ALL build system components SHALL have tests written before implementation validating correct configuration and compilation
2. ALL architecture-specific builds SHALL have tests validating proper cross-compilation and target selection
3. ALL klibc selection mechanisms SHALL have tests validating correct library linking for kernel vs test builds
4. ALL QEMU smoke tests SHALL have tests validating proper kernel boot, execution, and shutdown
5. ALL configuration options SHALL have tests validating proper feature enabling/disabling
6. ALL linker script functionality SHALL have tests validating proper memory segment organization and reclamation

### Requirement 17A: QEMU Integration Testing

**User Story:** As a kernel developer, I want automated QEMU-based smoke tests for kernel components, so that I can verify that kernel code works correctly in a real kernel environment and catch integration issues early.

#### Acceptance Criteria

1. THE Testing_Framework SHALL provide QEMU smoke tests for each major kernel component
2. THE QEMU_Tests SHALL verify basic functionality like kprintf("Hello World") output in a virtual machine
3. THE QEMU_Tests SHALL test kernel components in their actual runtime environment, not just userspace simulation
4. THE QEMU_Tests SHALL validate console output, memory management, and synchronization primitives in kernel mode
5. THE QEMU_Tests SHALL run automatically as part of the build and test process
6. THE QEMU_Tests SHALL support multiple target architectures (x86_64, ARM64, RISC-V)
7. THE QEMU_Tests SHALL provide clear pass/fail reporting with captured console output
8. THE QEMU_Tests SHALL test incremental functionality as each component is implemented
9. THE QEMU_Tests SHALL ensure kernel boots, runs test code, and shuts down cleanly
10. THE QEMU_Tests SHALL catch environment-specific issues not visible in userspace unit tests
11. THE QEMU_Tests SHALL launch QEMU with stdin/stdout configured as a serial interface to the kernel
12. THE QEMU_Tests SHALL configure the kernel console parameter to use the serial interface for all output
13. THE QEMU_Tests SHALL use QEMU serial redirection to capture kernel console output for automated validation
14. THE QEMU_Tests SHALL support programmatic input/output interaction with the kernel through the serial interface

### Requirement 18: Documentation and Project Structure

**User Story:** As a contributor, I want comprehensive documentation and clear project organization, so that I can understand and contribute to the system effectively.

#### Acceptance Criteria

1. THE FMI_OS SHALL include architectural documentation explaining the kernel design and service namespace
2. THE FMI_OS SHALL include API documentation for IPC and system services
3. THE FMI_OS SHALL include build and installation instructions
4. THE FMI_OS SHALL organize source code into logical directories (kernel/, docs/, scripts/, build/, tests/)
5. THE FMI_OS SHALL include developer guides for extending and modifying the system
6. THE FMI_OS SHALL document the interface between kernel and userland for future userland project development

### Requirement 19: System Boot and Initialization

**User Story:** As a system administrator, I want reliable system boot and initialization with support for standard bootloaders, so that the kernel starts correctly on various hardware platforms and core kernel services are properly initialized.

#### Acceptance Criteria

1. THE FMI_OS SHALL support being booted by U-Boot, UEFI, and GRUB multiboot bootloaders
2. THE Kernel SHALL initialize hardware and start essential kernel services in correct dependency order
3. THE FMI_OS SHALL NOT include its own bootloader but SHALL provide necessary boot protocol support
4. WHEN the system boots, THE FMI_OS SHALL verify integrity of core kernel components
5. THE FMI_OS SHALL provide configuration mechanisms for kernel startup parameters
6. WHEN boot failures occur, THE FMI_OS SHALL provide diagnostic information and recovery options
7. THE Kernel SHALL provide well-defined interfaces for future userland service initialization

### Requirement 19A: Boot Protocol Support

**User Story:** As a system administrator, I want comprehensive boot protocol support including Multiboot v2 and PVH ELF, so that the kernel can be booted by various bootloader configurations with proper header validation and debug capabilities.

#### TDD Approach

Boot protocol support follows Test-Driven Development with comprehensive tests written first for header validation, bootloader detection, and debug marker functionality, followed by implementation to satisfy those tests.

#### Acceptance Criteria

1. THE Build_System SHALL support Multiboot v2 and PVH ELF protocols for x86_64 targets
2. THE Build_System SHALL provide `--enable-multiboot2` configure option to enable Multiboot v2 support (default: yes)
3. THE Build_System SHALL provide `--enable-pvh-elf` configure option to enable PVH ELF support (default: yes)
4. THE Build_System SHALL allow both boot protocols to be enabled simultaneously for maximum bootloader compatibility
5. THE Multiboot v2 header SHALL be located within the first 32KB of the kernel binary file
6. THE Multiboot v2 header SHALL be placed in a `.multiboot` section for proper alignment and positioning
7. THE PVH ELF headers SHALL be properly configured with XEN_ELFNOTE_PHYS32_ENTRY and XEN_ELFNOTE_ENTRY notes
8. THE Bootstrap code SHALL detect which boot protocol was used during boot
9. THE Bootstrap code SHALL include debug markers to validate proper boot protocol detection
10. THE Bootstrap code SHALL provide appropriate entry points for different boot protocols
11. WHEN booted via Multiboot v2 in 32-bit mode, THE Bootstrap code SHALL perform 32-bit to 64-bit mode transition including CPU feature detection, page table setup, GDT configuration, and long mode activation
12. WHEN booted via Multiboot v2 in 64-bit mode, THE Bootstrap code SHALL validate that the bootloader has already established 64-bit long mode
13. WHEN booted via PVH ELF, THE Bootstrap code SHALL perform 32-bit to 64-bit mode transition as PVH always boots in 32-bit mode
14. THE Bootstrap code SHALL output debug characters to serial console to indicate successful boot protocol detection
15. THE Bootstrap code SHALL validate boot protocol magic numbers and structures
16. THE Bootstrap code SHALL save boot information structures for later kernel use
17. THE Kernel SHALL expose detected boot protocol through `//kern/boot/protocol` in the KFS
18. THE 32-bit to 64-bit mode transition SHALL include comprehensive CPU feature detection using CPUID for long mode and PAE support
19. THE Mode transition SHALL create minimal page tables (PML4→PDP→PD with 2MB pages) for 64-bit operation
20. THE Mode transition SHALL set up temporary GDT with 64-bit code and data segments
21. THE Mode transition SHALL enable PAE (Physical Address Extension) in CR4 register
22. THE Mode transition SHALL enable long mode via LME bit in EFER MSR (Extended Feature Enable Register)
23. THE Mode transition SHALL activate paging via PG bit in CR0 register
24. THE Mode transition SHALL perform far jump to 64-bit code segment to complete transition
25. THE Bootstrap code SHALL validate successful mode transition by checking EFER MSR long mode active bit

### Requirement 19B: PVH ELF Boot Protocol Support

**User Story:** As a system administrator and virtualization user, I want PVH ELF boot protocol support for optimized virtualization environments, so that the kernel can boot efficiently in Xen hypervisor and QEMU PVH configurations with proper 32-bit to 64-bit mode transitions and hypervisor integration.

#### TDD Approach

PVH ELF boot protocol support follows Test-Driven Development with comprehensive tests written first for ELF header generation, dual entry points, mode transitions, and hypervisor integration, followed by implementation to satisfy those tests.

#### Acceptance Criteria

1. THE Build_System SHALL support PVH ELF boot protocol through configure options
2. THE Build_System SHALL provide `--enable-pvh-elf` configure option to enable PVH ELF support (default: yes)
3. THE Build_System SHALL generate proper ELF64 executable format with PVH-compatible program headers
4. THE ELF header SHALL include XEN_ELFNOTE_PHYS32_ENTRY note pointing to 32-bit entry point
5. THE ELF header SHALL include XEN_ELFNOTE_ENTRY note pointing to 64-bit entry point
6. THE ELF notes section SHALL be properly structured and accessible to hypervisor
7. THE Bootstrap code SHALL provide dual entry points: `_pvh_start32` (32-bit) and `_pvh_start64` (64-bit)
8. THE 32-bit PVH entry point SHALL detect PVH start info structure via EBX register
9. THE 32-bit PVH entry point SHALL perform comprehensive CPU feature detection using CPUID
10. THE 32-bit PVH entry point SHALL create minimal page table structure (PML4→PDP→PD with 2MB pages)
11. THE 32-bit PVH entry point SHALL set up temporary GDT with 64-bit code and data segments
12. THE 32-bit PVH entry point SHALL perform complete 32-bit to 64-bit mode transition sequence
13. THE 64-bit PVH entry point SHALL validate existing 64-bit mode via EFER MSR checks
14. THE 64-bit PVH entry point SHALL parse PVH start info structure for memory map and boot parameters
15. THE Bootstrap code SHALL include debug markers for PVH boot protocol detection and validation
16. THE Bootstrap code SHALL extract memory map information from PVH start info structure
17. THE Bootstrap code SHALL parse command line parameters from PVH start info (if present)
18. THE Bootstrap code SHALL save PVH boot information for later kernel use
19. THE Kernel SHALL expose PVH start info through `//kern/boot/pvh_info` in the KFS
20. THE PVH boot protocol SHALL be compatible with Xen hypervisor PVH specification
21. THE PVH boot protocol SHALL support QEMU direct kernel boot using PVH protocol
22. THE Bootstrap code SHALL provide error handling and debug output for PVH boot failures
23. THE PVH boot protocol SHALL integrate with existing boot protocol detection and selection logic through `//kern/boot/protocol` in the KFS

### Requirement 19B: PVH ELF Boot Protocol Support

**User Story:** As a cloud infrastructure administrator, I want PVH ELF boot protocol support with proper 32-bit to 64-bit transition, so that the kernel can be booted efficiently in virtualized environments like Xen and QEMU with minimal bootloader overhead and faster boot times.

#### TDD Approach

PVH ELF support follows Test-Driven Development with comprehensive tests written first for ELF header validation, 32-bit to 64-bit mode transition, and virtualization environment detection, followed by implementation to satisfy those tests.

#### Acceptance Criteria

1. THE Build_System SHALL provide `--enable-pvh-elf` configure option to enable PVH ELF boot support (default: yes)
2. THE PVH_ELF_Support SHALL create a proper ELF64 executable with PVH-compatible entry point
3. THE PVH_ELF_Support SHALL include XEN_ELFNOTE_PHYS32_ENTRY note in the ELF header pointing to 32-bit entry point
4. THE PVH_ELF_Support SHALL include XEN_ELFNOTE_ENTRY note in the ELF header pointing to 64-bit entry point
5. THE PVH_ELF_Support SHALL provide a 32-bit entry point `_pvh_start32` for hypervisors that boot in 32-bit mode
6. THE PVH_ELF_Support SHALL provide a 64-bit entry point `_pvh_start64` for hypervisors that boot in 64-bit mode
7. WHEN booted via PVH ELF in 32-bit mode, THE Bootstrap code SHALL perform complete 32-bit to 64-bit mode transition
8. THE 32-bit to 64-bit transition SHALL include CPU feature detection (CPUID for long mode, PAE support)
9. THE 32-bit to 64-bit transition SHALL set up minimal page tables for 64-bit mode (PML4, PDP, PD with 2MB pages)
10. THE 32-bit to 64-bit transition SHALL create and load a temporary GDT with 64-bit code and data segments
11. THE 32-bit to 64-bit transition SHALL enable PAE (Physical Address Extension) in CR4
12. THE 32-bit to 64-bit transition SHALL enable long mode by setting LME bit in EFER MSR
13. THE 32-bit to 64-bit transition SHALL enable paging by setting PG bit in CR0
14. THE 32-bit to 64-bit transition SHALL perform far jump to 64-bit code segment
15. WHEN booted via PVH ELF in 64-bit mode, THE Bootstrap code SHALL validate existing 64-bit mode setup
16. THE Bootstrap code SHALL detect PVH boot protocol through EBX register containing PVH start info structure
17. THE Bootstrap code SHALL parse PVH start info structure for memory map and other boot parameters
18. THE Bootstrap code SHALL output debug characters to serial console during PVH boot process
19. THE Bootstrap code SHALL validate that long mode is active and properly configured
20. THE Bootstrap code SHALL save PVH start info structure for later kernel use
21. THE Kernel SHALL expose PVH boot information through `//kern/boot/pvh_info` in the KFS
22. THE PVH_ELF_Support SHALL be compatible with Xen hypervisor PVH boot protocol
23. THE PVH_ELF_Support SHALL be compatible with QEMU direct kernel boot using PVH protocol
24. THE Build_System SHALL generate proper ELF notes section with PVH entry points
25. THE Build_System SHALL validate ELF header structure and PVH note placement during build

#### TDD Test Requirements

1. ALL PVH ELF header generation SHALL have tests written before implementation validating proper ELF64 structure
2. ALL 32-bit to 64-bit transition code SHALL have tests validating CPU feature detection and mode switching
3. ALL page table setup SHALL have tests validating proper PML4/PDP/PD structure for long mode
4. ALL GDT setup SHALL have tests validating 64-bit segment descriptors and loading
5. ALL PVH start info parsing SHALL have tests validating memory map extraction and parameter handling
6. ALL debug marker output SHALL have tests validating proper serial console communication during boot
7. ALL QEMU PVH boot SHALL have integration tests validating successful kernel boot via PVH protocol information through `//kern/boot/protocol` in the KFS
18. THE Kernel SHALL expose multiboot information through `//kern/boot/multiboot_info` in the KFS
19. THE Build_System SHALL ensure proper linker script ordering to place multiboot headers at correct file positions
20. THE Build_System SHALL validate header positioning during build process and report errors if headers are misplaced

#### TDD Test Requirements

1. ALL boot protocol header generation SHALL have tests written before implementation validating correct positioning and format
2. ALL bootloader detection SHALL have tests validating proper magic number recognition and protocol identification
3. ALL debug marker functionality SHALL have tests validating correct serial output during boot process
4. ALL mode transition code SHALL have tests validating proper 32-bit to 64-bit transition for boot protocols that require it
5. ALL long mode validation SHALL have tests confirming proper 64-bit mode setup
6. ALL build system integration SHALL have tests validating correct header placement and linker script functionality

### Requirement 20: Documentation and Project Structure

**User Story:** As a contributor, I want comprehensive documentation and clear project organization, so that I can understand and contribute to the kernel effectively.

#### Acceptance Criteria

1. THE FMI_OS SHALL include architectural documentation explaining the kernel design and service namespace
2. THE FMI_OS SHALL include API documentation for IPC and system services
3. THE FMI_OS SHALL include build and installation instructions
4. THE FMI_OS SHALL organize source code into logical directories (kernel/, docs/, scripts/, build/, tests/)
5. THE FMI_OS SHALL include developer guides for extending and modifying the kernel
6. THE FMI_OS SHALL document the kernel-userland interface specifications for separate userland project development

### Requirement 21: Security and Isolation

**User Story:** As a security-conscious user, I want a capabilities-based security model with POSIX compatibility but without root privilege escalation, so that security vulnerabilities are contained and cannot compromise the entire system.

#### Acceptance Criteria

1. THE Kernel SHALL implement the comprehensive security and capabilities framework as defined in the project steering documents
2. THE Kernel SHALL integrate all subsystems with the capability system for access control
3. THE Kernel SHALL enforce speculative execution attack mitigations across all kernel code
4. THE Kernel SHALL provide secure boot mechanisms and integrity verification
5. THE Kernel SHALL implement memory safety requirements and corruption detection
6. THE Kernel SHALL provide security audit logging for all security-relevant operations

### Requirement 22: Architecture-Specific Boot Protocol Support

**User Story:** As a system architect, I want the build system to provide appropriate boot protocol support based on target architecture, so that compatible boot protocols are available for each architecture and proper bootloader integration is ensured.

#### Acceptance Criteria

1. THE Build_System SHALL provide Multiboot v2 and PVH ELF protocol support for x86_64 targets
2. THE Build_System SHALL provide UEFI and U-Boot protocol support for ARM64 targets
3. THE Build_System SHALL provide UEFI and U-Boot protocol support for RISC-V targets
4. THE Configure script SHALL validate boot protocol availability during configuration phase
5. THE Build_System SHALL allow multiple boot protocols to be enabled simultaneously for maximum bootloader compatibility
6. THE Build_System SHALL provide clear documentation about which boot protocols are supported for each target architecture
7. THE Build_System SHALL generate appropriate headers and entry points for each enabled boot protocol
8. THE Bootstrap code SHALL detect and handle different boot protocols appropriately for each architecture
9. THE Build_System SHALL ensure proper linker script configuration for each boot protocol
10. THE Build_System SHALL provide clear error messages if unsupported protocol combinations are requested