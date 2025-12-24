# Spec Organization Analysis and Cleanup Plan

## Current State Analysis

### Existing Specs
1. **bootstrap/** - Architecture-specific boot process
2. **build-system/** - Autoconf/automake build system
3. **ceedling-setup/** - Ceedling testing framework setup
4. **console/** - Console driver management
5. **git-workflow/** - Git workflow integration (in progress)
6. **ipc/** - Inter-process communication
7. **kernel/** - Core kernel functionality
8. **kfs-namespace/** - Kernel filesystem namespace
9. **memory/** - Memory management
10. **testing-framework/** - Comprehensive testing framework
11. **threads-processes/** - Thread and process management

### Summary Documents
- **INCREMENTAL_DEVELOPMENT_ROADMAP.md** - Development roadmap
- **INCREMENTAL_RESTRUCTURING_SUMMARY.md** - Restructuring summary
- **REFACTORING_SUMMARY.md** - Refactoring summary

## Identified Issues

### 1. Duplicate Testing Specs
- **testing-framework/** - Comprehensive testing framework specification
- **ceedling-setup/** - Specific Ceedling setup requirements

**Analysis**: These are overlapping. `testing-framework` is comprehensive and includes Ceedling as part of the overall testing strategy. `ceedling-setup` appears to be a more focused, earlier version.

**Recommendation**: Merge `ceedling-setup` into `testing-framework` and remove the duplicate.

### 2. Combined Specs That Should Be Split
- **threads-processes/** - Combines two distinct kernel subsystems

**Analysis**: Threads and processes are related but distinct subsystems that should have separate specifications for clarity and maintainability.

**Recommendation**: Split into `threads/` and `processes/` specs.

### 3. Summary Document Organization
The three summary documents at the root level should be organized better:

**Recommendation**: Move to a `planning/` subdirectory or integrate into relevant specs.

## Cleanup Plan

### Phase 1: Merge Duplicate Testing Specs
1. Review both `testing-framework` and `ceedling-setup` requirements
2. Merge unique requirements from `ceedling-setup` into `testing-framework`
3. Update `testing-framework` design to include specific Ceedling setup details
4. Remove `ceedling-setup/` directory
5. Update any references to the old spec

### Phase 2: Split Combined Specs
1. Create separate `threads/` and `processes/` spec directories
2. Split requirements from `threads-processes` into appropriate specs
3. Split design and tasks accordingly
4. Remove `threads-processes/` directory
5. Update cross-references between the new specs

### Phase 3: Organize Summary Documents
1. Create `planning/` subdirectory
2. Move summary documents to `planning/`
3. Update any references to these documents

### Phase 4: Validate Spec Completeness
1. Review all specs for completeness (requirements.md, design.md, tasks.md)
2. Ensure no missing dependencies between specs
3. Validate that all kernel subsystems are covered

## Final Spec Structure

```
.kiro/specs/
├── planning/
│   ├── INCREMENTAL_DEVELOPMENT_ROADMAP.md
│   ├── INCREMENTAL_RESTRUCTURING_SUMMARY.md
│   └── REFACTORING_SUMMARY.md
├── bootstrap/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── build-system/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── console/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── git-workflow/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── ipc/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── kernel/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── kfs-namespace/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── memory/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── processes/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
├── testing-framework/
│   ├── requirements.md
│   ├── design.md
│   └── tasks.md
└── threads/
    ├── requirements.md
    ├── design.md
    └── tasks.md
```

## Implementation Priority

1. **High Priority**: Merge testing specs (affects current development)
2. **Medium Priority**: Split threads-processes (affects kernel architecture)
3. **Low Priority**: Organize summary documents (cosmetic improvement)

This cleanup will result in a clean, organized spec structure with no duplicates and clear separation of concerns.