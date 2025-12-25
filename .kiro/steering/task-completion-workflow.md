---
inclusion: always
---

# Git Workflow and Task Completion Requirements

All development work MUST follow the established Git workflow to maintain code quality, traceability, and proper review processes.

## Critical Rule: Always Use Topic Branches

**NEVER work directly on the main branch.** All changes MUST be made on topic branches and submitted via pull requests.

## Before Starting Any Work

**MANDATORY STEPS** before creating any topic branch:

1. **Switch to main branch**:
   ```bash
   git checkout main
   ```

2. **Pull latest changes** (CRITICAL - always do this):
   ```bash
   git pull origin main
   ```

3. **Create topic branch** with proper naming:
   ```bash
   git checkout -b <type>/<descriptive-name>
   ```

### Branch Types
- **topic** - New features or enhancements
- **fix** - Bug fixes  
- **docs** - Documentation updates
- **refactor** - Code refactoring
- **test** - Test improvements
- **chore** - Maintenance tasks

### Example Branch Creation
```bash
# Always start fresh from main
git checkout main
git pull origin main

# Create appropriately named topic branch
git checkout -b topic/memory-allocator
git checkout -b fix/console-buffer-overflow  
git checkout -b docs/build-instructions
```

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

6. **MANDATORY: Create pull request immediately using GitHub CLI**:
   ```bash
   gh pr create --title "feat(scope): description of changes" --body "Detailed description with task references"
   ```

## Complete Example Workflow

```bash
# 1. ALWAYS start from clean, updated main
git checkout main
git pull origin main

# 2. Create topic branch
git checkout -b topic/slab-allocator

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

# 8. MANDATORY: Create pull request immediately using GitHub CLI
gh pr create --title "feat(memory): implement slab allocator core functionality" \
             --body "## Summary
Implement core slab allocator functionality for kernel memory management.

## Changes
- Add slab cache creation and management
- Implement object allocation and deallocation
- Add size class optimization

## Task References
- Implements: memory#2.1

## Testing
- [x] Code compiles without errors
- [x] Basic allocation tests pass
- [x] Memory leak tests pass"
```

## Pull Request Creation with GitHub CLI

**CRITICAL: Every pushed topic branch MUST have a pull request created immediately using the GitHub CLI tool (`gh`).**

### GitHub CLI Installation

Ensure the GitHub CLI is installed and authenticated:
```bash
# Install GitHub CLI (if not already installed)
# On macOS: brew install gh
# On Ubuntu: sudo apt install gh
# On other systems: see https://cli.github.com/

# Authenticate with GitHub (one-time setup)
gh auth login
```

### PR Creation Process

After pushing your topic branch, immediately create a PR using the GitHub CLI:

```bash
# Basic PR creation
gh pr create --title "feat(scope): description of changes" --body "Detailed description"

# PR creation with comprehensive template
gh pr create --title "feat(memory): implement slab allocator" \
             --body "## Summary
Implement core slab allocator functionality for kernel memory management.

## Changes
- Add slab cache creation and management
- Implement object allocation and deallocation
- Add size class optimization

## Task References
- Implements: memory#2.1

## Testing
- [x] Code compiles without errors
- [x] Basic allocation tests pass
- [x] Memory leak tests pass

## Notes
This implementation provides the foundation for efficient kernel memory allocation."
```

### GitHub CLI PR Options

Useful `gh pr create` options:
- `--title "Title"` - Set PR title (required)
- `--body "Description"` - Set PR description (required)
- `--draft` - Create as draft PR for work-in-progress
- `--assignee @me` - Assign PR to yourself
- `--label "enhancement"` - Add labels to PR
- `--reviewer username` - Request specific reviewers

### Why GitHub CLI is Preferred

1. **Speed**: Create PRs directly from command line without browser
2. **Consistency**: Standardized PR creation process
3. **Automation**: Can be scripted and integrated into workflows
4. **Efficiency**: No context switching between terminal and browser
5. **Template Support**: Easy to use consistent PR templates

### PR Requirements

All pull requests MUST include:

1. **Clear Title**: Use conventional commit format
2. **Detailed Description**: Explain what changes were made and why
3. **Task References**: Include links to relevant spec tasks
4. **Testing Notes**: Describe how changes were tested
5. **Breaking Changes**: Note any breaking changes or dependencies

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
- **style** - Code style changes
- **refactor** - Code refactoring
- **test** - Test additions or modifications
- **chore** - Maintenance tasks
- **build** - Build system changes
- **ci** - CI/CD changes

## Branch Protection

The following branches are protected and cannot be pushed to directly:
- **main** - Primary development branch
- **master** - Legacy primary branch (if exists)

Direct pushes to protected branches will be rejected with:
```
❌ Direct push to protected branch 'main' is not allowed
```

## Why Git Pull is Critical

**Always run `git pull origin main` before creating topic branches** because:

1. **Prevents Merge Conflicts**: Ensures your branch starts from the latest code
2. **Maintains Linear History**: Reduces complex merge scenarios
3. **Catches Integration Issues Early**: Identifies conflicts before development
4. **Ensures Consistency**: All developers work from the same baseline
5. **Simplifies Reviews**: PRs show only your changes, not outdated diffs

## Benefits

### Code Quality
- **Peer Review**: All changes reviewed before merging via pull requests
- **Automated Validation**: Git hooks validate commit format and prevent direct pushes
- **Traceability**: Clear connection between commits, tasks, and requirements
- **Conflict Prevention**: Starting from latest main prevents integration issues

### Development Process
- **Parallel Work**: Multiple developers can work on different features simultaneously
- **Clean History**: Linear history on main branch through proper workflow
- **Rollback Safety**: Easy to revert problematic changes
- **Consistent Baseline**: All work starts from the same up-to-date foundation
- **Team Visibility**: PRs provide visibility into all ongoing work

## Enforcement

This workflow is enforced by:

### Git Hooks
- **commit-msg hook**: Validates conventional commit format and task references
- **pre-push hook**: Prevents direct pushes to protected branches and validates branch naming

### Manual Process Requirements
- **Git pull mandate**: Must pull latest main before creating topic branches
- **Task status updates**: Manual updates ensure developer awareness of progress
- **PR creation mandate**: Every pushed topic branch must have an immediate PR
- **Pull request reviews**: Human review ensures code quality and standards compliance

### Common Mistakes to Avoid

❌ **Working directly on main branch**
✅ **Always create topic branches**

❌ **Creating topic branch without git pull**
✅ **Always git pull origin main first**

❌ **Pushing directly to main**
✅ **Push topic branches and create PRs**

❌ **Creating PR through browser instead of CLI**
✅ **Use gh pr create for immediate PR creation**

❌ **Forgetting task references in commits**
✅ **Include Task: spec-name#task-number**

❌ **Pushing without immediately creating PR**
✅ **Always run gh pr create right after git push**

This Git workflow ensures high code quality, proper traceability, and smooth collaboration while preventing the most common integration issues through mandatory git pull requirements and streamlined PR creation via GitHub CLI.
