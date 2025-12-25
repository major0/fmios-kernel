---
inclusion: always
---

# Test-Driven Development Methodology

FMI/OS follows a comprehensive Test-Driven Development (TDD) approach where tests are written first to define expected behavior, followed immediately by implementation to satisfy those tests. This methodology ensures:

- **Specification Clarity**: Tests serve as executable specifications that define exactly what each component should do
- **Quality Assurance**: All code is written to pass well-defined tests, ensuring correctness from the start
- **Regression Prevention**: Comprehensive test coverage prevents future changes from breaking existing functionality
- **Documentation**: Tests provide living documentation of system behavior and requirements

## TDD Process

1. **Write Tests First**: For every component, feature, or subsystem, comprehensive tests are written before any implementation code
2. **Implement to Pass**: Implementation code is written specifically to make the tests pass
3. **Validate Completeness**: All tests must pass before a component is considered complete
4. **Maintain Test Coverage**: New features require new tests; changes to existing features require test updates

## Test Categories

- **Unit Tests**: Test individual functions and components in isolation
- **Integration Tests**: Test interactions between components and subsystems
- **Property-Based Tests**: Test universal properties across all possible inputs using randomized testing
- **QEMU Smoke Tests**: Test kernel components in actual kernel environment using virtual machines
- **Architecture Tests**: Test multi-architecture compatibility and cross-compilation

## TDD Requirements for All Specs

### Universal TDD Approach
All subsystems follow strict Test-Driven Development where tests for each component are written first, defining expected behavior, followed by implementation to satisfy those tests.

### TDD Test Requirements Template
1. ALL [subsystem] functions SHALL have unit tests written before implementation
2. ALL [subsystem] operations SHALL have property-based tests validating correctness across random inputs
3. ALL [subsystem] integration points SHALL have tests validating proper interaction with other subsystems
4. ALL error conditions SHALL have tests validating proper handling and recovery
5. ALL concurrency operations SHALL have tests validating thread safety and race condition prevention

## Testing Infrastructure Requirements

### Dual Klibc Strategy
- **Kernel klibc**: Freestanding implementation for actual kernel execution
- **Testing klibc**: Hosted implementation that wraps standard libc for unit testing
- **Interface Parity**: Both implementations must provide identical interfaces
- **Build System Selection**: Linker flags select appropriate implementation automatically

### QEMU Integration Testing
- **Real Environment Testing**: Validate kernel components in actual kernel environment
- **Multi-Architecture Support**: Test across x86_64, ARM64, RISC-V architectures
- **Automated Execution**: Integration with build system for continuous testing
- **Console Capture**: Automated validation of kernel output and behavior

This TDD methodology applies to all FMI/OS subsystems and ensures consistent quality and reliability across the entire system.
