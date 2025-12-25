# Design Document: Testing Framework

## Overview

The FMI/OS Testing Framework provides a comprehensive testing infrastructure for a new operating system kernel being developed from scratch. The framework integrates Test-Driven Development (TDD) methodology with modern testing tools and practices from the beginning of the project. The framework is built around Ceedling as the primary testing tool, enhanced with property-based testing capabilities, QEMU integration for kernel testing, and multi-architecture support.

The framework addresses the unique challenges of kernel development by providing both freestanding and hosted testing environments through a dual klibc strategy, enabling developers to test kernel components in isolation while also validating them in actual kernel environments using QEMU virtualization.

Key design principles include:
- **Test-First Development**: All code will be developed using TDD methodology with tests written before implementation
- **Comprehensive Coverage**: Support for unit tests, integration tests, property-based tests, and QEMU smoke tests
- **Multi-Architecture Support**: Testing across x86_64, ARM64, and RISC-V architectures from the start
- **Automation**: Automatic test discovery, runner generation, and mock creation
- **Integration**: Seamless integration with the autoconf/automake build system being developed

## Architecture

The testing framework follows a layered architecture that separates concerns and provides flexibility:

```
┌─────────────────────────────────────────────────────────────┐
│                    Developer Interface                      │
├─────────────────────────────────────────────────────────────┤
│  TDD Workflow  │  Test Writing  │  Debugging  │  Reporting  │
├─────────────────────────────────────────────────────────────┤
│                    Testing Framework Core                   │
├─────────────────────────────────────────────────────────────┤
│   Ceedling    │  Property-Based │    QEMU     │   Coverage  │
│   Framework   │     Testing      │  Integration │  Analysis  │
├─────────────────────────────────────────────────────────────┤
│                    Build System Integration                 │
├─────────────────────────────────────────────────────────────┤
│  Autoconf/Automake  │  Cross-Compilation  │  CI/CD Support │
├─────────────────────────────────────────────────────────────┤
│                    Testing Infrastructure                   │
├─────────────────────────────────────────────────────────────┤
│  Dual Klibc  │  Mock Generation │  Test Discovery │ Runners │
└─────────────────────────────────────────────────────────────┘
```

### Core Components

1. **Ceedling Framework**: Ruby-based build system that wraps Unity and CMock
2. **Property-Based Testing Engine**: Generates random inputs to validate universal properties
3. **QEMU Integration Layer**: Executes kernel tests in virtual machine environments
4. **Multi-Architecture Support**: Cross-compilation and testing across target architectures
5. **Dual Klibc Strategy**: Separate kernel and testing C library implementations
6. **Build System Integration**: Seamless integration with autoconf/automake

## Components and Interfaces

### Ceedling Integration Component

**Purpose**: Provides the primary testing framework with automatic test runner generation and mock capabilities.

**Key Features**:
- Automatic test discovery and runner generation
- CMock integration for automatic mock generation
- Ruby-based configuration and extensibility
- Integration with Unity test framework
- Comprehensive reporting and coverage analysis

**Configuration**: Managed through `project.yml` configuration file with FMI/OS-specific settings:
```yaml
:project:
  :build_root: build/test
  :test_file_prefix: test_
  :source_file_prefix: ""

:paths:
  :test:
    - tests/**
  :source:
    - kernel/**
    - lib/**
    - arch/*/lib/**
  :include:
    - include/**
    - arch/*/include/**

:defines:
  :test:
    - UNIT_TEST
    - TEST_ENVIRONMENT
  :test_preprocess:
    - UNIT_TEST
    - TEST_ENVIRONMENT

:cmock:
  :mock_prefix: Mock
  :when_no_prototypes: :warn
  :enforce_strict_ordering: TRUE
  :plugins:
    - :ignore
    - :callback
    - :return_thru_ptr

:gcov:
  :reports:
    - HtmlDetailed
    - Text
  :gcovr:
    :html_medium_threshold: 75
    :html_high_threshold: 90
```

### Property-Based Testing Component

**Purpose**: Implements property-based testing capabilities for validating universal correctness properties.

**Key Features**:
- Random input generation for comprehensive testing
- Property definition and validation framework
- Shrinking/minimization of failing test cases
- Integration with existing test infrastructure
- Support for custom generators and constraints

**Implementation Strategy**:
- Extend Ceedling with custom Ruby tasks for property-based testing
- Implement C-based property testing library with random generation
- Provide macros and utilities for defining properties and generators
- Integration with existing Unity assertions and reporting

