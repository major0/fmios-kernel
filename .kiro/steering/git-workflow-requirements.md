---
inclusion: always
---

# Git Workflow Requirements

All development work MUST follow the established Git workflow to maintain code quality, traceability, and proper review processes.

## Critical Rule: Always Use Topic Branches

**NEVER work directly on the main branch.** All changes MUST be made on topic branches and submitted via pull requests.

### Before Starting Any Work

1. **Ensure you're on main branch**:
  ```bash
  git checkout main
  git pull origin main
  ```

2. **Create a topic branch** using the provided script:
  ```bash
  ./scripts/create-topic-branch.sh <type> <name> [task-reference]
  ```

3. **Examples**:
  ```bash
  ./scripts/create-topic-branch.sh topic memory-allocator memory#2.1
  ./scripts/create-topic-branch.sh fix console-buffer-overflow console#4.2
  ./scripts/create-topic-branch.sh docs build-instructions build-system#1
  ```

### Branch Types

- **topic** - New features or enhancements
- **fix** - Bug fixes
- **docs** - Documentation updates
- **refactor** - Code refactoring
- **test** - Test improvements
- **chore** - Maintenance tasks

## Task Completion Workflow on Topic Branches

When completing any task on a topic branch, follow this workflow:

### Required Steps

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

5. **Push topic branch**:
  ```bash
  git push -u origin <branch-name>
  ```

6. **Create pull request** - Use the GitHub URL provided after push

### Example Complete Workflow

```bash
# 1. Start from clean main
git checkout main
git pull origin main

# 2. Create topic branch
./scripts/create-topic-branch.sh topic slab-allocator memory#2.1

# 3. Make your code changes
# ... edit files ...

# 4. Update task status in .kiro/specs/memory/tasks.md
# Change: - [ ] 2.1 Implement slab allocator
# To:     - [x] 2.1 Implement slab allocator

# 5. Stage all changes
git add .

# 6. Commit with proper format
git commit -m "feat(memory): implement slab allocator core functionality

Add basic slab allocation with size classes and object pools.
Includes initialization, allocation, and deallocation functions.

Task: memory#2.1"

# 7. Push topic branch
git push -u origin topic/slab-allocator

# 8. Create pull request using the provided GitHub URL
```

## Commit Message Requirements

All commits MUST follow conventional commit format:

### Format
```
type(scope): description

Optional detailed description.

Task: spec-name#task-number
```

### Valid Types
- **feat** - New features
- **fix** - Bug fixes
- **docs** - Documentation changes
- **style** - Code style changes (formatting, etc.)
- **refactor** - Code refactoring
- **test** - Test additions or modifications
- **chore** - Maintenance tasks
- **build** - Build system changes
- **ci** - CI/CD changes

### Examples
```bash
feat(memory): implement slab allocator core functionality
fix(console): resolve serial output buffer overflow
docs(readme): update build instructions for ARM64
refactor(process): simplify process creation logic
test(memory): add property tests for slab allocator
```

## Branch Protection

The following branches are protected and cannot be pushed to directly:
- **main** - Primary development branch
- **master** - Legacy primary branch (if exists)

### Protection Enforcement

The pre-push hook will prevent direct pushes to protected branches with this error:
```
❌ Direct push to protected branch 'main' is not allowed

To contribute changes:
1. Create a topic branch: git checkout -b topic/your-feature
2. Make your changes and commit them
3. Push your topic branch: git push origin topic/your-feature
4. Create a pull request for review
```

## Pull Request Requirements

All changes MUST be submitted via pull requests:

1. **Create topic branch** from latest main
2. **Make changes** following coding standards
3. **Update task status** in spec files
4. **Commit with proper format** including task references
5. **Push topic branch** to origin
6. **Create pull request** with:
  - Clear description of changes
  - Task references in description
  - Link to relevant spec tasks

## Benefits

### Code Quality
- **Peer Review**: All changes reviewed before merging
- **Automated Validation**: Hooks validate commit format and branch protection
- **Traceability**: Clear connection between commits, tasks, and requirements

### Development Process
- **Parallel Work**: Multiple developers can work on different features simultaneously
- **Clean History**: Linear history on main branch through proper merging
- **Rollback Safety**: Easy to revert problematic changes

### Project Management
- **Task Tracking**: Clear progress tracking through task status updates
- **Requirement Traceability**: Direct links from code changes to specifications
- **Documentation**: Commit messages and PR descriptions provide change context

## Enforcement

This workflow is enforced by:

### Git Hooks
- **commit-msg hook**: Validates conventional commit format and task references
- **pre-push hook**: Prevents direct pushes to protected branches

### Scripts
- **create-topic-branch.sh**: Ensures proper branch naming and task validation
- **setup-git-hooks.sh**: Installs and configures all necessary hooks

### Manual Process
- **Task status updates**: Manual updates ensure developer awareness of progress
- **Pull request reviews**: Human review ensures code quality and standards compliance

## Troubleshooting

### Common Issues

**"Direct push to protected branch not allowed"**
- Solution: Create a topic branch and push that instead

**"Invalid commit message format"**
- Solution: Follow conventional commit format with proper type and scope

**"No task reference found"**
- Solution: Add `Task: spec-name#task-number` to commit message

**"Branch name doesn't follow conventions"**
- Solution: Use the create-topic-branch.sh script for proper naming

### Getting Help

1. **Check hook output**: Git hooks provide detailed error messages and examples
2. **Use provided scripts**: The create-topic-branch.sh script handles most complexity
3. **Follow examples**: This document provides complete workflow examples
4. **Review existing PRs**: Look at merged pull requests for reference

This Git workflow ensures high code quality, proper traceability, and smooth collaboration while maintaining the flexibility needed for effective development.
