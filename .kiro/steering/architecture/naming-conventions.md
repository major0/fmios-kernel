---
inclusion: always
---

# Naming Conventions

This document establishes the naming conventions for all code in the FMI/OS project.

## Core Principles

1. **No Namespace Prefixes**: Since the kernel compiles with `-ffreestanding`, there's no risk of namespace conflicts with standard C library functions. Therefore, no `fmi_` or similar prefixes are necessary.

2. **Consistent Type Naming**: All data types follow a standard convention to improve code readability and maintainability.

3. **Clear Struct Naming**: Structs that are typedef'd follow a specific pattern to distinguish the struct name from the typedef name.

4. **Standard Name Conflicts**: Data types and functions whose names overlap with C or POSIX standard types/functions MUST be prefixed with `k` to distinguish kernel implementations from standard library equivalents.

5. **Function Naming Convention**: All interface functions MUST follow the `<topic>_<verb>()` naming pattern to provide consistent and descriptive function names.

## Type Naming Conventions

### Rule 1: All Data Types Use `_t` Suffix

All typedef'd data types MUST use the `_t` suffix:

```c
// CORRECT: Type names with _t suffix
typedef struct kobj_s kobj_t;
typedef struct spinlock_s spinlock_t;
typedef struct list_s list_t;
typedef struct hash_table_s hash_table_t;
typedef struct btree_s btree_t;
typedef struct process_s process_t;
typedef struct thread_s thread_t;

// INCORRECT: Type names without _t suffix
typedef struct kobj_s kobj;
typedef struct spinlock_s spinlock;
```

### Rule 2: Typedef'd Structs Use `_s` Suffix

When a struct is typedef'd to create a type, the struct name MUST use the `_s` suffix:

```c
// CORRECT: Struct with _s suffix, typedef with _t suffix
struct kobj_s {
    enum kobj_type type;
    atomic_t ref_count;
    // ...
};
typedef struct kobj_s kobj_t;

// INCORRECT: Struct without _s suffix
struct kobj {
    enum kobj_type type;
    // ...
};
typedef struct kobj kobj_t;
```

### Rule 3: Standard Name Conflicts Use `k` Prefix

Data types and functions whose names would conflict with C or POSIX standard names MUST use the `k` prefix:

```c
// CORRECT: Kernel versions of standard functions
int kprintf(const char *format, ...);
void *kmalloc(size_t size);
void kfree(void *ptr);
int kopen(const char *pathname, int flags);
ssize_t kread(int fd, void *buf, size_t count);
ssize_t kwrite(int fd, const void *buf, size_t count);

// CORRECT: Kernel versions of standard types that might conflict
typedef struct kmutex_s kmutex_t;  // If different from POSIX mutex
typedef struct ksem_s ksem_t;      // If different from POSIX semaphore

// INCORRECT: Using standard names directly in kernel
int printf(const char *format, ...);  // Conflicts with standard printf
void *malloc(size_t size);            // Conflicts with standard malloc
```

### Rule 4: Non-Typedef'd Structs Have No Suffix

Structs that are NOT typedef'd should use plain names without suffixes:

```c
// CORRECT: Plain struct name (not typedef'd)
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

// Usage: struct list_head my_list;

// INCORRECT: Adding _s suffix to non-typedef'd struct
struct list_head_s {
    struct list_head_s *next;
    struct list_head_s *prev;
};
```

## Naming Pattern Examples

### Synchronization Primitives

```c
// Spinlock
struct spinlock_s {
    atomic_flag locked;
    atomic_int owner_cpu;
};
typedef struct spinlock_s spinlock_t;

// Usage
spinlock_t my_lock;
spinlock_init(&my_lock);
```

### Kernel Objects

```c
// Kernel Object
struct kobj_s {
    enum kobj_type type;
    atomic_t ref_count;
    spinlock_t *obj_lock;
    struct kobj_pool *pool;
    uint32_t magic;
    struct list_head pool_list;
};
typedef struct kobj_s kobj_t;

// Usage
kobj_t my_obj;
kobj_init(&my_obj, KOBJ_SERVICE);
```

### Data Structures

