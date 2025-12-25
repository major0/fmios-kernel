---
inclusion: always
---

# Security Mitigations and Hardening

This document establishes the security requirements and mitigations that must be implemented throughout the FMI/OS kernel to protect against various attack vectors, with particular focus on speculative execution attacks.

## Core Security Principles

1. **Defense in Depth**: Multiple layers of security mitigations
2. **Secure by Default**: All code includes security mitigations from the start
3. **Zero Tolerance**: No vulnerable patterns allowed in kernel code
4. **Architecture Awareness**: Security mitigations tailored to each target architecture
5. **Continuous Validation**: Build-time and runtime security validation

## Speculative Execution Attack Mitigations

### Universal Requirements

All kernel code MUST implement comprehensive protection against speculative execution attacks including Spectre, Meltdown, and related vulnerabilities.

**Compiler-Level Mitigations:**
- `-mretpoline` - Enable retpoline mitigations for indirect branches (x86_64)
- `-mindirect-branch=thunk-extern` - Use external thunks for indirect branches (x86_64)
- `-mfunction-return=thunk-extern` - Use external thunks for function returns (x86_64)
- `-mindirect-branch-register` - Restrict indirect branch targets to registers (x86_64)
- `-fcf-protection=full` - Enable Intel CET (Control Flow Enforcement Technology)
- `-fstack-protector-strong` - Enable stack canary protection

### Architecture-Specific Assembly Requirements

#### x86_64 Speculative Execution Mitigations

**Mandatory Patterns:**
```assembly
# Indirect calls MUST use retpoline or proper barriers
call *%rax          # FORBIDDEN
call __x86_indirect_thunk_rax  # CORRECT

# Memory barriers before sensitive operations
mov sensitive_data(%rip), %rax  # FORBIDDEN without barrier
mfence                          # REQUIRED before sensitive access
mov sensitive_data(%rip), %rax  # CORRECT

# Serializing instructions before privileged operations
mov %cr3, %rax      # FORBIDDEN without serialization
lfence              # REQUIRED before privileged access
mov %cr3, %rax      # CORRECT

# Branch prediction barriers after conditional branches
cmp $secret, %rax
je sensitive_path   # FORBIDDEN without barrier
lfence              # REQUIRED after conditional branch
# sensitive code here
```

**Prohibited Patterns:**
- Direct indirect jumps: `jmp *%rax`
- Unprotected indirect calls: `call *%rax`
- Speculative memory access without barriers
- Conditional branches on secret data without `lfence`

#### ARM64 Speculative Execution Mitigations

**Mandatory Patterns:**
```assembly
# Speculation barriers after indirect branches
br x0               // FORBIDDEN without barrier
dsb sy              // REQUIRED after indirect branch
isb                 // REQUIRED instruction synchronization barrier

# Memory barriers before sensitive operations
ldr x0, =sensitive_data  // FORBIDDEN without barrier
dmb sy                   // REQUIRED before sensitive access
ldr x0, [x1]            // CORRECT

# Instruction synchronization before privileged operations
msr ttbr0_el1, x0   // FORBIDDEN without synchronization
isb                 // REQUIRED before privileged access
```

**Prohibited Patterns:**
- Unprotected indirect branches: `br x0`
- Speculative loads without memory barriers
- Privileged operations without instruction synchronization

#### RISC-V Speculative Execution Mitigations

**Mandatory Patterns:**
```assembly
# Fence instructions after indirect branches
jalr x0             # FORBIDDEN without fence
fence.i             # REQUIRED after indirect branch

# Memory barriers before sensitive operations
ld x1, sensitive_data  # FORBIDDEN without barrier
fence rw,rw           # REQUIRED before sensitive access
ld x1, 0(x2)         # CORRECT

# Instruction fence after code modifications
sw x1, 0(x2)        # Code modification
fence.i             # REQUIRED after code change
```

**Prohibited Patterns:**
- Unprotected indirect jumps: `jalr` without `fence.i`
- Speculative memory access without proper fencing
- Code modifications without instruction fence

## Memory Safety Requirements

### Buffer Overflow Protection

**Stack Protection:**
- Stack canaries enabled via `-fstack-protector-strong`
- No variable-length arrays (VLAs) in kernel code
- All array accesses must be bounds-checked
- No unsafe string functions (`strcpy`, `sprintf`, etc.)

