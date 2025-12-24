# Kernel Object Safety and Thread-Safe Data Structures

This document establishes the requirements and implementation guidelines for kernel object safety and thread-safe data structures in FMI/OS.

## Core Principles

**All kernel data structures MUST be made safe via kobj locking integration.** This includes linked lists, hash tables, B-trees, and any other data structures used within the kernel.

**All library code MUST use existing core library routines and MUST NOT duplicate functionality.** Code reuse and proper layering prevent maintenance issues and ensure consistency across the kernel.

**All data types MUST follow the established naming conventions.** See `naming-conventions.md` for detailed requirements on type naming with `_t` suffixes and struct naming with `_s` suffixes.

## Design Requirements

### 1. Unified Kernel Object Header

All kernel data structures MUST use the unified `kobj_t` header which provides:

- **Type Safety**: Magic number validation for object type verification
- **Reference Counting**: Atomic reference counting for safe memory management
- **Lock Integration**: Object-level locking for thread safety
- **Pool Management**: Integration with kernel object pools for efficient allocation

### 2. Thread-Safe Data Structures

The kernel provides thread-safe data structures that integrate with kobj:

#### Lists (`list_t`)
- **Purpose**: Thread-safe doubly-linked lists with kobj integration
- **Locking**: Read-write locks for concurrent readers, exclusive writers
- **Features**: Automatic element counting, owner-based lock hierarchy
- **Usage**: All kernel lists must use this implementation
- **Note**: These structures appear always safe; no unsafe variants exist

#### Hash Tables (`hash_table_t`)
- **Purpose**: Thread-safe hash tables with per-bucket locking
- **Locking**: Per-bucket list_t structures for fine-grained concurrency
- **Features**: Atomic total count, configurable bucket count
- **Usage**: All kernel hash tables must use this implementation

#### B-Trees (`btree_t`)
- **Purpose**: Thread-safe B-tree implementation with kobj pools
- **Locking**: Tree-level read-write locks with proper ordering
- **Features**: Kobj pool integration for node allocation, iterator support
- **Usage**: Process trees, namespace trees, and other kernel trees

### 3. Lock Hierarchy and Ordering

To prevent deadlocks, all kobj-safe data structures follow a strict lock hierarchy:

1. **Global Subsystem Locks** (highest priority)
2. **Owner Object Locks** (medium priority)  
3. **Data Structure Locks** (lowest priority)

**Rule**: Always acquire locks in order from highest to lowest priority.

### 4. Core Library Routine Usage

All library code MUST use existing core library routines and MUST NOT duplicate functionality:

#### Synchronization Primitives
- **MUST** use `lib/spinlock.c` functions for spinlock operations
- **MUST** use `lib/condvar.c` functions for condition variable operations
- **MUST** use `lib/waitqueue.c` functions for wait queue operations
- **MUST NOT** implement custom synchronization primitives

#### Memory Management
- **MUST** use `lib/kobj.c` functions for kernel object operations
- **MUST** use kobj pools for object allocation/deallocation
- **MUST NOT** implement custom memory allocation schemes
- **MUST NOT** use raw kmalloc/kfree for kernel objects

#### Data Structures
- **MUST** use `lib/lists.c` functions for all list, hash table, and B-tree operations
- **MUST NOT** implement custom data structure variants
- **MUST NOT** duplicate existing data structure functionality

#### Error Handling
- **MUST** use consistent error codes across all library functions
- **MUST** follow established error handling patterns
- **MUST NOT** create custom error handling mechanisms

### 5. Memory Management Integration

All kobj-safe data structures integrate with the kernel memory management:

- **Object Pools**: Use kobj pools for efficient allocation/deallocation
- **Reference Counting**: Automatic cleanup when reference count reaches zero
- **Pool Return**: Objects returned to pools rather than freed to main memory
- **Slab Integration**: Leverage slab allocator for optimal memory utilization

## Implementation Guidelines

### 1. Data Structure Usage

When using kobj-safe data structures:

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

### 2. Core Library Integration

All library code must use existing core routines:

```c
// CORRECT: Use existing spinlock functions
#include "sync.h"
spinlock_t my_lock;
spinlock_init(&my_lock);
spin_lock(&my_lock);
// ... critical section ...
spin_unlock(&my_lock);

// INCORRECT: Custom spinlock implementation
// NEVER implement custom synchronization primitives

// CORRECT: Use existing kobj functions
#include "kobj.h"
kobj_t obj;
kobj_init(&obj, KOBJ_SERVICE);
kobj_get(&obj);  // Increment reference
kobj_put(&obj);  // Decrement reference

// INCORRECT: Custom reference counting
// NEVER implement custom memory management
```

### 3. Iteration Patterns

Safe iteration requires proper lock acquisition:

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

### 4. Error Handling

All kobj-safe operations return error codes:

```c
// Check return values
if (list_add_safe(&my_list, &new_entry) != 0) {
    // Handle allocation or locking failure
    return -ENOMEM;
}
```

### 5. Owner Object Requirements

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

## Code Duplication Prevention

### Prohibited Practices

The following practices are STRICTLY FORBIDDEN:

