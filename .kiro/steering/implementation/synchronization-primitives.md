# Low-Level Synchronization Primitives

This document describes the approach for implementing synchronization primitives in the FMI/OS kernel.

## Design Philosophy

The kernel synchronization primitives are designed as **simple, atomic-based locks** that do not depend on thread management infrastructure. This creates a clean layering where:

1. **Lowest Level**: Atomic-based synchronization primitives (spinlocks only)
2. **Middle Level**: Thread management system with thread-aware synchronization (mutexes, semaphores, condition variables)
3. **Higher Level**: POSIX-style blocking synchronization (built on thread management)

**Core Library Principle**: All kernel code MUST use the synchronization functions provided in `lib/spinlock.c`, `lib/condvar.c`, and `lib/waitqueue.c`. Custom synchronization implementations are STRICTLY FORBIDDEN to prevent code duplication and ensure consistency.

**Naming Convention Compliance**: All synchronization types MUST follow the established naming conventions with `_t` suffixes for types and `_s` suffixes for typedef'd structs. See `naming-conventions.md` for detailed requirements.

## Implementation Approach

### What These Primitives ARE
- Simple atomic operations using `stdatomic.h`
- Busy-waiting (spinning) implementations
- Self-contained with no external dependencies
- Suitable for use in interrupt handlers and early kernel initialization
- Foundation for building higher-level synchronization

### What These Primitives are NOT
- POSIX-compliant blocking synchronization
- Thread-aware or priority-inheritance enabled
- Dependent on scheduler or thread management
- Suitable for long-duration locks in user code

## Specific Primitive Behaviors

### Spinlocks (`spinlock_t`)
- Pure atomic flag-based implementation
- Busy-wait with CPU pause instructions
- Suitable for short critical sections
- No thread ownership tracking

### Mutexes (`mutex_t`) 
- **NOT IMPLEMENTED** at this level
- Requires thread management infrastructure for proper blocking behavior
- Should provide owner tracking and priority inheritance
- Will be implemented as part of the threading framework

### Semaphores (`semaphore_t`)
- **NOT IMPLEMENTED** at this level
- Requires thread management infrastructure for proper blocking behavior
- Should provide resource counting with thread blocking/wakeup
- Will be implemented as part of the threading framework

### Condition Variables (`condvar_t`)
- **NOT IMPLEMENTED** at this level
- Requires thread management infrastructure
- Placeholder functions that do nothing

### Wait Queues (`wait_queue_t`)
- **NOT IMPLEMENTED** at this level  
- Requires thread management infrastructure
- Placeholder functions that do nothing

## Architectural Separation

### Low-Level Primitives (This Layer)
- **Spinlocks**: For very short critical sections, interrupt handlers, early kernel initialization
- **Characteristics**: Busy-waiting, no thread awareness, no blocking

### Thread-Level Synchronization (Threading Framework)
- **Mutexes**: With owner tracking, priority inheritance, proper blocking
- **Semaphores**: Resource counting with thread blocking/wakeup mechanisms
- **Condition Variables**: Thread signaling and waiting
- **Characteristics**: Thread-aware, blocking behavior, scheduler integration

## Usage Guidelines

### Mandatory Core Library Usage

All kernel code MUST use the provided synchronization libraries:

```c
// REQUIRED: Use lib/spinlock.c functions
#include "sync.h"
spinlock_t my_lock;
spinlock_init(&my_lock);
spin_lock(&my_lock);
// ... critical section ...
spin_unlock(&my_lock);

// FORBIDDEN: Custom synchronization implementations
// NEVER implement your own spinlock or other sync primitives
```

### When to Use These Primitives
- Early kernel initialization before thread system is available
- Interrupt service routines
- Critical sections protecting simple data structures
- Building blocks for higher-level synchronization

### When NOT to Use These Primitives
- Long-duration locks (use thread-level mutexes and semaphores)
- User-space code (use POSIX synchronization)
- When priority inheritance is required (use thread-level mutexes)
- When resource counting is needed (use thread-level semaphores)
- When fairness guarantees are needed

### Performance Considerations
- These primitives use busy-waiting, consuming CPU cycles
- Suitable only for short critical sections
- Higher-level blocking primitives should be used for longer waits

### Code Duplication Prevention

The following practices are STRICTLY FORBIDDEN:

#### Custom Synchronization Primitives
```c
// FORBIDDEN: Custom spinlock implementation
struct my_spinlock {
    volatile int locked;
};

void my_spin_lock(struct my_spinlock *lock) {
    // Custom implementation - NEVER DO THIS
}

// FORBIDDEN: Custom atomic operations
static inline int my_atomic_compare_exchange(volatile int *ptr, int expected, int desired) {
    // Custom implementation - NEVER DO THIS
}
```

#### Duplicate Functionality
```c
// FORBIDDEN: Reimplementing existing sync functions
void my_custom_spinlock_init(struct my_spinlock *lock) {
    // Duplicates spinlock_init() - NEVER DO THIS
}
```

### Required Library Integration

All kernel subsystems must demonstrate proper library usage:

```c
// lib/kobj.c MUST use lib/spinlock.c
#include "sync.h"
struct spinlock pool_lock;
spinlock_init(&pool_lock);

// lib/lists.c MUST use lib/spinlock.c
#include "sync.h"  
struct spinlock list_lock;
spinlock_init(&list_lock);

// kernel/*.c MUST use lib/sync.c functions
#include "sync.h"
struct spinlock subsystem_lock;
spinlock_init(&subsystem_lock);
```

## Testing Strategy

### Unit Testing
- Test basic lock/unlock functionality for spinlocks
- Test try-lock operations
- Verify proper CPU pause behavior during busy-waiting

### Property-Based Testing
- Verify mutual exclusion properties for spinlocks
- Test atomic operation correctness
- Verify no data races in concurrent scenarios

### Integration Testing
- Test usage in multi-threaded test environment
- Verify no deadlocks in simple scenarios
- Test performance characteristics

## Future Evolution

As the kernel develops, these low-level primitives will serve as the foundation for:

1. **Thread-Aware Synchronization**: Mutexes and semaphores implemented in the threading framework
   - Proper blocking behavior instead of busy-waiting
   - Owner tracking and priority inheritance for mutexes
   - Resource counting with thread queuing for semaphores
2. **POSIX Compliance**: Full POSIX synchronization API implementation
3. **Debugging Support**: Lock tracking, deadlock detection, performance monitoring

The key principle is that these low-level primitives remain simple and dependency-free, while thread-aware synchronization (mutexes, semaphores, condition variables) is implemented at the threading layer where proper blocking, scheduling integration, and advanced features can be provided.