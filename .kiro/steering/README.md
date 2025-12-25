# FMI/OS Development Guidelines

This directory contains comprehensive development guidelines organized by functional area. These guidelines ensure consistency, quality, and maintainability across the FMI/OS codebase.

## Organization

### [Architecture](architecture/)
System architecture and code organization principles:
- **Naming Conventions** - Consistent naming patterns for types, functions, and structures
- **Kernel Subsystem Organization** - How kernel subsystems should be structured and organized
- **Private-First Interfaces** - Interface design principles for minimal public exposure

### [Build System](build-system/)
Autoconf/automake build system configuration and compilation guidelines:
- **Autoconf Build System** - Comprehensive build system architecture and requirements
- **Build Flags** - Compiler and linker flags for different build targets
- **Include Paths** - Header inclusion guidelines and architecture separation

### [Implementation](implementation/)
Kernel component implementation standards and patterns:
- **Standard Library Usage** - Mandatory use of compiler-provided functions and prohibition of custom implementations
- **Security Mitigations** - Comprehensive protection against speculative execution and other attacks
- **Synchronization Primitives** - Low-level synchronization approach and layering
- **Kernel Object Safety** - Thread-safe data structures and concurrent programming
- **Kernel Initialization** - Initialization patterns and memory reclamation

### [Development](development/)
Development process and communication standards:
- **AI Communication Style** - Communication principles for clear, precise technical discussion

## Core Guidelines

### Spec Development Guidelines
Comprehensive guidelines for creating and maintaining specification documents:
- **No Time Estimates** - Prohibition of estimated effort information in task lists
- **Dependency-Based Ordering** - Task ordering based on technical dependencies
- **Priority Classification** - Technical priority levels with clear justifications
- **Test-Driven Development** - Integration of testing requirements in all specs

## Key Principles

The FMI/OS development guidelines are built around several core principles:

### 1. Architectural Integrity
- **Clean Separation** - Clear boundaries between generic and architecture-specific code
- **Minimal Interfaces** - Private-first design with justified public interfaces
- **Consistent Organization** - Standardized patterns for subsystem organization

### 2. Code Quality
- **Standard Library Usage** - Mandatory use of compiler-provided functions instead of custom implementations
- **No Duplication** - Consistent use of core library functions throughout
- **Interface Compliance** - All code uses properly defined interfaces from headers
- **Naming Consistency** - Uniform naming conventions across all components

### 3. Build System Standards
- **Multi-Architecture Support** - Clean cross-compilation for x86_64, ARM64, RISC-V
- **Quality Enforcement** - Automated detection of architectural violations
- **Standard Compliance** - GNU autotools for maximum portability

### 4. Implementation Safety
- **Thread Safety** - All kernel data structures safe for concurrent access
- **Memory Safety** - Proper object lifecycle management and memory reclamation
- **Security Hardening** - Comprehensive protection against speculative execution attacks
- **Performance** - Lock-free operations and O(1) algorithms where appropriate

## Usage

These guidelines should be consulted during:
- **New Feature Development** - Ensure architectural compliance from the start
- **Spec Creation** - Follow spec development guidelines to avoid time estimates
- **Code Reviews** - Validate adherence to established patterns and standards
- **Refactoring** - Maintain consistency when modifying existing code
- **Documentation** - Follow communication standards for clear technical writing

## Compliance

All FMI/OS code must demonstrate compliance with these guidelines:
- **Architecture Guidelines** - Proper separation and organization
- **Build System Guidelines** - Correct configuration and compilation
- **Implementation Guidelines** - Safe, efficient, and consistent implementation
- **Development Guidelines** - Clear communication and documentation
- **Spec Guidelines** - No time estimates, dependency-based task ordering

These guidelines ensure that FMI/OS maintains its architectural integrity, code quality, and development velocity as the project grows and evolves.
