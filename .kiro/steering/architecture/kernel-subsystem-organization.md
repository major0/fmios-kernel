---
inclusion: always
---

# Kernel Subsystem Organization

This document establishes the organization principles for kernel subsystems in the FMI/OS project.

## Core Principles

1. **One Subsystem Per File**: Each kernel subsystem is implemented in a single source file in the `kernel/` directory
2. **Global Resource Management**: Kernel subsystems manage global directory structures and resources
3. **Filesystem Interface**: Each subsystem provides a filesystem interface for inspection and tuning
4. **Clear Separation**: Interfaces without global state or filesystem needs belong in `lib/`

## Directory Structure

### Kernel Directory (`kernel/`)

The `kernel/` directory contains:
- `main.c` - Kernel entry point and subsystem initialization
- One source file per kernel subsystem (e.g., `sched.c`, `proc.c`, `mem.c`)

### Library Directory (`lib/`)

The `lib/` directory contains:
- Interfaces with custom types but no global data structures
- Utility functions and data structures
- Code that doesn't require filesystem interfaces

## Kernel Subsystem Definition

A kernel subsystem is characterized by:

1. **Global Directory Structure**: Manages resources through a global data structure (tree, list, hash table, etc.)
2. **Filesystem Interface**: Provides a virtual filesystem interface for inspection and tuning
3. **Resource Management**: Controls allocation, deallocation, and lifecycle of system resources

### Examples of Kernel Subsystems

#### Scheduler (`kernel/sched.c`)
- **Global Structure**: Manages global scheduler queues and thread scheduling
- **Filesystem Interface**: `//kern/sched/` directory
  - `//kern/sched/stats` - Scheduler statistics
  - `//kern/sched/policy` - Current scheduling policy
  - `//kern/sched/quantum` - Time quantum settings
  - `//kern/sched/queues` - Information about scheduler queues
- **Resource Management**: Thread scheduling, CPU time allocation, priority management

#### Process Management (`kernel/proc.c`)
- **Global Structure**: Manages global process tree and process table
- **Filesystem Interface**: `//proc/` directory
  - `//proc/` - Root directory listing all processes
  - `//proc/[pid]/` - Per-process directory
  - `//proc/[pid]/status` - Process status information
  - `//proc/[pid]/mem` - Memory usage information
  - `//proc/[pid]/threads` - Thread information
- **Resource Management**: Process creation, destruction, PID allocation, process tree maintenance

#### Memory Management (`kernel/mem.c`)
- **Global Structure**: Manages global memory pools, page tables, and memory statistics
- **Filesystem Interface**: `//kern/mem/` directory
  - `//kern/mem/stats` - Memory usage statistics
  - `//kern/mem/pools` - Information about memory pools
  - `//kern/mem/pages` - Physical page information
  - `//kern/mem/slabs` - Slab allocator statistics
- **Resource Management**: Physical memory allocation, virtual memory management, memory pools

#### Slab Allocator (`kernel/slab.c`)
- **Global Structure**: Manages global slab caches and size classes for kernel memory allocation
- **Filesystem Interface**: `//kern/slab/` directory in the KFS
  - `//kern/slab/stats` - Slab allocation statistics
  - `//kern/slab/caches` - Information about size classes and slab caches
  - `//kern/slab/usage` - Memory usage by size class
- **Resource Management**: Kernel memory allocation (kmalloc/kfree), slab cache management, object allocation

#### Namespace Management (`kernel/ns.c`)
- **Global Structure**: Manages global namespace tree and mount points
- **Filesystem Interface**: `//kern/ns/` directory
  - `//kern/ns/mounts` - Current mount points
  - `//kern/ns/stats` - Namespace statistics
  - `//kern/ns/tree` - Namespace tree structure
- **Resource Management**: Namespace creation, mounting, path resolution

#### IPC Management (`kernel/ipc.c`)
- **Global Structure**: Manages global IPC channels and message queues
- **Filesystem Interface**: `//kern/ipc/` directory
  - `//kern/ipc/channels` - Active IPC channels
  - `//kern/ipc/stats` - IPC usage statistics
  - `//kern/ipc/queues` - Message queue information
- **Resource Management**: IPC channel creation, message routing, resource cleanup

## Library Components

Components that belong in `lib/` rather than `kernel/`:

### Data Structures (`lib/lists.c`, `lib/kobj.c`)
- **Purpose**: Provide reusable data structures and object management
- **No Global State**: Don't manage global resources directly
- **No Filesystem Interface**: No need for inspection or tuning via filesystem
- **Usage**: Used by kernel subsystems to implement their functionality

### Synchronization Primitives (`lib/spinlock.c`)
- **Purpose**: Provide low-level synchronization mechanisms
- **No Global State**: Individual locks don't require global management
- **No Filesystem Interface**: Lock state inspection handled by subsystems using them
- **Usage**: Building blocks for kernel subsystem synchronization

