# Design Document

## Overview

The Memory Management subsystem provides comprehensive memory allocation, protection, and management services for the FMI/OS kernel. It implements a sophisticated multi-layer memory management system with a slab allocator for kernel memory, architecture-specific page management, and virtual memory protection with comprehensive statistics and monitoring.

## Architecture

### Memory Management Hierarchy

```
Kernel Code → kmalloc()/kfree() → Slab Allocator → Page Allocator → Physical Memory
     ↓              ↓                    ↓               ↓              ↓
  Memory Requests  Size Classes      Page Allocation  Arch-Specific   Hardware RAM
  VMA Management   Bitmap Tracking   3-Level Tables   Memory Detection
```

### Multi-Layer Architecture

1. **Slab Allocator Layer**: Efficient kernel memory allocation with power-of-2 size classes
2. **Page Allocator Layer**: Architecture-specific physical page management
3. **Virtual Memory Layer**: Process isolation and memory protection
4. **Statistics Layer**: Comprehensive monitoring and reporting

### Architecture-Specific Components

```
arch/x86_64/lib/page.c     # x86_64 page allocator with PAE/long mode support
arch/arm64/lib/page.c      # ARM64 page allocator with LPAE support
arch/riscv64/lib/page.c    # RISC-V page allocator with Sv39/Sv48 support
```

## Components and Interfaces

### Slab Allocator Interface

```c
// Internal slab allocator functions
void *_kmalloc(size_t size);
void _kfree(void *ptr);
void *_krealloc(void *ptr, size_t new_size);
void *_kcalloc(size_t nmemb, size_t size);

// Slab cache management
struct slab_cache_s {
    size_t obj_size;
    size_t objects_per_slab;
    list_t available_slabs;
    list_t full_slabs;
    spinlock_t cache_lock;
    uint64_t alloc_count;
    uint64_t free_count;
    uint32_t *free_bitmap;
} slab_cache_t;

// Size class management
enum size_class {
    SIZE_16 = 0, SIZE_32, SIZE_64, SIZE_128,
    SIZE_256, SIZE_512, SIZE_1024, SIZE_2048, SIZE_4096,
    MAX_SIZE_CLASSES
};
```

### Page Allocator Interface

```c
// Page allocation functions
void *pages_alloc(size_t page_count);
void pages_free(void *addr, size_t page_count);
int pages_mark_invalid(void *addr, size_t page_count);

// Page state management
enum page_state {
    PAGE_FREE = 0,
    PAGE_ALLOCATED = 1,
    PAGE_INVALID = 2,
    PAGE_RESERVED = 3
};

// 3-level page table structure
struct page_table_l1_s {
    struct page_table_l2_s *l2_tables[1024];  // 1GB regions
    spinlock_t l1_lock;
} page_table_l1_t;

struct page_table_l2_s {
    struct page_table_l3_s *l3_tables[64];    // 16MB regions
    spinlock_t l2_lock;
} page_table_l2_t;

struct page_table_l3_s {
    uint32_t page_bitmap[2048];                // 256KB regions, 2-bit per page
    spinlock_t l3_lock;
    uint32_t free_count;
} page_table_l3_t;
```

### Virtual Memory Management

```c
// Virtual Memory Area management
struct vma_s {
    uint64_t start_addr;
    uint64_t end_addr;
    uint32_t permissions;
    uint32_t flags;
    struct list_head vma_list;
    kobj_t vma_kobj;
} vma_t;

// Memory space per process
struct memory_space_s {
    list_t vma_list;
    spinlock_t space_lock;
    uint64_t total_size;
    uint64_t used_size;
    kobj_t space_kobj;
} memory_space_t;

// Memory management functions
int memory_map_region(memory_space_t *space, uint64_t addr, size_t size, uint32_t perms);
int memory_unmap_region(memory_space_t *space, uint64_t addr, size_t size);
int memory_protect_region(memory_space_t *space, uint64_t addr, size_t size, uint32_t perms);
```

## Data Models

### Memory Statistics Structure

