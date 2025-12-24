---
inclusion: always
---

# Task Completion Workflow

When completing any task, you MUST follow this workflow to maintain proper traceability:

## Required Steps

1. **Complete the implementation** - Finish all code changes for the task
2. **Update task status** - Manually change the task status in the spec file:
   - Change `- [ ]` to `- [-]` when starting work (in progress)
   - Change `- [-]` to `- [x]` when completing work (completed)
3. **Stage all changes** - Add both implementation files AND the updated spec file:
   ```bash
   git add .
   ```
4. **Commit with proper format** - Use conventional commit format with task reference:
   ```bash
   git commit -m "feat(scope): description of changes

   Detailed description if needed.

   Task: spec-name#task-number"
   ```

## Example Workflow

```bash
# 1. Make your code changes
# ... edit files ...

# 2. Update task status in .kiro/specs/memory/tasks.md
# Change: - [-] 2.1 Implement slab allocator
# To:     - [x] 2.1 Implement slab allocator

# 3. Stage all changes
git add .

# 4. Commit with proper format
git commit -m "feat(memory): implement slab allocator core functionality

Add basic slab allocation with size classes and object pools.
Includes initialization, allocation, and deallocation functions.

Task: memory#2.1"
```

## Benefits

- **Simple**: No complex scripts needed
- **Traceable**: Each commit includes both implementation and task status
- **Atomic**: Task completion is atomic with the implementation
- **Conventional**: Uses standard git workflow with conventional commits

## Enforcement

This workflow is enforced by:
- Pre-commit hooks validating conventional commit format
- Commit message hooks validating task references
- Manual task status updates ensure developer awareness of progress