# Include Path Guidelines

This document establishes the proper use of `#include` directives in the FMI/OS project with the new autoconf/automake-based build system and architecture separation.

## Core Principle

**NEVER use relative or absolute paths in `#include` directives.** All includes should rely on the `-I` compiler flags provided at build time by the autoconf/automake system.

## Directory Structure

The project follows a clean architecture separation:

```
fmios/
├── include/           # Generic headers (architecture-neutral)
│   ├── arch -> ../arch/x86_64/include  # Symlink to selected arch
│   ├── console.h      # Generic console interface
│   └── memory.h       # Generic memory management
│
├── lib/               # Generic libraries (architecture-neutral)
│   ├── arch -> ../arch/x86_64/lib      # Symlink to selected arch
│   └── c/             # Kernel C library (klibc)
│       └── klibc.h
├── arch/              # Architecture-specific code
│   ├── x86_64/
│   │   ├── include/   # x86_64-specific headers
│   │   │   ├── arch.h # Architecture definitions
│   │   │   └── mmu.h  # Memory management unit
│   │   ├── lib/       # x86_64-specific libraries
│   │   └── drivers/   # x86_64-specific drivers
│   │       ├── serial_console.c
│   │       └── video_console.c
│   ├── arm64/
│   │   ├── include/
│   │   ├── lib/
│   │   └── drivers/
│   └── riscv64/
│       ├── include/
│       ├── lib/
│       └── drivers/
├── kernel/            # Generic kernel code
├── tests/             # Test suite
└── docs/              # Documentation
```

## Correct Include Patterns

### ✅ CORRECT - Use include paths configured by autoconf/automake
```c
#include "klibc.h"           // From lib/c/ via -I flag
#include "console.h"         // From include/ via -I flag
#include "memory.h"          // From include/ via -I flag
#include "scsi/scsi.h"       // From include/scsi/ via -I flag
#include "arch/arch.h"       // From arch/x86_64/include/ via symlink
#include "arch/mmu.h"        // From arch/x86_64/include/ via symlink
#include "unity.h"           // From tests/unity/src/ via -I flag
```

### ❌ INCORRECT - Never use relative paths
```c
#include "../include/console.h"      // BAD: relative path
#include "../../lib/c/klibc.h"      // BAD: relative path
#include "../arch/x86_64/include/arch.h" // BAD: relative path
#include "./local_header.h"         // BAD: relative path
```

### ❌ INCORRECT - Never use absolute paths
```c
#include "/usr/include/stdio.h"     // BAD: absolute path
#include "/include/memory.h"        // BAD: absolute path
```

## Autoconf/Automake Configuration

The build system uses autoconf/automake to configure appropriate `-I` flags:

### Configure Options
```bash
./configure --target=x86_64-fmios --enable-serial-console --enable-video-console
./configure --target=aarch64-fmios --disable-debug-console
./configure --target=riscv64-fmios --enable-scsi-subsystem
```

### Generated Makefile Flags
```makefile
# Configured by autoconf/automake
CFLAGS += -Iinclude                    # Generic headers
CFLAGS += -Ilib/c                      # Kernel C library
CFLAGS += -Iarch/$(target_arch)/include # Architecture-specific headers

# Linker flags
LDFLAGS += -Llib/c                     # Kernel C library
LDFLAGS += -Llib/arch                  # Architecture-specific libraries
```

### Test Build Configuration
```makefile
# Test builds (configured by autoconf/automake)
TEST_CFLAGS += -Iinclude               # Access to generic headers
TEST_CFLAGS += -Ilib/c                 # Access to klibc headers
TEST_CFLAGS += -Itests/unity/src       # Unity test framework
TEST_CFLAGS += -Itests                 # Test-specific headers
TEST_LDFLAGS += -Ltests/klibc          # Testing klibc implementation
```

## Architecture Separation Rules

### Generic Code (include/, lib/)
- **MUST** be architecture-neutral
- **MUST NOT** contain inline assembly
- **MUST NOT** depend on endianness
- **MUST NOT** use architecture-specific types or constants

### Architecture-Specific Code (arch/*/include/, arch/*/lib/, arch/*/drivers/)
- **MAY** contain inline assembly
- **MAY** depend on endianness
- **MAY** use architecture-specific types and constants
- **MUST** provide consistent interfaces to generic code

### Symlink Management
The autoconf/automake system manages architecture symlinks:
- `include/arch` → `../arch/$(target_arch)/include`
- `lib/arch` → `../arch/$(target_arch)/lib`

## Benefits of This Approach

1. **Clean Architecture**: Clear separation between generic and architecture-specific code
2. **Autoconf Integration**: Standard GNU build system with proper configuration management
3. **Portability**: Code works across different architectures without modification
4. **Flexibility**: Configure script controls feature selection and architecture targeting
5. **Maintainability**: Easier to add new architectures and features
6. **Cross-compilation**: Seamless support for different target architectures

## Standard Library Headers

Standard library headers should always use angle brackets:

```c
#include <stddef.h>      // Standard library
#include <stdint.h>      // Standard library
#include <stdbool.h>     // Standard library
```

Project headers should always use quotes:

```c
#include "klibc.h"       // Project header
#include "console.h"     // Project header
#include "arch/arch.h"   // Architecture-specific header via symlink
```

## Implementation Rules