```c
struct memory_stats_s {
    // Physical memory statistics
    struct {
        uint64_t total;
        uint64_t free;
        uint64_t used;
        uint64_t cached;
    } physical;
    
    // Virtual memory statistics
    struct {
        uint64_t total;
        uint64_t allocated;
        uint64_t reserved;
    } virtual;
    
    // System statistics
    struct {
        uint64_t allocations;
        uint64_t faults;
        uint64_t swapping;
    } system;
    
    // Pool statistics
    struct {
        uint32_t pool_count;
        uint32_t utilization;
        uint32_t fragmentation;
    } pools;
    
    spinlock_t stats_lock;
} memory_stats_t;
```

### Memory Detection Interface

```c
struct memory_region_s {
    uint64_t base_addr;
    uint64_t length;
    enum memory_type type;
} memory_region_t;

struct memory_map_s {
    memory_region_t regions[128];
    size_t region_count;
    uint64_t total_memory;
    uint64_t usable_memory;
    uint64_t highest_addr;
} memory_map_t;

// Memory detection functions
int memory_detect_from_bootloader(boot_info_t *boot_info, memory_map_t *map);
bool memory_region_is_usable(const memory_region_t *region);
uint64_t memory_calculate_usable(const memory_map_t *map);
```

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Slab Allocation Consistency
*For any* successful kmalloc() call, the returned pointer should be valid, properly aligned, and the corresponding kfree() should succeed without corruption.
**Validates: Requirements 1.1, 1.2**

### Property 2: Page Allocation State Consistency
*For any* page allocation or deallocation operation, the page state should be correctly updated and no double-allocation should occur.
**Validates: Requirements 2.1, 2.7**

### Property 3: Memory Protection Boundary Enforcement
*For any* process memory access, the virtual memory manager should enforce protection boundaries and prevent unauthorized access.
**Validates: Requirements 3.2, 3.3**

### Property 4: Memory Statistics Accuracy
*For any* memory operation, the statistics should be updated atomically and reflect the actual memory state accurately.
**Validates: Requirements 4.10, 4.11**

### Property 5: Allocation Safety Detection
*For any* memory corruption or double-free condition, the safety mechanisms should detect and report the error appropriately.
**Validates: Requirements 5.2, 5.3**

### Property 6: Memory Initialization Ordering
*For any* memory subsystem initialization, the dependencies should be satisfied and initialization should complete successfully.
**Validates: Requirements 6.1, 6.2, 6.3, 6.4**

### Property 7: VMA Management Consistency
*For any* VMA operation (map, unmap, protect), the VMA list should remain consistent and memory regions should not overlap inappropriately.
**Validates: Requirements 3.6, 3.9**

## Error Handling

### Slab Allocator Errors
- Out of memory → Return NULL and update statistics
- Double-free detection → Log error and halt in debug mode
- Corruption detection → Log detailed error and attempt recovery

### Page Allocator Errors
- Invalid page state transition → Log error and maintain current state
- Memory region conflicts → Resolve using bootloader priority
- Hardware memory errors → Mark pages invalid and continue

### Virtual Memory Errors
- Invalid memory access → Generate page fault and handle appropriately
- VMA overlap conflicts → Reject operation and return error
- Permission violations → Log security event and deny access

### Statistics Errors
- Atomic update failure → Retry operation with backoff
- Statistics corruption → Reset to known good state
- Access synchronization failure → Use emergency read-only mode

## Testing Strategy

### Unit Testing
- Test slab allocator with various allocation patterns and sizes
- Test page allocator with different memory configurations
- Test VMA management with various mapping scenarios
- Test memory statistics accuracy and atomic updates

### Property-Based Testing
- Generate random allocation/deallocation sequences and verify consistency
- Test concurrent memory operations for thread safety
- Validate memory protection with random access patterns
- Test memory statistics with random operation sequences

### Integration Testing
- QEMU testing with different memory configurations
- Multi-architecture testing (x86_64, ARM64, RISC-V)
- Bootloader integration testing with various memory maps
- Stress testing with high memory pressure scenarios

### Safety Testing
- Test double-free detection with intentional violations
- Test use-after-free detection with poisoned memory
- Test memory corruption detection with deliberate corruption
- Test allocation tracking and leak detection capabilities