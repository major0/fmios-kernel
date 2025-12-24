---
inclusion: always
---

# Kernel Object Safety and Thread-Safe Data Structures

All kernel subsystems must use thread-safe data structures integrated with the kernel object (kobj) system. This ensures safe concurrent operations and automatic memory management across all kernel components.

## Core Principles

**All kernel data structures MUST be made safe via kobj locking integration.** This includes linked lists, hash tables, B-trees, and any other data structures used within the kernel.

**All subsystems MUST use existing core library routines and MUST NOT duplicate functionality.** Code reuse and proper layering prevent maintenance issues and ensure consistency across the kernel.

## Required Safe Data Structures

### Thread-Safe Lists (safe_list)
- **Purpose**: Thread-safe doubly-linked lists with kobj integration
- **Locking**: Read-write locks for concurrent readers, exclusive writers
- **Features**: Automatic element counting, owner-based lock hierarchy
- **Usage**: All kernel lists must use this implementation

### Thread-Safe Hash Tables (safe_hash_table)
- **Purpose**: Thread-safe hash tables with per-bucket locking
- **Locking**: Per-bucket list structures for fine-grained concurrency
- **Features**: Atomic total count, configurable bucket count
- **Usage**: All kernel hash tables must use this implementation

### Thread-Safe B-Trees (safe_btree)
- **Purpose**: Thread-safe B-tree implementation with kobj pools
- **Locking**: Tree-level read-write locks with proper ordering
- **Features**: Kobj pool integration for node allocation, iterator support
- **Usage**: Process trees, namespace trees, and other kernel trees

## Kernel Object Integration

### Unified kobj Headers
All kernel data structures must use unified `kobj_t` headers which provide:
- **Type Safety**: Magic number validation for object type verification
- **Reference Counting**: Atomic reference counting for safe memory management
- **Lock Integration**: Object-level locking for thread safety
- **Pool Management**: Integration with kernel object pools for efficient allocation

### Owner Object Requirements
All safe data structures require an owner kobj for lock hierarchy:
```c
struct my_subsystem {
    kobj_t kobj;        // Owner object
    list_t item_list;   // Safe list owned by this object
};

// Initialize with proper ownership
kobj_init(&subsystem->kobj, KOBJ_SERVICE);
list_init(&subsystem->item_list, &subsystem->kobj);
```

## Lock Hierarchy and Ordering

To prevent deadlocks, all kobj-safe data structures follow strict lock hierarchy:

1. **Global Subsystem Locks** (highest priority)
2. **Owner Object Locks** (medium priority)  
3. **Data Structure Locks** (lowest priority)

**Rule**: Always acquire locks in order from highest to lowest priority.

## Memory Management Integration

All kobj-safe data structures integrate with kernel memory management:
- **Object Pools**: Use kobj pools for efficient allocation/deallocation
- **Reference Counting**: Automatic cleanup when reference count reaches zero
- **Pool Return**: Objects returned to pools rather than freed to main memory
- **Slab Integration**: Leverage slab allocator for optimal memory utilization

## Subsystem Requirements

### Console Subsystem
- **MUST** use `safe_list` for console driver registration and management
- **MUST** provide kobj-based console driver entries
- **MUST** use proper locking for console multiplexing

### Memory Management
- **MUST** use `safe_list` for VMA management
- **MUST** use `safe_list` for free page tracking
- **MUST** integrate with kobj pools for allocation

### Process Management
- **MUST** use `safe_btree` for process PID lookup
- **MUST** use `safe_list` for process groups and sessions
- **MUST** provide thread-safe process tree operations

### Thread Management
- **SHOULD** use `safe_list` for scheduler queues where beneficial
- **MUST** integrate with kobj system for thread objects
- **MUST** provide proper lock ordering with process management

### IPC Management
- **MUST** use `safe_list` for IPC channel management
- **MUST** integrate with kobj system for IPC objects
- **MUST** provide thread-safe message queue operations

### Namespace Management
- **MUST** use `safe_btree` for namespace tree structure
- **MUST** use `safe_list` for service registration
- **MUST** provide thread-safe namespace operations

## Implementation Guidelines

### Data Structure Usage
```c
// Thread-safe list usage
list_t my_list;
list_init(&my_list, &owner_kobj);

// Thread-safe hash table usage
hash_table_t my_table;
hash_table_init(&my_table, bucket_count, &owner_kobj);

// Thread-safe B-tree usage
btree_t my_tree;
btree_init(&my_tree, &owner_kobj);
```

### Safe Iteration Patterns
```c
// Safe list iteration
if (list_for_each_begin(&my_list) == 0) {
    struct list_head *pos;
    list_for_each_safe_locked(pos, &my_list) {
        // Process entry
    }
    list_for_each_end(&my_list);
}
```

### Error Handling
All kobj-safe operations return error codes:
```c
// Check return values
if (list_add_safe(&my_list, &new_entry) != 0) {
    // Handle allocation or locking failure
    return -ENOMEM;
}
```

This kernel object safety framework ensures that all FMI/OS subsystems maintain thread safety and proper memory management while providing consistent interfaces and preventing common concurrency issues.