```c
// Thread-safe list
struct list_s {
    struct list_head head;
    spinlock_t *list_lock;
    size_t count;
    kobj_t *owner;
};
typedef struct list_s list_t;

// Usage
list_t my_list;
list_init(&my_list, &owner_kobj);

// Hash table
struct hash_table_s {
    list_t *buckets;
    size_t bucket_count;
    kobj_t *owner;
    size_t total_entries;
};
typedef struct hash_table_s hash_table_t;

// Usage
hash_table_t my_table;
hash_table_init(&my_table, 256, &owner_kobj);

// B-tree
struct btree_s {
    struct btree_node *root;
    int height;
    size_t node_count;
    spinlock_t *tree_lock;
    kobj_t *owner;
    struct kobj_pool *node_pool;
};
typedef struct btree_s btree_t;

// Usage
btree_t my_tree;
btree_init(&my_tree, &owner_kobj);
```

### Process and Thread Types

```c
// Process
struct process_s {
    kobj_t kobj;
    pid_t pid;
    pid_t ppid;
    struct process_s *parent;
    // ...
};
typedef struct process_s process_t;

// Thread
struct thread_s {
    kobj_t kobj;
    tid_t tid;
    process_t *process;
    // ...
};
typedef struct thread_s thread_t;

// Process Group
struct process_group_s {
    kobj_t kobj;
    pgid_t pgid;
    // ...
};
typedef struct process_group_s process_group_t;
```

## Function Naming Conventions

### Rule 5: Interface Functions Use `<topic>_<verb>()` Pattern

All interface functions MUST follow the `<topic>_<verb>()` naming pattern for consistency and clarity:

```c
// CORRECT: <topic>_<verb>() pattern
void *pages_alloc(size_t pages);        // Page allocator: pages_alloc()
void pages_free(void *addr, size_t pages); // Page allocator: pages_free()
int pages_init(void);                    // Page allocator: pages_init()

void *slab_alloc(slab_cache_t *cache);   // Slab allocator: slab_alloc()
void slab_free(slab_cache_t *cache, void *obj); // Slab allocator: slab_free()

int proc_create(process_t *parent);      // Process management: proc_create()
void proc_destroy(process_t *proc);      // Process management: proc_destroy()

// INCORRECT: Inconsistent naming patterns
void *alloc_pages(size_t pages);         // Should be pages_alloc()
void free_pages(void *addr, size_t pages); // Should be pages_free()
void *arch_alloc_pages(size_t pages);    // Should be pages_alloc()
```

### Rule 6: Functions Use Descriptive Names Without Prefixes

Functions should use clear, descriptive names that indicate their purpose. However, functions that would conflict with C or POSIX standard functions MUST use the `k` prefix:

```c
// CORRECT: Clear function names (no conflicts)
void spinlock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

void kobj_init(kobj_t *obj, enum kobj_type type);
void kobj_get(kobj_t *obj);
void kobj_put(kobj_t *obj);

// CORRECT: Kernel functions that would conflict with standard library
int kprintf(const char *format, ...);
void *kmalloc(size_t size);
void kfree(void *ptr);
int kopen(const char *pathname, int flags);
ssize_t kread(int fd, void *buf, size_t count);
ssize_t kwrite(int fd, const void *buf, size_t count);
int kclose(int fd);

// INCORRECT: Using standard names that could cause confusion
int printf(const char *format, ...);   // Should be kprintf
void *malloc(size_t size);             // Should be kmalloc
void free(void *ptr);                  // Should be kfree

// INCORRECT: Unnecessary prefixes for non-conflicting names
void fmi_spinlock_init(spinlock_t *lock);
void fmi_kobj_init(kobj_t *obj, enum kobj_type type);
```

### Rule 7: Type-Specific Operations Use Type Name Prefix

Operations on specific types should use the type name (without `_t`) as a prefix:

```c
// Spinlock operations
void spinlock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
bool spin_trylock(spinlock_t *lock);

// Kernel object operations
void kobj_init(kobj_t *obj, enum kobj_type type);
void kobj_get(kobj_t *obj);
void kobj_put(kobj_t *obj);
bool kobj_validate(kobj_t *obj);

// List operations
int list_init(list_t *list, kobj_t *owner);
void list_destroy(list_t *list);
int list_add_safe(list_t *list, struct list_head *entry);
```

## Standard Library Conflict Resolution

### When to Use `k` Prefix

The `k` prefix MUST be used for kernel implementations that would otherwise conflict with C standard library or POSIX function names:

#### Memory Management Functions
```c
// Kernel memory management (conflicts with stdlib.h)
void *kmalloc(size_t size);
void *kcalloc(size_t nmemb, size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);
```