**Heap Protection:**
- Slab allocator includes metadata validation
- Double-free detection in `kfree()` implementation
- Use-after-free detection via poisoning freed memory
- Guard pages around sensitive allocations

### Bounds Violation and Off-by-One Detection

**Compile-Time Detection:**
- Enable `-fsanitize=bounds` for array bounds checking (when available)
- Use `-Warray-bounds` to detect compile-time bounds violations
- Enable `-Wstringop-overflow` to detect string operation overflows
- Use `-fstack-clash-protection` to detect stack clash attacks
- Enable `-D_FORTIFY_SOURCE=2` for enhanced bounds checking in debug builds

**Runtime Detection Mechanisms:**
```c
// Mandatory bounds checking macros for all array access
#define BOUNDS_CHECK(array, index, size) \
  do { \
    if ((index) >= (size)) { \
      kpanic("Bounds violation: index %zu >= size %zu at %s:%d", \
        (size_t)(index), (size_t)(size), __FILE__, __LINE__); \
    } \
  } while (0)

// Off-by-one detection for buffer operations
#define SAFE_BUFFER_COPY(dest, src, dest_size, src_size) \
  do { \
    if ((src_size) >= (dest_size)) { \
      kpanic("Buffer overflow: src_size %zu >= dest_size %zu at %s:%d", \
        (size_t)(src_size), (size_t)(dest_size), __FILE__, __LINE__); \
    } \
    memcpy((dest), (src), (src_size)); \
    (dest)[(src_size)] = '\0'; /* Null terminate if string */ \
  } while (0)

// Safe array indexing with bounds validation
#define SAFE_ARRAY_ACCESS(array, index, size) \
  ({ \
    BOUNDS_CHECK(array, index, size); \
    &(array)[(index)]; \
  })
```

**Memory Allocation Bounds Checking:**
```c
// Enhanced kmalloc with bounds tracking
typedef struct allocation_header_s {
  uint32_t magic;           // Magic number for validation
  size_t size;              // Allocated size
  uint32_t canary_start;    // Start canary
  // User data follows
  // End canary follows user data
} allocation_header_t;

#define ALLOC_MAGIC 0xDEADBEEF
#define CANARY_VALUE 0xCAFEBABE

void* kmalloc_safe(size_t size);
void kfree_safe(void* ptr);
void validate_allocation(void* ptr); // Check canaries and bounds
```

**String Operation Safety:**
```c
// REQUIRED: Use standard library safe functions with proper bounds checking
#include <string.h>  // Compiler-provided standard functions

// Safe string operations (REQUIRED)
size_t len = strlen(str);                                    // Standard function
strncpy(dest, src, sizeof(dest) - 1);                      // Safe standard function
dest[sizeof(dest) - 1] = '\0';                             // Ensure null termination
strncat(dest, src, sizeof(dest) - strlen(dest) - 1);       // Safe standard function
snprintf(buf, sizeof(buf), "format %s", str);              // Safe standard function

// Memory operations (REQUIRED)
memset(ptr, 0, size);                                       // Standard function
memcpy(dest, src, size);                                    // Standard function
int result = memcmp(ptr1, ptr2, size);                     // Standard function

// Prohibited unsafe functions (compile-time detection)
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
```

### Integer Overflow Protection

**Safe Arithmetic:**
- Use compiler built-ins for overflow detection: `__builtin_add_overflow()`
- Validate all size calculations before allocation
- Check for wraparound in pointer arithmetic
- Use `SIZE_MAX` checks for large allocations

**Overflow Detection Macros:**
```c
// Safe arithmetic operations with overflow detection
#define SAFE_ADD(a, b, result) \
  ({ \
    bool overflow = __builtin_add_overflow((a), (b), (result)); \
    if (overflow) { \
      kpanic("Integer overflow: %zu + %zu at %s:%d", \
        (size_t)(a), (size_t)(b), __FILE__, __LINE__); \
    } \
    !overflow; \
  })

#define SAFE_MUL(a, b, result) \
  ({ \
    bool overflow = __builtin_mul_overflow((a), (b), (result)); \
    if (overflow) { \
      kpanic("Integer overflow: %zu * %zu at %s:%d", \
        (size_t)(a), (size_t)(b), __FILE__, __LINE__); \
    } \
    !overflow; \
  })

// Safe size calculations for allocations
#define SAFE_ALLOC_SIZE(count, size) \
  ({ \
    size_t total; \
    if (!SAFE_MUL(count, size, &total)) { \
      return NULL; /* Overflow detected */ \
    } \
    if (total > SIZE_MAX / 2) { \
      kpanic("Allocation too large: %zu bytes at %s:%d", \
        total, __FILE__, __LINE__); \
    } \
    total; \
  })
```

