---
inclusion: always
---

# Security and Capabilities Framework

All FMI/OS subsystems must implement comprehensive security measures based on a capabilities model. This ensures system-wide security without traditional root privilege escalation vulnerabilities.

## Core Security Principles

### Capabilities-Based Security Model
- **Fine-Grained Access Control**: All system resource access controlled by capabilities
- **No Root Privilege**: POSIX compatibility without root privilege escalation hole
- **Process Isolation**: Memory protection and capability restrictions between processes
- **Secure by Default**: All operations require explicit capability grants

### Defense in Depth
- **Multiple Security Layers**: Overlapping security mechanisms
- **Fail-Safe Design**: Security failures result in denial rather than compromise
- **Least Privilege**: Minimum necessary capabilities for each operation
- **Audit and Monitoring**: Comprehensive logging of security-relevant operations

## Capability System Integration

### Subsystem Requirements
All kernel subsystems must integrate with the capability system:

#### KFS Access Control
- **Filesystem Permissions**: Enforce capabilities through filesystem-style permissions
- **Path-Based Security**: Different capabilities for different KFS paths
- **Service Registration**: Secure service registration with capability validation
- **Information Access**: Capability-controlled access to kernel and process information

#### IPC Security
- **Communication Authentication**: Authenticate and authorize inter-process communications
- **Channel Security**: Secure IPC channel establishment and access control
- **Message Validation**: Validate message content and sender capabilities
- **Memory Mapping Security**: Secure page mapping with capability checks

#### Process and Thread Security
- **Process Creation**: Capability inheritance and restriction during process creation
- **Namespace Isolation**: Secure namespace customization with capability controls
- **Resource Limits**: Capability-based resource allocation and limits
- **Container Security**: OCI-compliant container security through capability restriction

#### Memory Management Security
- **Memory Protection**: Enforce memory boundaries through capabilities
- **Allocation Control**: Capability-based memory allocation permissions
- **Shared Memory**: Secure shared memory access with capability validation
- **Resource Accounting**: Track memory usage per capability set

## Security Implementation Requirements

### Access Control Lists
Each subsystem must implement capability-based access control:
```c
// Example capability check
if (!capability_check(current_process, CAP_MEMORY_ALLOCATE)) {
    return -EPERM;
}
```

### Audit Logging
All security-relevant operations must be logged:
```c
// Security audit logging
audit_log(AUDIT_CAPABILITY_CHECK, "Process %d attempted %s operation",
          process_id, operation_name);
```

### Secure Boot Integration
- **Boot-Time Capabilities**: Establish initial capability grants during secure boot
- **Integrity Verification**: Verify system integrity before capability establishment
- **Trust Chain**: Maintain trust chain from bootloader through kernel initialization
- **Recovery Mechanisms**: Secure recovery options for boot failures

## Speculative Execution Attack Mitigations

### Universal Requirements
All kernel code must implement protection against speculative execution attacks:

#### Compiler-Level Mitigations
- **Retpoline**: Enable `-mretpoline` for indirect branch protection
- **CET Support**: Use `-fcf-protection=full` for Intel Control Flow Enforcement
- **Stack Protection**: Enable `-fstack-protector-strong` for stack canary protection
- **Bounds Checking**: Use `-fsanitize=bounds` in debug builds

#### Assembly Code Requirements
- **Memory Barriers**: Use appropriate barriers before sensitive operations
- **Serializing Instructions**: Use `lfence`, `mfence`, `cpuid` before privileged operations
- **Indirect Branch Protection**: Use retpoline or proper barriers for indirect calls
- **Speculation Barriers**: Architecture-specific speculation barriers after branches

### Architecture-Specific Mitigations

#### x86_64 Requirements
```assembly
# Required patterns
call *%rax          # FORBIDDEN
call __x86_indirect_thunk_rax  # CORRECT

# Memory barriers before sensitive operations
mfence
mov sensitive_data(%rip), %rax

# Serializing instructions before privileged operations
lfence
mov %cr3, %rax
```

#### ARM64 Requirements
```assembly
# Speculation barriers
br x0               // FORBIDDEN without barrier
dsb sy              // REQUIRED after indirect branch
isb                 // REQUIRED instruction synchronization
```

#### RISC-V Requirements
```assembly
# Fence instructions
jalr x0             # FORBIDDEN without fence
fence.i             # REQUIRED after indirect branch
```

## Memory Safety Requirements

### Buffer Overflow Protection
- **Stack Canaries**: Enable stack protection for all functions with local arrays
- **Bounds Checking**: Validate all array accesses and buffer operations
- **Safe String Functions**: Use bounds-checked string operations
- **Integer Overflow**: Detect and prevent integer overflow in size calculations

### Memory Corruption Detection
- **Allocation Canaries**: Protect memory allocations with canary values
- **Double-Free Detection**: Detect and prevent double-free conditions
- **Use-After-Free**: Poison freed memory to detect use-after-free
- **Heap Validation**: Validate heap metadata and detect corruption

## Subsystem Security Requirements

### Console Security
- **Output Control**: Capability-based console output permissions
- **Input Validation**: Validate all console input and commands
- **Privilege Separation**: Separate console drivers from console management
- **Audit Logging**: Log security-relevant console operations

### Memory Security
- **Allocation Permissions**: Capability-based memory allocation control
- **Protection Boundaries**: Enforce memory protection between processes
- **Secure Cleanup**: Secure memory cleanup on process termination
- **Resource Limits**: Capability-based memory resource limits

### IPC Security
- **Channel Authentication**: Authenticate IPC channel establishment
- **Message Integrity**: Validate message integrity and authenticity
- **Access Control**: Capability-based IPC access control
- **Secure Forwarding**: Secure message forwarding between kernel and userland

### Process Security
- **Creation Control**: Capability-based process creation permissions
- **Namespace Security**: Secure namespace inheritance and modification
- **Resource Isolation**: Isolate process resources through capabilities
- **Container Security**: Secure container implementation through capability restriction

## Security Testing Requirements

### Security Validation
- **Penetration Testing**: Regular security testing and vulnerability assessment
- **Fuzzing**: Comprehensive fuzzing of all external interfaces
- **Static Analysis**: Static code analysis for security vulnerabilities
- **Dynamic Analysis**: Runtime security validation and monitoring

### Compliance Verification
- **Capability Enforcement**: Verify capability system enforcement
- **Access Control**: Validate access control mechanisms
- **Audit Completeness**: Ensure comprehensive audit logging
- **Security Policy**: Verify security policy implementation and enforcement

This security framework ensures that all FMI/OS subsystems maintain the highest security standards while providing the flexibility and performance required for a modern operating system.
