---
inclusion: always
---

# Standard Library Usage Guidelines

This document establishes the mandatory use of compiler-provided standard library functions in the FMI/OS kernel and explicitly prohibits custom implementations of standard functionality.

## Core Principle

**The kernel MUST use compiler-provided standard library functions for all basic string and memory operations. Custom implementations of standard library functions are STRICTLY PROHIBITED.**

## Rationale

### Why Use Standard Library Functions

1. **Compiler Optimization**: Standard library functions are heavily optimized by compiler vendors
2. **Security**: Well-tested implementations with known security characteristics
3. **Maintenance**: No need to maintain custom implementations of standard functionality
4. **Consistency**: Uniform behavior across different compiler and architecture combinations
5. **Freestanding Environment**: Compiler-provided headers are safe in `-ffreestanding` mode

### Why Prohibit Custom Implementations

1. **Code Duplication**: Custom implementations duplicate well-tested standard functionality
2. **Security Risks**: Custom implementations may introduce bugs or vulnerabilities
3. **Maintenance Burden**: Additional code to maintain, test, and debug
4. **Performance**: Compiler-provided functions are typically more optimized
5. **Standards Compliance**: Standard functions follow well-defined specifications

## Required Standard Library Functions

### Memory Operations (REQUIRED)

```c
#include <string.h>  // Compiler-provided header

// Memory manipulation (REQUIRED)
void *memset(void *s, int c, size_t n);              // Zero or set memory
void *memcpy(void *dest, const void *src, size_t n); // Copy memory (non-overlapping)
void *memmove(void *dest, const void *src, size_t n);// Copy memory (overlapping safe)
int memcmp(const void *s1, const void *s2, size_t n);// Compare memory

// Usage examples
memset(buffer, 0, sizeof(buffer));                   // Clear buffer
memcpy(dest, src, data_size);                        // Copy data
if (memcmp(data1, data2, size) == 0) { /* equal */ } // Compare data
```

### String Operations (REQUIRED)

```c
#include <string.h>  // Compiler-provided header

// String length (REQUIRED)
size_t strlen(const char *s);                         // Get string length

// Safe string operations (REQUIRED)
char *strncpy(char *dest, const char *src, size_t n); // Copy with limit
char *strncat(char *dest, const char *src, size_t n); // Concatenate with limit
int strcmp(const char *s1, const char *s2);          // Compare strings
int strncmp(const char *s1, const char *s2, size_t n);// Compare with limit

// Usage examples with proper bounds checking
strncpy(dest, src, sizeof(dest) - 1);                // Safe copy
dest[sizeof(dest) - 1] = '\0';                       // Ensure termination
strncat(dest, src, sizeof(dest) - strlen(dest) - 1); // Safe concatenation
```

### Formatted Output (REQUIRED)

```c
#include <stdio.h>   // For snprintf/vsnprintf (compiler-provided)

// Safe formatted output (REQUIRED)
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

// Usage examples
snprintf(buffer, sizeof(buffer), "Value: %d", value);  // Safe formatting
vsnprintf(buffer, sizeof(buffer), format, args);       // Safe formatting with va_list
```

## Prohibited Functions and Implementations

### Unsafe Standard Functions (PROHIBITED)

```c
// PROHIBITED: Unsafe standard library functions
#define strcpy(dest, src) \
  _Static_assert(0, "strcpy is prohibited - use strncpy with bounds checking")
#define strcat(dest, src) \
  _Static_assert(0, "strcat is prohibited - use strncat with bounds checking")
#define sprintf(buf, fmt, ...) \
  _Static_assert(0, "sprintf is prohibited - use snprintf with buffer size")
#define vsprintf(buf, fmt, args) \
  _Static_assert(0, "vsprintf is prohibited - use vsnprintf with buffer size")
#define gets(buf) \
  _Static_assert(0, "gets is prohibited - use fgets or other safe input")
```

### Custom String Implementations (PROHIBITED)

```c
// PROHIBITED: Custom string function implementations
#define kstrlen(s) \
  _Static_assert(0, "kstrlen is prohibited - use standard strlen")
#define kstrcpy(dest, src) \
  _Static_assert(0, "kstrcpy is prohibited - use standard strncpy with bounds checking")
#define kstrncpy(dest, src, n) \
  _Static_assert(0, "kstrncpy is prohibited - use standard strncpy")
#define kstrcmp(s1, s2) \
  _Static_assert(0, "kstrcmp is prohibited - use standard strcmp")
#define kstrncmp(s1, s2, n) \
  _Static_assert(0, "kstrncmp is prohibited - use standard strncmp")

// PROHIBITED: Custom memory function implementations
#define kmemset(s, c, n) \
  _Static_assert(0, "kmemset is prohibited - use standard memset")
#define kmemcpy(dest, src, n) \
  _Static_assert(0, "kmemcpy is prohibited - use standard memcpy")
#define kmemcmp(s1, s2, n) \
  _Static_assert(0, "kmemcmp is prohibited - use standard memcmp")
#define kmemmove(dest, src, n) \
  _Static_assert(0, "kmemmove is prohibited - use standard memmove")
```

### Prohibited Source Files

The following types of source files are PROHIBITED:

- `kstring.c` - Custom string function implementations
- `kmemory.c` - Custom memory function implementations
- `kstr*.c` - Any custom string-related implementations
- `kmem*.c` - Any custom memory operation implementations (except kernel allocators)

## Exceptions: Kernel-Specific Functions

The `k` prefix is ONLY allowed for functions that have fundamentally different semantics than their standard library counterparts:

### Memory Management (ALLOWED)

```c
// Kernel memory allocators (different semantics than standard malloc/free)
void *kmalloc(size_t size);              // Kernel slab allocator
void *kcalloc(size_t nmemb, size_t size); // Kernel zero-initialized allocator
void *krealloc(void *ptr, size_t size);   // Kernel reallocation
void kfree(void *ptr);                    // Kernel memory deallocation
```

### I/O and System Functions (ALLOWED)

```c
// Kernel I/O functions (different semantics than standard I/O)
int kprintf(const char *format, ...);     // Kernel console output
int ksnprintf(char *str, size_t size, const char *format, ...); // Kernel formatting
int kopen(const char *pathname, int flags);  // Kernel filesystem operations
ssize_t kread(int fd, void *buf, size_t count);  // Kernel file operations
ssize_t kwrite(int fd, const void *buf, size_t count); // Kernel file operations
int kclose(int fd);                        // Kernel file operations

// Kernel logging and debugging
int klogf(klog_level_t level, const char *format, ...);  // Kernel logging
void kpanic(const char *format, ...) __attribute__((noreturn)); // Kernel panic
void kassert(int condition, const char *message);  // Kernel assertions
```

## Implementation Guidelines

### Proper Header Inclusion

```c
// CORRECT: Include compiler-provided headers
#include <string.h>   // For string and memory functions
#include <stdio.h>    // For snprintf/vsnprintf (if available in freestanding)
#include <stddef.h>   // For size_t, NULL
#include <stdint.h>   // For fixed-width integer types
#include <stdbool.h>  // For bool, true, false
#include <stdatomic.h> // For atomic operations

// INCORRECT: Custom header for standard functions
#include "kstring.h"  // PROHIBITED
#include "kmemory.h"  // PROHIBITED
```

### Safe Usage Patterns

```c
// Safe string copying
void safe_string_copy(char *dest, const char *src, size_t dest_size) {
  if (dest_size > 0) {
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';  // Ensure null termination
  }
}

// Safe string concatenation
void safe_string_concat(char *dest, const char *src, size_t dest_size) {
  size_t current_len = strlen(dest);
  size_t remaining = dest_size - current_len - 1;
  if (remaining > 0) {
    strncat(dest, src, remaining);
  }
}

// Safe formatted output
void safe_format_string(char *buffer, size_t buffer_size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, buffer_size, format, args);
  va_end(args);
  buffer[buffer_size - 1] = '\0';  // Ensure null termination
}
```

## Build System Enforcement

### Automated Detection

The build system MUST detect and prevent prohibited implementations:

```bash
#!/bin/bash
# Check for prohibited custom implementations

# Detect custom string function implementations
if grep -r "kstr\(len\|cpy\|ncpy\|cmp\|ncmp\)" --include="*.c" --include="*.h" .; then
  echo "ERROR: Custom string functions found - use standard library functions"
  exit 1
fi

# Detect custom memory function implementations
if grep -r "k\(memset\|memcpy\|memcmp\|memmove\)" --include="*.c" --include="*.h" .; then
  echo "ERROR: Custom memory functions found - use standard library functions"
  exit 1
fi

# Detect unsafe function usage
if grep -r "\b\(strcpy\|strcat\|sprintf\|vsprintf\|gets\)\s*(" --include="*.c" .; then
  echo "ERROR: Unsafe functions found - use safe alternatives"
  exit 1
fi

# Detect prohibited source files
if find . -name "kstring.c" -o -name "kmemory.c" -o -name "kstr*.c"; then
  echo "ERROR: Prohibited source files found - remove custom implementations"
  exit 1
fi

echo "Standard library usage validation passed"
```

### Makefile Integration

```makefile
# Add to kernel Makefile.am
check-stdlib-usage:
  @echo "Checking standard library usage compliance..."
  @$(top_srcdir)/scripts/check-stdlib-usage.sh
  @echo "Standard library usage validation passed"

# Make it part of the build process
all-local: check-stdlib-usage
```

## Migration Strategy

### Phase 1: Remove Prohibited Files

1. **Delete custom implementations**: Remove `kstring.c`, `kmemory.c`, and related files
2. **Remove from build system**: Update `Makefile.am` to remove deleted files
3. **Remove headers**: Delete `kstring.h`, `kmemory.h`, and related headers

### Phase 2: Update Function Calls

1. **Replace custom calls**: Update all `kstrlen()` → `strlen()`, `kstrcpy()` → `strncpy()`, etc.
2. **Add proper includes**: Ensure `#include <string.h>` is present where needed
3. **Fix unsafe calls**: Replace `strcpy()` → `strncpy()`, `sprintf()` → `snprintf()`

### Phase 3: Add Safety Measures

1. **Add compile-time checks**: Include `_Static_assert` macros to prevent prohibited usage
2. **Update build validation**: Add automated checks to build system
3. **Test thoroughly**: Ensure all functionality works with standard library functions

## Compliance Requirements

All kernel code MUST demonstrate:

1. **No Custom Implementations**: Zero custom string or memory function implementations
2. **Standard Library Usage**: Direct use of compiler-provided functions
3. **Safe Function Usage**: No unsafe functions like `strcpy`, `sprintf`
4. **Proper Includes**: Correct `#include <string.h>` usage
5. **Build Validation**: Automated detection of violations
6. **Exception Compliance**: `k` prefix only for truly different semantics

This approach ensures that FMI/OS leverages well-tested, optimized standard library functions while maintaining security and avoiding unnecessary code duplication.