#### I/O Functions
```c
// Kernel I/O functions (conflicts with unistd.h, stdio.h)
int kprintf(const char *format, ...);
int ksprintf(char *str, const char *format, ...);
int ksnprintf(char *str, size_t size, const char *format, ...);
int kopen(const char *pathname, int flags);
ssize_t kread(int fd, void *buf, size_t count);
ssize_t kwrite(int fd, const void *buf, size_t count);
int kclose(int fd);
```

#### String Functions
```c
// Kernel string functions (conflicts with string.h)
size_t kstrlen(const char *s);
char *kstrcpy(char *dest, const char *src);
char *kstrncpy(char *dest, const char *src, size_t n);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, size_t n);
```

#### Threading and Synchronization (if different from POSIX)
```c
// Only if kernel implementation differs significantly from POSIX
typedef struct kmutex_s kmutex_t;      // If not using POSIX mutex semantics
typedef struct ksem_s ksem_t;          // If not using POSIX semaphore semantics
typedef struct kthread_s kthread_t;    // If not using POSIX thread semantics
```

### When NOT to Use `k` Prefix

Do NOT use the `k` prefix for kernel-specific types and functions that have no standard library equivalent:

```c
// CORRECT: No conflicts with standard library
typedef struct kobj_s kobj_t;          // No standard equivalent
typedef struct spinlock_s spinlock_t;  // No standard equivalent
typedef struct list_s list_t;          // No standard equivalent
typedef struct btree_s btree_t;        // No standard equivalent

void kobj_init(kobj_t *obj, enum kobj_type type);  // No standard equivalent
void spinlock_init(spinlock_t *lock);              // No standard equivalent
void list_add_safe(list_t *list, struct list_head *entry);  // No standard equivalent
```

## Rationale

### Why `_t` Suffix for Types?

The `_t` suffix clearly indicates that a name represents a type rather than a variable or function. This convention:
- Improves code readability
- Follows POSIX conventions (e.g., `size_t`, `pid_t`, `pthread_t`)
- Makes type names immediately recognizable
- Prevents confusion between types and variables

### Why `_s` Suffix for Typedef'd Structs?

The `_s` suffix on struct names:
- Distinguishes the struct name from the typedef name
- Allows forward declarations using the struct name
- Provides flexibility when both forms are needed
- Follows common C conventions (e.g., `struct pthread_s` / `pthread_t`)

### Why `k` Prefix for Standard Conflicts?

The `k` prefix for conflicting names:
- Clearly identifies kernel implementations vs. standard library functions
- Prevents confusion when reading code about which implementation is being used
- Allows kernel to provide different semantics than standard library equivalents
- Maintains consistency with common kernel naming practices (Linux kernel uses similar approach)
- Enables easier debugging by making kernel vs. standard library calls obvious

### Why No Namespace Prefixes?

Since the kernel compiles with `-ffreestanding`:
- No standard C library functions are available
- No risk of namespace conflicts
- Shorter, cleaner names improve readability
- Reduces unnecessary verbosity in the codebase

## Migration Guidelines

When updating existing code to follow these conventions:

1. **Identify all typedef'd types** and add `_t` suffix
2. **Update struct definitions** for typedef'd structs to use `_s` suffix
3. **Identify standard library conflicts** and add `k` prefix where needed
4. **Update all usage sites** to use the new type names and function names
5. **Update function declarations** to use the new naming conventions
6. **Update documentation** to reflect the new naming conventions

### Standard Library Function Usage Guidelines

**REQUIRED: Use Standard Library Functions Directly**

The kernel MUST use compiler-provided standard library functions for basic operations:

```c
// CORRECT: Use standard library functions directly
#include <string.h>  // Compiler-provided header

// Memory operations (REQUIRED)
memset(ptr, 0, size);                    // Use standard memset
memcpy(dest, src, size);                 // Use standard memcpy
memcmp(ptr1, ptr2, size);                // Use standard memcmp
memmove(dest, src, size);                // Use standard memmove

// Safe string operations (REQUIRED)
strlen(str);                             // Use standard strlen
strncpy(dest, src, sizeof(dest) - 1);   // Use standard strncpy (safe)
strncat(dest, src, remaining_space);     // Use standard strncat (safe)
strcmp(s1, s2);                          // Use standard strcmp
strncmp(s1, s2, n);                      // Use standard strncmp

// Safe formatted output (REQUIRED)
snprintf(buf, sizeof(buf), fmt, ...);    // Use standard snprintf (safe)
vsnprintf(buf, sizeof(buf), fmt, args);  // Use standard vsnprintf (safe)
```