#### Custom Synchronization Primitives
```c
// FORBIDDEN: Custom spinlock implementation
struct my_custom_lock {
    volatile int locked;
};

// FORBIDDEN: Custom atomic operations
static inline void my_custom_lock(struct my_custom_lock *lock) {
    // Custom implementation
}
```

#### Custom Memory Management
```c
// FORBIDDEN: Custom allocation schemes
void *my_custom_alloc(size_t size) {
    // Custom implementation
}

// FORBIDDEN: Custom reference counting
struct my_object {
    int ref_count;  // Should use kobj reference counting
};
```

#### Custom Data Structures
```c
// FORBIDDEN: Custom list implementation
struct my_custom_list {
    struct my_node *head;
    // Custom implementation
};

// FORBIDDEN: Duplicate functionality
int my_list_add(struct my_custom_list *list, void *data) {
    // Duplicates existing list functionality
}
```

### Required Practices

#### Use Existing Core Libraries
```c
// REQUIRED: Use existing synchronization
#include "sync.h"
struct spinlock lock;
spinlock_init(&lock);

// REQUIRED: Use existing kobj system
#include "kobj.h"
struct kobj obj;
kobj_init(&obj, KOBJ_SERVICE);

// REQUIRED: Use existing data structures
#include "lists.h"
struct list list;
list_init(&list, &owner_kobj);
```

#### Proper Library Layering
```c
// lib/kobj.c MUST use lib/spinlock.c functions
#include "sync.h"  // For spinlock functions

// lib/lists.c MUST use lib/spinlock.c functions  
#include "sync.h"  // For spinlock functions

// kernel/*.c MUST use lib/*.c functions
#include "kobj.h"   // For kobj functions
#include "lists.h"  // For data structure functions
```

## Subsystem Requirements

### Console Subsystem
- **MUST** use `list` for console driver registration
- **MUST** provide kobj-based console driver entries
- **MUST** use proper locking for console multiplexing

### Memory Management
- **MUST** use `list` for VMA management
- **MUST** use `list` for free page tracking
- **MUST** integrate with kobj pools for allocation

### Process Management
- **MUST** use `fmi_btree` for process PID lookup
- **MUST** use `list` for process groups and sessions
- **MUST** provide thread-safe process tree operations

### Thread Management
- **SHOULD** use `list` for scheduler queues where beneficial
- **MUST** integrate with kobj system for thread objects
- **MUST** provide proper lock ordering with process management

## Testing Requirements

### Unit Testing
- **MUST** test all safe data structure operations
- **MUST** verify proper error handling and cleanup
- **MUST** test lock acquisition and release patterns
- **MUST** verify no code duplication across library modules

### Property-Based Testing
- **MUST** verify concurrent safety properties
- **MUST** test reference counting correctness
- **MUST** verify no memory leaks or double-frees
- **MUST** test proper library layering and function usage

### Integration Testing
- **MUST** test cross-subsystem lock ordering
- **MUST** verify performance under concurrent load
- **MUST** test proper cleanup during system shutdown
- **MUST** verify no duplicate functionality across modules

## Performance Considerations

### Read-Heavy Workloads
- Lists use read-write locks allowing concurrent readers
- Hash tables provide per-bucket locking for reduced contention
- B-trees support concurrent searches with minimal locking

### Write-Heavy Workloads
- Object pools reduce allocation overhead
- Atomic operations minimize lock contention
- Proper lock granularity prevents unnecessary blocking

### Memory Efficiency
- Kobj pools reuse objects rather than frequent malloc/free
- Slab allocator integration optimizes memory layout
- Reference counting prevents premature deallocation

### Code Efficiency
- No duplicate implementations reduce maintenance overhead
- Consistent APIs across all subsystems improve developer productivity
- Shared library functions reduce binary size and improve cache locality

## Migration Strategy

### Phase 1: Core Infrastructure
1. Implement thread-safe data structure library
2. Update console subsystem to use fmi_list
3. Update memory management to use fmi_list

### Phase 2: Process Management
1. Convert process tree to fmi_btree
2. Update process groups to use fmi_list
3. Verify proper lock ordering

### Phase 3: Code Deduplication
1. Audit all library code for duplicate functionality
2. Refactor duplicate code to use existing core libraries
3. Verify proper library layering and dependencies

### Phase 4: Validation and Optimization
1. Run comprehensive test suite
2. Performance benchmarking and optimization
3. Documentation updates and code review

## Compliance Verification

All kernel subsystems MUST demonstrate:

1. **Type Safety**: All data structures use kobj headers
2. **Thread Safety**: All operations are properly synchronized
3. **Lock Ordering**: No deadlock potential in lock acquisition
4. **Memory Safety**: No leaks or use-after-free conditions
5. **Performance**: Acceptable overhead for safety guarantees
6. **Code Reuse**: No duplicate functionality across library modules
7. **Library Layering**: Proper use of existing core library functions

This approach ensures that FMI/OS maintains both safety and performance while providing a solid foundation for concurrent kernel operations. The prevention of code duplication ensures maintainability, consistency, and reduces the potential for bugs across the kernel codebase.