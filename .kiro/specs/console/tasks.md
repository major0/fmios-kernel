# Implementation Plan: Console Subsystem

## Overview

This implementation plan establishes the console subsystem for FMI/OS following the incremental development approach. The console system starts with minimal direct serial output for Stage 1 and grows to support comprehensive multi-device console management by Stage 4.

**INCREMENTAL APPROACH**:
- Stage 1: Direct serial output only for Hello World kernel
- Stage 4: Multi-device console system with driver management
- Stage 5: ARM64 console drivers
- Stage 6: RISC-V console drivers

## Tasks

### Stage 1: Direct Serial Output

- [ ] 1. Implement minimal direct serial console output
**Priority: CRITICAL BLOCKER - Required for Stage 1 Hello World kernel**
**Dependencies: None**

Create minimal serial console output for immediate debug capabilities in Stage 1.

- Implement direct x86_64 serial port output (port 0x3F8)
- Create basic kprintf() function with simple format string support
- Support basic format specifiers (%s, %d, %x, %c)
- Ensure output works immediately after bootstrap without initialization
- No console driver registration or management needed
- No buffering or complex formatting required
- Focus on reliability and immediate availability
- Support emergency output for critical failures
- Test with QEMU serial console output
- _Requirements: 6.1, 6.2, 6.4, 6.9_

- [ ] 1.1 Write property test for direct serial output reliability
  - **Property 1: Direct Serial Output Reliability**
  - **Validates: Requirements 6.1, 6.4, 6.9**

### Stage 4: Multi-Device Console System

- [ ] 2. Implement comprehensive console subsystem
**Priority: HIGH FUNCTIONALITY - Required for Stage 4 feature expansion**
**Dependencies: Task 1 (direct serial output), Stage 4 kernel features**

Implement full console subsystem with multi-device support for Stage 4.

- Design and implement console driver interface structure
- Create console multiplexer for managing multiple output devices
- Implement console driver registration and management system
- Add console device priority and selection mechanisms
- Implement thread-safe console operations with proper locking
- Support console selection via boot parameters (console=ttyS0,115200)
- Add console driver lifecycle management
- Implement console fallback when primary console fails
- Add console statistics and status reporting
- Integrate with boot command line parameter system
- _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 2.10, 5.1, 5.2, 5.3, 5.4, 5.6_

- [ ] 2.1 Write property test for console driver registration consistency
  - **Property 2: Console Driver Registration Consistency**
  - **Validates: Requirements 5.1, 5.2**

- [ ] 2.2 Write property test for console device failover reliability
  - **Property 3: Console Device Failover Reliability**
  - **Validates: Requirements 1.8, 2.5**

- [ ] 3. Implement kernel logging system
**Priority: HIGH FUNCTIONALITY - Essential for Stage 4 system monitoring**
**Dependencies: Task 2 (console subsystem)**

Implement comprehensive kernel logging with levels and timestamps for Stage 4.

- Implement klogf() function wrapping kprintf()
- Add support for log levels (DEBUG, INFO, WARN, ERROR, CRITICAL)
- Produce syslog-style formatted output with timestamps
- Use jiffies since boot as timestamps
- Format messages with timestamp, level, and content
- Implement log level filtering mechanisms
- Handle variable argument formatting safely
- Ensure thread-safe logging operations
- Provide emergency logging capabilities
- Add time-based timestamps for testing framework
- _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 4.8, 4.9, 4.10_

- [ ] 3.1 Write property test for log level filtering correctness
  - **Property 4: Log Level Filtering Correctness**
  - **Validates: Requirements 4.6, 4.2**

- [ ] 3.2 Write property test for message format consistency
  - **Property 5: Message Format Consistency**
  - **Validates: Requirements 4.3, 4.4, 4.5**

- [ ] 4. Implement x86_64 console drivers
**Priority: HIGH FUNCTIONALITY - Platform-specific hardware support for Stage 4**
**Dependencies: Task 3 (kernel logging system)**

Implement x86_64-specific console drivers for Stage 4 enhanced console system.

- Enhance x86_64 serial console driver for standard PC serial ports
- Implement x86_64 video console driver for VGA text mode output
- Provide consistent interface with generic console management system
- Handle x86_64-specific initialization requirements
- Support x86_64-specific features and optimizations
- Integrate with generic console management system
- Implement proper hardware detection and configuration
- Add x86_64-specific error handling and recovery
- _Requirements: 3.1, 3.2, 3.7, 3.8_

