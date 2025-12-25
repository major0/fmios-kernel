# FMI/OS - Microkernel Operating System

## Overview

FMI/OS is a modern microkernel operating system designed with security, performance, and maintainability as core principles. The system implements a capabilities-based security model, spec-driven development methodology, and comprehensive testing framework to ensure reliable and secure operation.

### Key Features

- **Microkernel Architecture**: Minimal kernel with services running in userspace
- **Capabilities-Based Security**: Fine-grained access control without traditional root privilege escalation
- **Multi-Architecture Support**: x86_64, ARM64, and RISC-V architectures
- **Spec-Driven Development**: Requirements-first development with property-based testing
- **Git Workflow Integration**: Topic branches with conventional commits and dual repository management

## Building

FMI/OS uses an autoconf/automake-based build system with support for cross-compilation across multiple architectures.

### Prerequisites

- GCC or Clang compiler with cross-compilation support
- GNU autotools (autoconf, automake, libtool)
- QEMU for testing (qemu-system-x86_64, qemu-system-aarch64, qemu-system-riscv64)

### Build Instructions

1. **Configure the build system:**
  ```bash
  # For x86_64 target
  ./configure --target=x86_64-fmios

  # For ARM64 target
  ./configure --target=aarch64-fmios

  # For RISC-V target
  ./configure --target=riscv64-fmios
  ```

2. **Build the kernel:**
  ```bash
  make
  ```

3. **Run tests:**
  ```bash
  make check
  ```

4. **Test in QEMU:**
  ```bash
  # x86_64 testing
  make qemu-test-x86_64

  # ARM64 testing
  make qemu-test-aarch64

  # RISC-V testing
  make qemu-test-riscv64
  ```

### Build Configuration Options

- `--enable-debug` - Enable debug builds with additional validation
- `--enable-serial-console` - Enable serial console support (default: yes)
- `--enable-video-console` - Enable VGA video console support (default: yes)
- `--enable-qemu-testing` - Enable QEMU smoke tests (default: yes)

## Testing

FMI/OS follows a comprehensive Test-Driven Development (TDD) approach:

- **Unit Tests**: Test individual functions and components in isolation
- **Property-Based Tests**: Test universal properties across randomized inputs
- **Integration Tests**: Test interactions between components and subsystems
- **QEMU Smoke Tests**: Test kernel components in actual kernel environment

Run the complete test suite:
```bash
make check
```

## Contributing

FMI/OS uses a spec-driven development process with Git workflow integration.

### Development Workflow

1. **Setup Git Hooks**: Install pre-commit hooks for automated validation
  ```bash
  ./scripts/setup-git-hooks.sh
  ```

2. **Spec-Driven Development**: All features start with requirements and design specifications
3. **Topic Branches**: Each task gets its own branch following `{spec-name}/{task-number}-{brief-description}` format
4. **Conventional Commits**: All commits follow conventional commit format with task references
5. **Dual Repository Management**: Changes are tracked in both implementation and specification repositories

### Git Workflow

1. **Create a topic branch:**
  ```bash
  ./scripts/create-topic-branch.sh <spec-name> <task-number> <description>
  ```

2. **Make changes with conventional commits:**
  ```bash
  git commit -m "feat(memory): implement slab allocator core functionality

  Task: memory#2.1"
  ```

3. **Prepare for merge:**
  ```bash
  ./scripts/prepare-merge.sh
  ```

4. **Create pull request** with proper task references and acceptance criteria

### Commit Message Format

All commits must follow conventional commit specification:

```
{type}({scope}): {description}

{optional body}

Task: {spec-name}#{task-number}
```

**Commit Types:**
- `feat`: New feature implementation
- `fix`: Bug fixes
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring without feature changes
- `test`: Adding or modifying tests
- `chore`: Maintenance tasks
- `build`: Build system changes
- `ci`: CI/CD configuration changes

### Code Standards

- **Security First**: All code includes comprehensive security mitigations
- **Standard Library Usage**: Use compiler-provided functions, no custom string/memory implementations
- **Thread Safety**: All kernel data structures must be safe for concurrent access
- **Property-Based Testing**: Universal properties validated across all inputs
- **Architecture Separation**: Clean separation between generic and architecture-specific code

### Quality Gates

All contributions must pass:
- [ ] Unit tests and property-based tests
- [ ] Code style and formatting checks
- [ ] Security vulnerability scans
- [ ] QEMU smoke tests across all architectures
- [ ] GPL v2 license header validation
- [ ] Conventional commit format validation

## Architecture

FMI/OS follows a clean microkernel architecture:

- **Kernel**: Minimal kernel providing basic services (memory, scheduling, IPC)
- **Drivers**: Hardware-specific drivers in userspace
- **Services**: System services running as userspace processes
- **Applications**: User applications with restricted capabilities

### Directory Structure

```
fmios/
├── include/           # Generic headers (architecture-neutral)
├── lib/               # Generic libraries and kernel C library
├── arch/              # Architecture-specific code
│   ├── x86_64/        # x86_64-specific implementation
│   ├── arm64/         # ARM64-specific implementation
│   └── riscv64/       # RISC-V-specific implementation
├── kernel/            # Generic kernel code
├── drivers/           # Generic driver interfaces
├── tests/             # Test suite
└── docs/              # Documentation
```

## Documentation

- **API Documentation**: Generated from source code comments
- **Architecture Guides**: Located in `docs/` directory
- **Development Guidelines**: Located in `.kiro/steering/` directory

## License

This project is licensed under the GNU General Public License v2.0 - see [LICENSE.txt](LICENSE.txt) for details.

## Support

For questions, issues, or contributions:
- Follow the Git workflow for all contributions
- Ensure all quality gates pass before submitting pull requests
- Include proper task references in all commits and pull requests