**Pointer Arithmetic Safety:**
```c
// Safe pointer arithmetic with bounds checking
#define SAFE_PTR_ADD(ptr, offset, base, size) \
  ({ \
    uintptr_t ptr_val = (uintptr_t)(ptr); \
    uintptr_t base_val = (uintptr_t)(base); \
    uintptr_t end_val = base_val + (size); \
    uintptr_t new_ptr = ptr_val + (offset); \
    if (new_ptr < base_val || new_ptr >= end_val) { \
      kpanic("Pointer bounds violation: %p + %zu outside [%p, %p) at %s:%d", \
        (ptr), (size_t)(offset), (base), (void*)end_val, __FILE__, __LINE__); \
    } \
    (typeof(ptr))new_ptr; \
  })
```

## Control Flow Integrity

### Branch Protection

**Intel CET Support:**
- Enable CET via `-fcf-protection=full` when available
- Use shadow stack for return address protection
- Indirect branch tracking for forward-edge protection

**ARM Pointer Authentication:**
- Use ARM64 pointer authentication when available
- Sign return addresses and function pointers
- Validate signatures before use

### Function Pointer Protection

**Mandatory Practices:**
- No function pointers in writable memory sections
- Use const qualifiers for function pointer tables
- Validate function pointers before calls
- Use CFI (Control Flow Integrity) when available

## Kernel Address Space Layout Randomization (KASLR)

### KASLR Requirements

**Position Independence:**
- All kernel code compiled with `-fPIC`
- No absolute addresses in kernel code
- Relative addressing in all assembly code
- Support for loading at any physical address

**Entropy Sources:**
- Use hardware random number generators when available
- Combine multiple entropy sources (RDRAND, RDSEED, etc.)
- Minimum 8 bits of entropy for kernel base randomization
- Re-randomize on each boot

## Side-Channel Attack Mitigations

### Cache-Based Attacks

**Cache Line Alignment:**
- Align sensitive data structures to cache line boundaries
- Use cache-line-sized padding for isolation
- Implement constant-time algorithms for cryptographic operations

**Memory Access Patterns:**
- Avoid data-dependent memory access patterns
- Use lookup tables with constant access patterns
- Implement cache-oblivious algorithms where possible

### Timing Attack Mitigations

**Constant-Time Operations:**
- All cryptographic operations must be constant-time
- Use bit manipulation instead of conditional branches for secrets
- Implement constant-time comparison functions

## Build System Security Validation

### Static Analysis Requirements

**Mandatory Checks:**
- Scan for vulnerable patterns in assembly code
- Detect missing speculation barriers
- Validate PIC compliance (no absolute addresses)
- Check for unsafe function usage
- **Detect bounds violations and off-by-one errors**
- **Validate array access patterns**
- **Check for integer overflow vulnerabilities**

**Automated Validation:**
```bash
# Example validation script
check_speculation_barriers() {
  # Scan for indirect branches without barriers
  grep -n "call \*" *.S && echo "ERROR: Unprotected indirect call found"
  grep -n "jmp \*" *.S && echo "ERROR: Unprotected indirect jump found"
}

check_pic_compliance() {
  # Scan for absolute addresses
  objdump -d kernel.o | grep -E "0x[0-9a-f]{8,}" && echo "ERROR: Absolute address found"
}

check_bounds_violations() {
  # Scan for unsafe array access patterns
  grep -n "\[\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\]" *.c | \
  grep -v "BOUNDS_CHECK\|SAFE_ARRAY_ACCESS" && \
  echo "ERROR: Unchecked array access found"

  # Scan for prohibited unsafe functions
  grep -n -E "(strcpy|strcat|sprintf|vsprintf|gets)\s*\(" *.c && \
  echo "ERROR: Unsafe string function found"

  # Scan for prohibited custom string implementations
  grep -n -E "(kstrlen|kstrcpy|kstrncpy|kstrcmp|kstrncmp)\s*\(" *.c && \
  echo "ERROR: Custom string function found - use standard library functions"

  # Scan for custom memory function implementations
  grep -n -E "k(memset|memcpy|memcmp|memmove)\s*\(" *.c && \
  echo "ERROR: Custom memory function found - use standard library functions"

  # Scan for prohibited unsafe functions
  grep -n -E "(strcpy|strcat|sprintf|gets)\s*\(" *.c && \
  echo "ERROR: Unsafe string function found"

  # Scan for potential off-by-one errors
  grep -n -E "(<=|>=)\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\+\s*1" *.c && \
  echo "WARNING: Potential off-by-one pattern found"
}

check_integer_overflow() {
  # Scan for arithmetic operations without overflow checks
  grep -n -E "\+\s*[a-zA-Z_][a-zA-Z0-9_]*\s*\*" *.c | \
  grep -v "SAFE_ADD\|SAFE_MUL\|__builtin.*_overflow" && \
  echo "ERROR: Unchecked arithmetic operation found"
}

# Compile-time bounds checking validation
validate_compile_flags() {
  echo "$CFLAGS" | grep -q "\-fsanitize=bounds" || \
  echo "WARNING: Bounds sanitizer not enabled"

  echo "$CFLAGS" | grep -q "\-Warray-bounds" || \
  echo "WARNING: Array bounds warnings not enabled"

  echo "$CFLAGS" | grep -q "\-fstack-protector-strong" || \
  echo "ERROR: Stack protection not enabled"
}
```

