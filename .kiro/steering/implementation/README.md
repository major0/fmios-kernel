# Implementation Guidelines

This directory contains guidelines for implementing specific kernel components and subsystems.

## Documents

### [standard-library-usage.md](standard-library-usage.md)
Establishes mandatory use of compiler-provided standard library functions:
- Required use of standard string and memory functions
- Explicit prohibition of custom string/memory implementations
- Safe usage patterns and bounds checking requirements
- Build system enforcement of standard library compliance

### [security-mitigations.md](security-mitigations.md)
Comprehensive security requirements and mitigations:
- Speculative execution attack protections
- Memory safety and bounds checking requirements
- Control flow integrity and KASLR support
- Build-time and runtime security validation

### [synchronization-primitives.md](synchronization-primitives.md)
Describes the approach for implementing synchronization primitives:
- Simple, atomic-based locks at the lowest level
- Clean layering with thread management infrastructure
- Mandatory use of core library functions
- Performance considerations and usage guidelines

### [kobj-safety.md](kobj-safety.md)
Establishes requirements for kernel object safety and thread-safe data structures:
- Unified kernel object header with integrated locking
- Thread-safe data structures (lists, hash tables, B-trees)
- Lock hierarchy and ordering to prevent deadlocks
- Code duplication prevention through consistent library usage

### [kernel-initialization.md](kernel-initialization.md)
Design principles for kernel initialization and memory management:
- Centralized initialization in `kernel/main.c`
- Linker script integration for memory reclamation
- Memory efficiency through initialization code freeing
- Clean separation between initialization and runtime code

## Core Principles

1. **Standard Library Usage** - Mandatory use of compiler-provided functions for string and memory operations
2. **No Custom Implementations** - Prohibition of custom string/memory function implementations
3. **Layered Architecture** - Clear separation between low-level primitives and high-level services
4. **Thread Safety** - All kernel data structures must be safe for concurrent access
5. **Memory Efficiency** - Optimal memory usage through object pools and reclamation
6. **Code Reuse** - Consistent use of core library functions to prevent duplication
7. **Security First** - Comprehensive protection against attacks and vulnerabilities

These guidelines ensure that FMI/OS kernel components are implemented consistently, safely, and efficiently while maintaining the clean architectural principles established for the system.