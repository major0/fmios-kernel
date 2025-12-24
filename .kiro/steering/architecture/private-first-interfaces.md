---
inclusion: always
---

# Private-First Interface Design

This document establishes the principle of private-first interface design for all FMI/OS kernel subsystems, ensuring minimal public exposure and maximum encapsulation.

## Core Principle

**All subsystem interfaces MUST be private (static) by default. Public interfaces are only created when there is an explicit, actionable need for external access.**

This principle prevents:
- Circular dependencies between subsystems
- Uncontrolled interface proliferation
- Tight coupling between modules
- Accidental exposure of implementation details

## Implementation Rules

### Rule 1: Static-by-Default Functions

All functions in kernel subsystems MUST be declared `static` unless there is a specific need for external access:

```c
// CORRECT: Private function by default
static int slab_cache_create(const char *name, size_t obj_size, size_t align,
                            void *(*ctor)(void *), void (*dtor)(void *));

// INCORRECT: Public function without justification
int slab_cache_create(const char *name, size_t obj_size, size_t align,
                     void *(*ctor)(void *), void (*dtor)(void *));
```

### Rule 2: Minimal Public Headers

Public headers MUST contain only:
1. **Setup/initialization functions** called from `kernel/main.c`
2. **Essential types** needed by other subsystems
3. **Core operations** with proven external need

```c
// CORRECT: Minimal public interface in include/slab.h
#ifndef _SLAB_H
#define _SLAB_H

#include "ktypes.h"

/* Only the setup function is exposed */
int slab_subsystem_setup(void);

#endif /* _SLAB_H */
```

```c
// INCORRECT: Exposing internal implementation
#ifndef _SLAB_H
#define _SLAB_H

#include "ktypes.h"

/* Internal structures exposed - WRONG */
struct slab_cache_s {
    size_t obj_size;
    /* ... internal details ... */
};

/* Internal functions exposed - WRONG */
slab_cache_t *slab_cache_create(const char *name, size_t obj_size, size_t align,
                                void *(*ctor)(void *), void (*dtor)(void *));
void slab_cache_destroy(slab_cache_t *cache);

#endif /* _SLAB_H */
```

### Rule 3: Opaque Type Declarations

When external code needs to reference internal types, use opaque forward declarations:

```c
// CORRECT: Opaque forward declaration in include/kobj.h
struct kobj_pool_s;  /* Implementation hidden in kernel/kobject.c */

struct kobj_s {
    enum kobj_type type;
    atomic_t ref_count;
    struct kobj_pool_s *pool;  /* Opaque pointer */
    /* ... */
};
```

```c
// INCORRECT: Exposing full structure definition
struct kobj_pool_s {
    enum kobj_type type;        /* Internal details exposed - WRONG */
    size_t obj_size;
    size_t pool_size;
    /* ... all internal implementation details ... */
};
```

### Rule 4: Internal Structure Definitions

All internal structures MUST be defined in the implementation file, not headers:

```c
// CORRECT: Internal structure in kernel/slab.c
struct slab_cache_s {
    char name[32];              /* Private implementation details */
    size_t obj_size;
    size_t align;
    /* ... */
};
typedef struct slab_cache_s slab_cache_t;  /* Local typedef */
```

### Rule 5: Justification for Public Interfaces

Before making any function public (non-static), you MUST provide explicit justification:

**Acceptable justifications:**
- Called from `kernel/main.c` for subsystem initialization
- Core operation needed by multiple other subsystems
- Part of a well-defined public API (e.g., basic kobj operations)

**Unacceptable justifications:**
- "Might be useful later"
- "Other code could use this"
- "It's a utility function"
- "For debugging purposes"

## Subsystem Organization Patterns

### Pattern 1: Kernel Subsystems

```
kernel/subsystem.c:
├── Internal structures (static)
├── Internal functions (static)
├── Filesystem interface functions (static)
└── Public setup function (non-static)

include/subsystem.h:
└── Setup function declaration only
```