**Static Analysis Tools Integration:**
```bash
# Clang Static Analyzer for bounds checking
clang --analyze -Xanalyzer -analyzer-checker=security.insecureAPI.strcpy \
    -Xanalyzer -analyzer-checker=alpha.security.ArrayBound \
    -Xanalyzer -analyzer-checker=alpha.security.MallocOverflow \
    *.c

# Cppcheck for additional bounds checking
cppcheck --enable=all --error-exitcode=1 \
      --suppress=missingIncludeSystem \
      --check-config *.c

# Custom bounds checking script
./scripts/check_bounds_safety.sh
```

### Runtime Validation

**Boot-Time Checks:**
- Verify KASLR is enabled and working
- Check CPU vulnerability mitigations are active
- Validate stack canaries are functioning
- Test speculation barrier effectiveness
- **Validate bounds checking mechanisms are active**
- **Test allocation canary protection**
- **Verify integer overflow detection is working**

**Runtime Bounds Validation:**
```c
// Boot-time validation of bounds checking
void validate_bounds_protection(void) {
  // Test array bounds checking
  char test_array[10];
  bool bounds_check_working = false;

  // This should trigger bounds check (in debug build)
  #ifdef DEBUG
  // Intentionally trigger bounds violation to test detection
  volatile int test_index = 15;
  if (setjmp(bounds_violation_handler)) {
    bounds_check_working = true;
    kprintf("[BOOT] Bounds checking: ACTIVE\n");
  } else {
    // This should panic in debug builds
    test_array[test_index] = 'X';
    kprintf("[BOOT] Bounds checking: FAILED\n");
  }
  #else
  kprintf("[BOOT] Bounds checking: ENABLED (release mode)\n");
  #endif
}

// Test allocation canary protection
void validate_canary_protection(void) {
  void* test_ptr = kmalloc_safe(64);
  if (test_ptr) {
    // Validate canaries are in place
    validate_allocation(test_ptr);
    kfree_safe(test_ptr);
    kprintf("[BOOT] Allocation canaries: ACTIVE\n");
  } else {
    kprintf("[BOOT] Allocation canaries: FAILED\n");
  }
}

// Test integer overflow detection
void validate_overflow_detection(void) {
  size_t result;
  size_t large_val = SIZE_MAX - 1;

  // This should detect overflow
  if (!SAFE_ADD(large_val, 10, &result)) {
    kprintf("[BOOT] Integer overflow detection: ACTIVE\n");
  } else {
    kprintf("[BOOT] Integer overflow detection: FAILED\n");
  }
}
```

## Compliance Requirements

### Mandatory Security Features

All kernel code MUST demonstrate:

1. **Speculative Execution Protection**: Proper barriers and mitigations in all assembly code
2. **Memory Safety**: Buffer overflow protection and bounds checking
3. **Control Flow Integrity**: Protected function pointers and return addresses
4. **Position Independence**: No absolute addresses, full PIC compliance
5. **Side-Channel Resistance**: Constant-time algorithms and cache-aware design

### Security Review Process

