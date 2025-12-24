# Architecture Guidelines

This directory contains guidelines for the overall system architecture and code organization.

## Documents

### [naming-conventions.md](naming-conventions.md)
Establishes naming conventions for all code in the FMI/OS project, including:
- Type naming with `_t` suffix
- Struct naming with `_s` suffix for typedef'd structs
- Function naming patterns
- Standard library conflict resolution with `k` prefix

### [kernel-subsystem-organization.md](kernel-subsystem-organization.md)
Defines the organization principles for kernel subsystems:
- One subsystem per file in `kernel/` directory
- Global resource management patterns
- Filesystem interface requirements
- Clear separation between kernel and library code

### [private-first-interfaces.md](private-first-interfaces.md)
Establishes the principle of private-first interface design:
- Static-by-default functions
- Minimal public headers
- Opaque type declarations
- Justification requirements for public interfaces

## Key Principles

1. **Consistency** - All code follows the same naming and organization patterns
2. **Encapsulation** - Minimal public interfaces with maximum information hiding
3. **Maintainability** - Clear structure that supports long-term development
4. **Separation of Concerns** - Clean boundaries between different system components

These guidelines ensure that FMI/OS maintains a clean, maintainable, and secure architecture while preventing common architectural problems that arise from over-exposed subsystem internals.