- [ ] 4.1 Write property test for x86_64 driver compatibility
  - **Property 6: x86_64 Driver Compatibility**
  - **Validates: Requirements 3.7, 3.8**

### Stage 5: ARM64 Console Support

- [ ] 5. Implement ARM64 console drivers
**Priority: HIGH FUNCTIONALITY - Required for Stage 5 ARM64 port**
**Dependencies: Task 4 (x86_64 drivers), Stage 5 ARM64 architecture support**

Implement ARM64-specific console drivers for Stage 5.

- Implement ARM64 console drivers for platform-specific UART devices
- Provide consistent interface across ARM64 and x86_64 architectures
- Handle ARM64-specific initialization requirements
- Support ARM64-specific features and optimizations
- Integrate with generic console management system
- Implement proper hardware detection and configuration
- Add ARM64-specific error handling and recovery
- Ensure feature parity with x86_64 console capabilities
- _Requirements: 3.3, 3.4, 3.7, 3.8_

- [ ] 5.1 Write property test for ARM64 driver compatibility
  - **Property 7: ARM64 Driver Compatibility**
  - **Validates: Requirements 3.7, 3.8**

### Stage 6: RISC-V Console Support

- [ ] 6. Implement RISC-V console drivers
**Priority: HIGH FUNCTIONALITY - Required for Stage 6 RISC-V port**
**Dependencies: Task 5 (ARM64 drivers), Stage 6 RISC-V architecture support**

Complete multi-architecture console support with RISC-V drivers for Stage 6.

- Implement RISC-V console drivers for SBI console interface
- Provide consistent interface across all three architectures
- Handle RISC-V-specific initialization requirements
- Support RISC-V-specific features and optimizations
- Integrate with generic console management system
- Implement proper hardware detection and configuration
- Add RISC-V-specific error handling and recovery
- Ensure feature parity across all architectures (x86_64, ARM64, RISC-V)
- _Requirements: 3.5, 3.6, 3.7, 3.8, 3.9, 3.10_

- [ ] 6.1 Write property test for RISC-V driver compatibility
  - **Property 8: RISC-V Driver Compatibility**
  - **Validates: Requirements 3.7, 3.8**

### Enhanced Console Features

- [ ] 7. Implement early console initialization
**Priority: HIGH FUNCTIONALITY - Critical for debug output across all stages**
**Dependencies: Task 6 (RISC-V drivers)**

Ensure console output is available as early as possible across all architectures.

- Initialize console with highest priority among kernel subsystems
- Initialize before any subsystem producing debug output
- Initialize immediately after minimum required dependencies
- Enable output capture for subsequent initializations
- Implement fallback methods when console initialization fails
- Provide emergency output capabilities
- Handle pre-console message buffering
- Flush buffered messages after initialization
- Maintain initialization order documentation
- Validate console readiness before proceeding
- _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 2.10_

- [ ] 7.1 Write property test for early console availability
  - **Property 9: Early Console Availability**
  - **Validates: Requirements 2.1, 2.2**

- [ ] 8. Implement console configuration interface
**Priority: MEDIUM ENHANCEMENT - Runtime configuration support**
**Dependencies: Task 7 (early console initialization)**

Implement runtime console configuration through the KFS interface.

- Expose configuration via `//kern/console/config`
- Support runtime log level adjustment
- Support console device enable/disable operations
- Support console output redirection
- Provide console statistics and status information
- Support console buffer size configuration
- Handle configuration validation and error checking
- Provide configuration change notifications
- Maintain configuration persistence where possible
- Support configuration rollback on errors
- _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6, 7.7, 7.8, 7.9, 7.10_

- [ ] 8.1 Write property test for console configuration
  - **Property 10: Console Configuration Management**
  - **Validates: Requirements 7.1, 7.2, 7.7, 7.10**

- [ ] 9. Final checkpoint - Comprehensive console system validation
Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks are organized by development stage to support incremental approach
- Stage 1 focuses on minimal direct serial output for Hello World kernel
- Stage 4 implements full multi-device console system for enhanced kernel
- Stages 5-6 add architecture-specific drivers for ARM64 and RISC-V
- Each stage builds upon previous console capabilities
- Property tests validate universal correctness properties
- Console system grows from minimal to comprehensive functionality
- Early console availability critical for debug output across all stages
- Architecture-specific drivers added when architectures are introduced