### Pattern 2: Library Modules

```
lib/module.c:
├── Internal helper functions (static)
└── Core operations (non-static, justified)

include/module.h:
├── Essential types
├── Core operation declarations
└── Setup function declaration
```

### Pattern 3: Architecture-Specific Code

```
arch/x86_64/module.c:
├── All functions (static by default)
└── Architecture interface functions (non-static, justified)

arch/x86_64/include/arch.h:
└── Architecture interface declarations only
```

## Enforcement Guidelines

### Code Review Checklist

For every non-static function, verify:
- [ ] Is there explicit justification for public exposure?
- [ ] Is it called from multiple source files?
- [ ] Could it be made static instead?
- [ ] Does it expose implementation details?

For every public header, verify:
- [ ] Contains only essential declarations
- [ ] No internal structure definitions
- [ ] No "convenience" functions
- [ ] No debugging or utility functions

### Build System Integration

The build system SHOULD enforce these rules by:
1. **Scanning for unjustified public functions**
2. **Detecting exposed internal structures**
3. **Flagging headers with too many declarations**
4. **Requiring justification comments for public interfaces**

## Examples of Correct Implementation

### Slab Allocator (Completely Private)

```c
// include/slab.h - Minimal public interface
#ifndef _SLAB_H
#define _SLAB_H
#include "ktypes.h"
int slab_subsystem_setup(void);  /* Only setup function exposed */
#endif

// kernel/slab.c - All implementation private
struct slab_cache_s { /* Internal structure */ };
static slab_cache_t *slab_cache_create(...) { /* Private function */ }
static void slab_cache_destroy(...) { /* Private function */ }
int slab_subsystem_setup(void) { /* Public setup function */ }
```

### Kernel Objects (Minimal Public Interface)

```c
// include/kobj.h - Essential operations only
struct kobj_pool_s;  /* Opaque forward declaration */
int kobj_init(kobj_t *obj, enum kobj_type type);  /* Essential operation */
void kobj_get(kobj_t *obj);                       /* Essential operation */
void kobj_put(kobj_t *obj);                       /* Essential operation */
int kobj_subsystem_setup(void);                   /* Setup function */

// kernel/kobject.c - Pool management completely private
struct kobj_pool_s { /* Full definition - private */ };
static struct kobj_pool_s *kobj_pool_create(...) { /* Private */ }
static void kobj_pool_destroy(...) { /* Private */ }
```

## Benefits of Private-First Design

### Maintainability
- Clear separation between public API and implementation
- Freedom to change internal implementation without breaking external code
- Reduced cognitive load when understanding subsystem interfaces

### Reliability
- Prevents circular dependencies
- Reduces coupling between subsystems
- Makes testing easier by isolating functionality

### Security
- Minimizes attack surface by reducing exposed interfaces
- Prevents accidental misuse of internal functions
- Enforces proper access patterns

### Performance
- Enables aggressive compiler optimizations for static functions
- Reduces symbol table size
- Allows for better dead code elimination

## Migration Strategy

When updating existing code to follow private-first design:

1. **Audit all public functions** - Identify which are actually needed externally
2. **Make functions static** - Start with functions that are only used within their module
3. **Remove from headers** - Clean up public headers to contain only essential declarations
4. **Create opaque types** - Replace exposed structures with forward declarations
5. **Document justifications** - Add comments explaining why remaining public functions are necessary

## Compliance Verification

All kernel subsystems MUST demonstrate:

1. **Minimal Public Interface**: Headers contain only essential declarations
2. **Static-by-Default**: All functions are static unless justified
3. **Opaque Types**: Internal structures are not exposed in headers
4. **Clear Separation**: Public API is distinct from implementation details
5. **Justified Exposure**: Every public function has documented justification

This private-first approach ensures that FMI/OS maintains clean, maintainable, and secure interfaces while preventing the architectural problems that arise from over-exposed subsystem internals.