### Utility Functions (`lib/string.c`, `lib/math.c`)
- **Purpose**: Provide utility functions for kernel subsystems
- **No Global State**: Stateless utility functions
- **No Filesystem Interface**: No configuration or inspection needed
- **Usage**: Helper functions used throughout the kernel

## Subsystem Implementation Pattern

Each kernel subsystem follows a consistent implementation pattern:

### File Structure
```c
// kernel/subsystem.c

/* Global subsystem state */
static struct subsystem_manager {
    // Global data structures
    // Statistics
    // Configuration
} global_subsystem;

/* Subsystem initialization (called from main.c) */
int subsystem_init(void);

/* Core subsystem operations */
// Resource allocation/deallocation
// Resource management functions
// Internal helper functions

/* Filesystem interface implementation */
// VFS operations for subsystem directory
// File read/write operations for tuning
// Statistics reporting functions
```

### Filesystem Interface Pattern
```c
// Filesystem operations for //kern/subsystem/ or //subsystem/
static struct vfs_operations subsystem_vfs_ops = {
    .readdir = subsystem_readdir,
    .open = subsystem_open,
    .read = subsystem_read,
    .write = subsystem_write,
    .close = subsystem_close,
};

// Register filesystem interface during initialization
int subsystem_init(void) {
    // Initialize global state
    // Register VFS interface
    vfs_register_subsystem("subsystem", &subsystem_vfs_ops);
    return 0;
}
```

## Subsystem Interaction Guidelines

### Inter-Subsystem Communication
- Subsystems interact through well-defined APIs
- No direct access to other subsystems' global state
- Use proper locking when accessing shared resources
- Follow established lock ordering to prevent deadlocks

### Resource Sharing
- Shared resources managed through explicit interfaces
- Use reference counting for shared objects
- Implement proper cleanup on subsystem shutdown
- Coordinate resource allocation to prevent conflicts

### Filesystem Interface Coordination
- Each subsystem owns its filesystem namespace
- No conflicts in filesystem paths between subsystems
- Consistent interface patterns across all subsystems
- Proper error handling and resource cleanup

## Migration Guidelines

When organizing existing code into this structure:

### Identify Kernel Subsystems
1. **Look for global state management**: Code that manages system-wide resources
2. **Identify inspection needs**: Code that would benefit from runtime inspection
3. **Find tuning requirements**: Code with parameters that need runtime adjustment

### Move Code to Appropriate Locations
1. **Kernel subsystems** → `kernel/subsystem.c`
2. **Utility functions** → `lib/utility.c`
3. **Data structures** → `lib/structures.c`
4. **Type definitions** → `include/types.h`

### Implement Filesystem Interfaces
1. **Design directory structure** for each subsystem
2. **Implement VFS operations** for inspection and tuning
3. **Add statistics collection** for runtime monitoring
4. **Provide configuration interfaces** for tunable parameters

## Examples

### Scheduler Subsystem
```c
// kernel/sched.c
static struct scheduler_manager {
    list_t ready_queues[MAX_PRIORITIES];
    btree_t thread_tree;
    spinlock_t sched_lock;
    struct sched_stats stats;
    struct sched_config config;
} global_scheduler;

int sched_init(void);
int sched_add_thread(thread_t *thread);
int sched_remove_thread(thread_t *thread);
thread_t *sched_next_thread(void);

// Filesystem interface: //kern/sched/
static int sched_vfs_read_stats(char *buf, size_t size);
static int sched_vfs_write_config(const char *buf, size_t size);
```

### Process Subsystem
```c
// kernel/proc.c
static struct process_manager {
    btree_t process_tree;
    hash_table_t pid_table;
    spinlock_t proc_lock;
    struct proc_stats stats;
    pid_t next_pid;
} global_processes;

int proc_init(void);
process_t *proc_create(process_t *parent);
int proc_destroy(process_t *proc);
process_t *proc_find_by_pid(pid_t pid);

// Filesystem interface: //proc/
static int proc_vfs_readdir(struct vfs_dir *dir);
static int proc_vfs_read_status(process_t *proc, char *buf, size_t size);
```

## Compliance Verification

All kernel code MUST demonstrate:

1. **Proper Organization**: Kernel subsystems in `kernel/`, utilities in `lib/`
2. **Global State Management**: Each subsystem manages its global resources
3. **Filesystem Interface**: Each subsystem provides inspection and tuning interfaces
4. **Consistent Patterns**: All subsystems follow the established implementation pattern
5. **Clear Separation**: No mixing of subsystem and utility code
6. **Resource Management**: Proper allocation, tracking, and cleanup of resources

This organization ensures a clean, maintainable, and inspectable kernel architecture where each subsystem has clear responsibilities and well-defined interfaces.