**Property Definition Pattern**:
```c
// Property definition macro
#define PROPERTY_TEST(name, iterations) \
    void test_property_##name(void) { \
        for (int i = 0; i < iterations; i++) { \
            /* Property test implementation */ \
        } \
    }

// Example property test
PROPERTY_TEST(list_add_preserves_count, 100) {
    list_t list;
    int initial_count = random_int_range(0, 100);
    int item = random_int();

    // Setup list with initial_count items
    setup_list_with_count(&list, initial_count);

    // Add item and verify count increases by 1
    list_add(&list, &item);
    TEST_ASSERT_EQUAL(initial_count + 1, list_count(&list));
}
```

### QEMU Integration Component

**Purpose**: Enables testing of kernel components in actual kernel environments using QEMU virtualization.

**Key Features**:
- Multi-architecture QEMU support (x86_64, ARM64, RISC-V)
- Serial console output capture and validation
- Automated QEMU invocation and management
- Timeout handling and error detection
- Integration with Ceedling test reporting

**QEMU Test Execution Flow**:
1. Build minimal test kernel with test code
2. Launch QEMU with appropriate architecture and parameters
3. Capture serial console output
4. Validate expected output patterns
5. Report test results through Ceedling

**Custom Ceedling Tasks** (in `rakefile.rb`):
```ruby
namespace :qemu do
  desc "Run QEMU tests for all architectures"
  task :test_all do
    architectures = ['x86_64', 'arm64', 'riscv64']
    architectures.each do |arch|
      Rake::Task["qemu:test_#{arch}"].invoke
    end
  end

  desc "Run QEMU tests for x86_64"
  task :test_x86_64 do
    run_qemu_test('x86_64', 'qemu-system-x86_64')
  end

  desc "Run QEMU tests for ARM64"
  task :test_arm64 do
    run_qemu_test('arm64', 'qemu-system-aarch64')
  end

  desc "Run QEMU tests for RISC-V"
  task :test_riscv64 do
    run_qemu_test('riscv64', 'qemu-system-riscv64')
  end
end

def run_qemu_test(arch, qemu_binary)
  # QEMU test execution implementation
  # Follows QEMU invocation standard with timeout
  cmd = "timeout 30s #{qemu_binary} -kernel kernel/fmi-kernel -serial mon:stdio -nographic"
  # Execute and validate output
end
```

### Multi-Architecture Testing Component

**Purpose**: Ensures code correctness across all supported processor architectures.

**Key Features**:
- Cross-compilation support for x86_64, ARM64, RISC-V
- Architecture-specific compiler flag handling
- Separate test execution for each architecture
- Architecture-neutral code validation
- Per-architecture result reporting

**Architecture Configuration**:
```yaml
:architectures:
  :x86_64:
    :compiler: x86_64-fmios-gcc
    :flags:
      - -mcmodel=kernel
      - -mno-red-zone
      - -mno-mmx
      - -mno-sse
      - -mno-sse2
  :arm64:
    :compiler: aarch64-fmios-gcc
    :flags:
      - -mcpu=cortex-a57
      - -mgeneral-regs-only
  :riscv64:
    :compiler: riscv64-fmios-gcc
    :flags:
      - -march=rv64imac
      - -mabi=lp64
```

### Dual Klibc Strategy Component

**Purpose**: Provides separate kernel and testing C library implementations for different testing environments.

**Key Features**:
- Kernel klibc: Freestanding implementation for actual kernel execution
- Testing klibc: Hosted implementation wrapping standard libc for unit testing
- Interface parity validation between implementations
- Automatic selection based on test type
- Build system integration for library selection

**Library Selection Logic**:
- Kernel/QEMU tests: Link against `lib/c/libklibc.a` (freestanding)
- Unit tests: Link against `tests/klibc/libklibc-test.a` (hosted)
- Integration tests: Use kernel klibc with hosted test harness
- Property tests: Use testing klibc for random generation capabilities

## Data Models

### Test Configuration Model

```c
typedef struct test_config_s {
    char project_name[64];
    char build_root[256];
    char test_file_prefix[16];
    char source_file_prefix[16];

    struct {
        char test_paths[MAX_PATHS][256];
        char source_paths[MAX_PATHS][256];
        char include_paths[MAX_PATHS][256];
        int path_count[3];  // test, source, include
    } paths;

    struct {
        char test_defines[MAX_DEFINES][64];
        char source_defines[MAX_DEFINES][64];
        int define_count[2];  // test, source
    } defines;

    struct {
        bool enabled;
        char mock_prefix[16];
        bool strict_ordering;
        char plugins[MAX_PLUGINS][32];
        int plugin_count;
    } cmock;

    struct {
        bool enabled;
        char report_types[MAX_REPORTS][32];
        int report_count;
        int html_medium_threshold;
        int html_high_threshold;
    } coverage;
} test_config_t;
```

### Test Result Model