**PROHIBITED: Custom String/Memory Function Implementations**

```c
// INCORRECT: Custom implementations are prohibited
size_t kstrlen(const char *s);           // PROHIBITED - use strlen()
char *kstrcpy(char *dest, const char *src);  // PROHIBITED - use strncpy()
char *kstrncpy(char *dest, const char *src, size_t n);  // PROHIBITED - use strncpy()
int kstrcmp(const char *s1, const char *s2);  // PROHIBITED - use strcmp()
int kstrncmp(const char *s1, const char *s2, size_t n);  // PROHIBITED - use strncmp()
void *kmemset(void *s, int c, size_t n);     // PROHIBITED - use memset()
void *kmemcpy(void *dest, const void *src, size_t n);  // PROHIBITED - use memcpy()
```

### Functions Requiring `k` Prefix (Memory Management Only)

The `k` prefix is ONLY required for kernel-specific memory management functions that have different semantics than standard library equivalents:

```c
// Memory management functions (conflicts with stdlib.h)
void *kmalloc(size_t size);              // Kernel allocator (different from malloc)
void *kcalloc(size_t nmemb, size_t size); // Kernel allocator (different from calloc)
void *krealloc(void *ptr, size_t size);   // Kernel allocator (different from realloc)
void kfree(void *ptr);                    // Kernel allocator (different from free)

// I/O functions (conflicts with stdio.h/unistd.h)
int kprintf(const char *format, ...);     // Kernel console output
int ksprintf(char *str, const char *format, ...);  // DEPRECATED - use snprintf
int ksnprintf(char *str, size_t size, const char *format, ...);  // Kernel-specific formatting
int kopen(const char *pathname, int flags);  // Kernel file operations
ssize_t kread(int fd, void *buf, size_t count);  // Kernel file operations
ssize_t kwrite(int fd, const void *buf, size_t count);  // Kernel file operations
int kclose(int fd);                        // Kernel file operations
```

### Migration Guidelines for Existing Code

When updating existing code to follow these guidelines:

1. **Remove custom string/memory implementations**: Delete `kstring.c`, custom `kmemory.c`, etc.
2. **Replace custom function calls**: Update all `kstrlen()` → `strlen()`, `kstrcpy()` → `strncpy()`, etc.
3. **Add proper includes**: Ensure `#include <string.h>` is present where needed
4. **Update unsafe calls**: Replace `strcpy()` → `strncpy()`, `sprintf()` → `snprintf()`
5. **Keep kernel-specific functions**: Retain `kmalloc()`, `kprintf()`, etc. that have different semantics

## Compliance Verification

All code MUST demonstrate:

1. **Type Naming**: All typedef'd types use `_t` suffix
2. **Struct Naming**: All typedef'd structs use `_s` suffix in struct name
3. **Standard Library Usage**: Direct use of standard library functions for string/memory operations
4. **No Custom Implementations**: No custom string or memory function implementations
5. **Kernel-Specific Prefixes**: `k` prefix only for functions with different semantics (memory management, I/O)
6. **Consistency**: Naming conventions applied uniformly across all code
7. **No Unnecessary Prefixes**: No namespace prefixes (e.g., `fmi_`) except `k` for true conflicts
8. **Clarity**: Names clearly indicate their purpose and type

### Build System Enforcement

The build system MUST detect and prevent:

```bash
# Detect prohibited custom implementations
grep -r "kstr\(len\|cpy\|ncpy\|cmp\|ncmp\)" --include="*.c" --include="*.h" . && \
echo "ERROR: Custom string functions found - use standard library"

grep -r "k\(memset\|memcpy\|memcmp\|memmove\)" --include="*.c" --include="*.h" . && \
echo "ERROR: Custom memory functions found - use standard library"

# Detect unsafe function usage
grep -r "\b\(strcpy\|strcat\|sprintf\|vsprintf\|gets\)\s*(" --include="*.c" . && \
echo "ERROR: Unsafe functions found - use safe alternatives"
```

This naming convention ensures consistency, readability, and maintainability across the entire FMI/OS codebase while leveraging compiler-provided standard library functions and avoiding unnecessary custom implementations.