1. **All `#include` directives MUST use paths resolvable via autoconf/automake `-I` flags**
2. **Autoconf/automake MUST provide all necessary `-I` and `-L` flags**
3. **Headers MUST be organized to avoid naming conflicts**
4. **Use quotes for project headers, angle brackets for system headers**
5. **Never use `../` or absolute paths in includes**
6. **Generic code MUST NOT depend on architecture-specific features**
7. **Architecture-specific code MUST be isolated in `arch/` directories**
8. **Symlinks MUST be managed by the build system, not manually**

## Forward Declaration Prohibition

**CRITICAL RULE: Forward declarations are ONLY allowed within the same module scope. Cross-module forward declarations are STRICTLY PROHIBITED.**

### Module Scope Definition

A **module** consists of a header file and its corresponding implementation file(s):
- `list.h` and `list.c` form one module
- `memory.h` and `memory.c` form one module  
- `process.h` and `process.c` form one module

### Allowed Forward Declarations (Within Module Scope)

#### Header Files - Internal Forward Declarations
```c
// In include/list.h - ALLOWED: Forward declarations for list.c functions
struct list_node_s;         // OK: Used internally in list.c
static int list_internal_helper(struct list_node_s *node);  // OK: Internal to list module

// Public interface - no forward declarations needed
typedef struct list_s {
    struct list_node_s *head;  // OK: Internal structure
    size_t count;
} list_t;
```

#### Source Files - Internal Forward Declarations  
```c
// In lib/list.c - ALLOWED: Forward declarations for internal functions
static struct list_node_s *find_node(list_t *list, void *data);  // OK: Internal function
static void merge_nodes(struct list_node_s *a, struct list_node_s *b);  // OK: Internal function

// Internal structure definition
struct list_node_s {
    void *data;
    struct list_node_s *next;
};
```

### Prohibited Practices (Cross-Module Forward Declarations)

#### FORBIDDEN: Cross-Module Forward Declarations in Headers
```c
// In include/memory.h - FORBIDDEN: Forward declarations from other modules
struct process_s;           // BAD: process_s is defined in process.h/process.c
struct capability_set;      // BAD: capability_set is defined in capability.h/capability.c

// Instead, include the proper header
#include "process.h"        // CORRECT: Include full definition
#include "capability.h"     // CORRECT: Include full definition
```

#### FORBIDDEN: Cross-Module Forward Declarations in Source Files
```c
// In lib/memory.c - FORBIDDEN: Forward declarations from other modules
int process_create(void);   // BAD: process_create is defined in process.h/process.c
struct thread_s;            // BAD: thread_s is defined in thread.h/thread.c

// Instead, include the proper header
#include "process.h"        // CORRECT: Include function declarations
#include "thread.h"         // CORRECT: Include struct definitions
```

### Rationale for Module-Scoped Forward Declarations

1. **Encapsulation**: Internal implementation details can use forward declarations within the module
2. **Interface Clarity**: Cross-module dependencies must be explicit through includes
3. **Maintainability**: Changes to public interfaces require updating the authoritative header
4. **Build System Enforcement**: The build system can detect cross-module forward declarations
5. **Circular Dependency Prevention**: Forces proper module design and dependency management

### Required Practices

#### Proper Cross-Module Dependencies
```c
// In include/memory.h - CORRECT approach
#include "process.h"        // Include cross-module dependencies
#include "capability.h"     // Include cross-module dependencies

// Now use the types directly
struct memory_space_s {
    process_t *owner;       // Direct use of process_t from process.h
    capability_set_t *caps; // Direct use from capability.h
    // ... rest of structure
};
```

#### Internal Module Organization
```c
// In include/list.h - Module public interface
typedef struct list_s list_t;  // Opaque type for users

// Public functions
int list_init(list_t *list);
int list_add(list_t *list, void *data);

// In lib/list.c - Module implementation
struct list_node_s;  // OK: Internal forward declaration

struct list_s {      // Full definition hidden from users
    struct list_node_s *head;
    size_t count;
};

struct list_node_s { // Internal structure
    void *data;
    struct list_node_s *next;
};

// Internal functions with forward declarations - OK
static struct list_node_s *find_node(list_t *list, void *data);
```

#### Circular Dependency Resolution
When circular dependencies exist between modules, resolve them through:

1. **Interface Extraction**: Move common interfaces to a separate header
2. **Dependency Inversion**: Use function pointers or callbacks
3. **Opaque Pointers**: Use `void*` with proper casting in implementation files

```c
// CORRECT: Resolve circular dependencies without cross-module forward declarations

// In include/common_types.h - Shared type definitions
typedef struct process_s process_t;
typedef struct memory_space_s memory_space_t;

// In include/process.h
#include "common_types.h"
#include "memory.h"         // Full include, no forward declaration

// In include/memory.h  
#include "common_types.h"
#include "process.h"        // Full include, no forward declaration
```

### Build System Enforcement

The build system MUST enforce this rule by:

1. **Scanning all source files** for forward declarations
2. **Comparing against existing headers** to detect duplicates
3. **Generating compiler errors** when forward declarations are found
4. **Providing clear error messages** indicating which header should be included instead

### Compliance Verification

All code MUST demonstrate:

1. **Zero Forward Declarations**: No `struct name;` or function declarations in any file
2. **Proper Header Inclusion**: All types and functions accessed through proper `#include` directives
3. **Single Definition Rule**: Each type and function declared in exactly one header
4. **Clean Dependencies**: Clear, acyclic dependency relationships between headers
5. **Build System Validation**: Automated detection and prevention of forward declarations

This approach ensures maintainable, consistent, and reliable code by eliminating the maintenance burden and potential inconsistencies introduced by forward declarations.

This approach ensures clean, maintainable, and portable code across all build configurations while providing proper architecture separation and autoconf/automake integration.