**Code Review Checklist:**
- [ ] All indirect branches protected with appropriate barriers
- [ ] Memory barriers before sensitive data access
- [ ] No absolute addresses in assembly code
- [ ] Stack protection enabled for all functions with local arrays
- [ ] Integer overflow checks in size calculations
- [ ] Constant-time algorithms for sensitive operations
- [ ] Proper error handling without information leakage
- [ ] **All array accesses use bounds checking macros**
- [ ] **No unsafe string functions (strcpy, sprintf, etc.)**
- [ ] **Integer arithmetic uses overflow detection**
- [ ] **Pointer arithmetic includes bounds validation**
- [ ] **Memory allocations include canary protection**
- [ ] **Buffer operations check for off-by-one errors**

### Testing Requirements

**Security Testing:**
- Unit tests for speculation barrier effectiveness
- Fuzzing tests for buffer overflow detection
- Timing analysis for side-channel resistance
- KASLR entropy validation tests
- **Comprehensive bounds violation testing**
- **Off-by-one error detection tests**
- **Integer overflow vulnerability tests**

**Bounds Violation Unit Tests:**
```c
// Unit tests for bounds checking mechanisms
void test_array_bounds_checking(void) {
  char test_array[10];

  // Test valid access
  BOUNDS_CHECK(test_array, 5, 10); // Should pass

  // Test boundary conditions
  BOUNDS_CHECK(test_array, 9, 10); // Should pass (last valid index)

  // Test off-by-one detection
  expect_panic(BOUNDS_CHECK(test_array, 10, 10)); // Should panic
  expect_panic(BOUNDS_CHECK(test_array, -1, 10)); // Should panic
}

void test_safe_string_operations(void) {
  char dest[10];
  char src[] = "test";

  // Test safe copy
  assert(kstrncpy_safe(dest, src, sizeof(dest)) == 0);
  assert(strcmp(dest, "test") == 0);

  // Test overflow detection
  char long_src[] = "this string is too long for destination";
  expect_failure(kstrncpy_safe(dest, long_src, sizeof(dest)));
}

void test_allocation_bounds(void) {
  // Test normal allocation
  void* ptr = kmalloc_safe(64);
  assert(ptr != NULL);
  validate_allocation(ptr); // Should pass

  // Test canary corruption detection
  char* char_ptr = (char*)ptr;
  char_ptr[-1] = 0xFF; // Corrupt start canary
  expect_panic(validate_allocation(ptr));

  kfree_safe(ptr);
}

void test_integer_overflow_detection(void) {
  size_t result;

  // Test normal arithmetic
  assert(SAFE_ADD(100, 200, &result));
  assert(result == 300);

  // Test overflow detection
  assert(!SAFE_ADD(SIZE_MAX, 1, &result));
  assert(!SAFE_MUL(SIZE_MAX / 2, 3, &result));
}

void test_pointer_arithmetic_safety(void) {
  char buffer[100];
  char* ptr = buffer + 50;

  // Test valid pointer arithmetic
  char* new_ptr = SAFE_PTR_ADD(ptr, 10, buffer, sizeof(buffer));
  assert(new_ptr == ptr + 10);

  // Test bounds violation
  expect_panic(SAFE_PTR_ADD(ptr, 60, buffer, sizeof(buffer)));
  expect_panic(SAFE_PTR_ADD(ptr, -60, buffer, sizeof(buffer)));
}
```

**Fuzzing Tests for Bounds Violations:**
```c
// Fuzzing framework for bounds checking
void fuzz_array_access(void) {
  char test_array[FUZZ_ARRAY_SIZE];

  for (int i = 0; i < FUZZ_ITERATIONS; i++) {
    int index = random_int();
    size_t size = random_size();

    // Test should either succeed or panic appropriately
    if (index >= 0 && index < size) {
      BOUNDS_CHECK(test_array, index, size); // Should pass
    } else {
      expect_panic(BOUNDS_CHECK(test_array, index, size));
    }
  }
}

void fuzz_string_operations(void) {
  for (int i = 0; i < FUZZ_ITERATIONS; i++) {
    char dest[FUZZ_BUFFER_SIZE];
    char* src = generate_random_string();
    size_t dest_size = random_size_up_to(FUZZ_BUFFER_SIZE);

    int result = kstrncpy_safe(dest, src, dest_size);

    // Verify no buffer overflow occurred
    validate_buffer_integrity(dest, dest_size);

    free(src);
  }
}
```

This comprehensive security framework ensures that FMI/OS maintains the highest security standards while protecting against both current and emerging attack vectors.