```c
typedef struct test_result_s {
    char test_name[128];
    char test_file[256];
    enum test_status {
        TEST_PASSED,
        TEST_FAILED,
        TEST_IGNORED,
        TEST_ERROR
    } status;

    struct {
        int total;
        int passed;
        int failed;
        int ignored;
    } assertions;

    struct {
        double execution_time;
        size_t memory_used;
        int iterations;  // For property-based tests
    } metrics;

    struct {
        char message[512];
        char file[256];
        int line;
        char expected[256];
        char actual[256];
    } failure_info;

    struct {
        double line_coverage;
        double branch_coverage;
        double function_coverage;
    } coverage;
} test_result_t;
```

### QEMU Test Configuration Model

```c
typedef struct qemu_test_config_s {
    char architecture[16];      // x86_64, arm64, riscv64
    char qemu_binary[64];       // qemu-system-x86_64, etc.
    char kernel_image[256];     // Path to test kernel

    struct {
        int timeout_seconds;
        int memory_mb;
        int cpu_cores;
        char additional_args[512];
    } parameters;

    struct {
        char expected_patterns[MAX_PATTERNS][128];
        int pattern_count;
        char error_patterns[MAX_PATTERNS][128];
        int error_pattern_count;
    } validation;

    struct {
        char output_file[256];
        bool capture_serial;
        bool save_logs;
    } output;
} qemu_test_config_t;
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

Based on the prework analysis, the following correctness properties have been identified for the testing framework:

### Property 1: Test Execution Completeness
*For any* set of discovered test files, when tests are executed, all tests should be run and results should be reported for each test.
**Validates: Requirements 1.3, 7.4, 7.5**

### Property 2: Automatic Test Discovery
*For any* test file following the naming convention, the testing framework should automatically discover and include it in test execution.
**Validates: Requirements 7.1, 7.2, 7.3**

### Property 3: Ceedling Integration Consistency
*For any* test configuration, when Ceedling is used as the primary testing tool, all test operations should be performed through Ceedling interfaces.
**Validates: Requirements 2.1, 2.2, 2.3**

### Property 4: Mock Generation Completeness
*For any* defined kernel interface, when mocks are requested, the framework should generate appropriate mock implementations with expected behavior.
**Validates: Requirements 2.4, 8.1, 8.2, 8.3, 8.4, 8.5**

### Property 5: Property-Based Test Execution
*For any* property-based test, the framework should execute the specified number of iterations with different random inputs and report results.
**Validates: Requirements 3.1, 3.2, 3.4**

### Property 6: Property Test Failure Minimization
*For any* failing property-based test, the framework should provide a minimal failing example that demonstrates the property violation.
**Validates: Requirements 3.3**

### Property 7: QEMU Multi-Architecture Support
*For any* supported architecture (x86_64, ARM64, RISC-V), QEMU tests should execute successfully and capture console output for validation.
**Validates: Requirements 4.1, 4.2, 4.3, 4.4, 4.5**

### Property 8: Multi-Architecture Test Consistency
*For any* architecture-neutral test, the test should produce equivalent results when executed on different target architectures.
**Validates: Requirements 5.1, 5.4, 5.5**

### Property 9: Architecture-Specific Compiler Flags
*For any* target architecture, when cross-compilation is used, the framework should apply the correct architecture-specific compiler flags.
**Validates: Requirements 5.2, 5.3**

### Property 10: Dual Klibc Library Selection
*For any* test type, the framework should automatically select the appropriate klibc implementation (kernel or testing) based on the test environment.
**Validates: Requirements 6.1, 6.2, 6.4**

### Property 11: Klibc Interface Parity
*For any* function in the klibc interface, both kernel and testing implementations should provide equivalent functionality and behavior.
**Validates: Requirements 6.3, 6.5**

### Property 12: Coverage Data Collection
*For any* test execution, the framework should collect and report line coverage, branch coverage, and function coverage information.
**Validates: Requirements 9.1, 9.2, 9.3**

### Property 13: Coverage Threshold Enforcement
*For any* configured coverage threshold, the framework should enforce the threshold and report violations when coverage falls below the requirement.
**Validates: Requirements 9.4, 9.5**

### Property 14: Build System Integration
*For any* build system invocation, the testing framework should integrate seamlessly with autoconf/automake and provide appropriate build targets.
**Validates: Requirements 10.2, 10.3, 10.4, 10.5**

### Property 15: Test Organization and Structure
*For any* test suite organization, the framework should support hierarchical organization, consistent naming, and proper setup/teardown mechanisms.
**Validates: Requirements 11.1, 11.2, 11.3, 11.4, 11.5**

### Property 16: Performance Test Measurement
*For any* performance test, the framework should collect timing and resource usage metrics and report performance data.
**Validates: Requirements 12.1, 12.3**

### Property 17: Performance Regression Detection
*For any* performance test with historical data, the framework should detect and report performance regressions when they occur.
**Validates: Requirements 12.4**

### Property 18: Error Reporting Completeness
*For any* test failure, the framework should provide detailed failure information including stack traces, context, and actionable error messages.
**Validates: Requirements 13.1, 13.3, 13.5**

### Property 19: Test Isolation Support
*For any* individual test, the framework should support running the test in isolation with proper debugging capabilities.
**Validates: Requirements 13.2, 13.4**

### Property 20: CI/CD Integration Compatibility
*For any* CI/CD environment, the framework should provide machine-readable output in standard formats and support parallel execution.
**Validates: Requirements 14.1, 14.2, 14.3, 14.4, 14.5**

### Property 21: Comprehensive Reporting
*For any* test execution, the framework should generate comprehensive reports in multiple formats with trend analysis and visualization capabilities.
**Validates: Requirements 15.1, 15.3, 15.4, 15.5**

## Error Handling

The testing framework implements comprehensive error handling across all components:

### Test Execution Errors
- **Test Discovery Failures**: When test files cannot be discovered or parsed, provide clear error messages indicating the issue and suggested fixes
- **Compilation Errors**: When test code fails to compile, report compiler errors with file and line information
- **Runtime Errors**: When tests crash or encounter runtime errors, capture stack traces and provide debugging information
- **Timeout Handling**: When tests exceed configured timeouts, terminate gracefully and report timeout conditions

### QEMU Integration Errors
- **QEMU Availability**: Detect when QEMU binaries are not available for target architectures and provide installation guidance
- **Kernel Image Errors**: When kernel images are missing or invalid, provide clear error messages and build instructions
- **Console Output Errors**: When expected output patterns are not found, report the actual output and expected patterns
- **Architecture Mismatch**: When QEMU architecture doesn't match kernel architecture, report the mismatch and suggest corrections

### Build System Integration Errors
- **Configuration Errors**: When autoconf/automake configuration is invalid, provide specific error messages and configuration guidance
- **Dependency Errors**: When required tools or libraries are missing, report missing dependencies and installation instructions
- **Cross-Compilation Errors**: When cross-compilation fails, report toolchain issues and configuration problems
- **Library Selection Errors**: When klibc selection fails, report library path issues and build configuration problems

### Coverage and Reporting Errors
- **Coverage Collection Errors**: When coverage data cannot be collected, report instrumentation issues and suggest fixes
- **Report Generation Errors**: When reports cannot be generated, report file system issues and permission problems
- **Threshold Violations**: When coverage thresholds are not met, provide detailed information about uncovered code
- **Format Errors**: When output formats are invalid, report format issues and provide valid format options

## Testing Strategy

The testing framework itself follows a comprehensive testing strategy that validates all components and integration points:

### Unit Testing Approach
- **Component Isolation**: Each framework component is tested in isolation using mocks for dependencies
- **Interface Validation**: All public interfaces are tested with valid and invalid inputs
- **Error Condition Testing**: All error paths and edge cases are tested with appropriate error injection
- **Configuration Testing**: All configuration options and combinations are tested for correctness

### Property-Based Testing for Framework
- **Configuration Property**: For any valid configuration, the framework should initialize correctly and provide expected functionality
- **Test Discovery Property**: For any test file structure, the framework should discover all valid tests and ignore invalid ones
- **Cross-Architecture Property**: For any test, results should be consistent across architectures (for architecture-neutral code)
- **Coverage Property**: For any test execution, coverage data should be accurate and complete

### Integration Testing Strategy
- **Build System Integration**: Test integration with autoconf/automake across different configurations and environments
- **QEMU Integration**: Test QEMU execution across all supported architectures with various kernel configurations
- **CI/CD Integration**: Test framework behavior in various CI/CD environments and configurations
- **Tool Chain Integration**: Test integration with different compiler toolchains and cross-compilation setups

### QEMU Smoke Testing
- **Framework Bootstrap**: Test that the testing framework itself can be validated using QEMU
- **Multi-Architecture Validation**: Ensure framework works correctly across all supported architectures
- **Console Output Validation**: Verify that test output is correctly captured and validated
- **Error Condition Testing**: Test framework behavior when QEMU tests fail or timeout

### Performance and Load Testing
- **Test Execution Performance**: Measure and validate test execution times across different test types
- **Parallel Execution**: Test framework behavior under parallel test execution loads
- **Memory Usage**: Monitor and validate memory usage during test execution
- **Scalability**: Test framework behavior with large numbers of tests and complex test suites

### Continuous Integration Testing
- **Multi-Environment Testing**: Test framework across different operating systems and environments
- **Version Compatibility**: Test compatibility with different versions of tools and dependencies
- **Regression Testing**: Maintain comprehensive regression test suite for framework changes
- **Documentation Testing**: Validate that documentation examples and instructions work correctly

This comprehensive testing strategy ensures that the testing framework itself is reliable, robust, and provides accurate results for FMI/OS kernel development.
