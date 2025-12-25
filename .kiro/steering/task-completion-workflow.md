---
inclusion: always
---

# Git Workflow and Task Completion Requirements

All development work MUST follow the established Git workflow to maintain code quality, traceability, and proper review processes.

## Critical Rule: Always Use Topic Branches

**NEVER work directly on the main branch.** All changes MUST be made on topic branches and submitted via pull requests.

## Before Starting Any Work

**SIMPLIFIED WORKFLOW** for creating topic branches:

1. **Create topic branch directly** with proper naming and upstream tracking:
   ```bash
   git checkout -b <type>/<descriptive-name> origin/main
   ```

2. **Pull latest changes** to ensure branch is up-to-date:
   ```bash
   git pull
   ```

### Branch Types
- **topic** - New features or enhancements
- **fix** - Bug fixes  
- **docs** - Documentation updates
- **refactor** - Code refactoring
- **test** - Test improvements
- **chore** - Maintenance tasks

### Why This Simplified Workflow is Better

Creating topic branches directly from `origin/main` and pulling afterward provides several advantages:

1. **Fewer Steps**: Eliminates need to checkout main and pull before branch creation
2. **Flexibility**: Can create branches from any current branch state
3. **Efficiency**: Reduces Git operations and context switching
4. **Upstream Tracking**: Branch automatically tracks origin/main for clear relationship
5. **Latest Code**: Pull after creation ensures branch has latest changes
6. **Cleaner History**: Avoids unnecessary local main branch updates

### Example Branch Creation
```bash
# Create topic branch directly from origin/main
git checkout -b topic/memory-allocator origin/main
git pull

# Create fix branch directly from origin/main
git checkout -b fix/console-buffer-overflow origin/main
git pull

# Create docs branch directly from origin/main
git checkout -b docs/build-instructions origin/main
git pull
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
# 1. Create topic branch directly from origin/main with upstream tracking
git checkout -b topic/slab-allocator origin/main

# 2. Pull latest changes to ensure branch is up-to-date
git pull

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

## Why Git Pull After Branch Creation is Important

**Running `git pull` after creating topic branches from `origin/main`** ensures:

1. **Latest Changes**: Branch contains the most recent commits from main
2. **Conflict Prevention**: Early detection of potential merge conflicts
3. **Consistent Base**: All developers work from the same code baseline
4. **Integration Safety**: Reduces risk of integration issues later
5. **Clean Development**: Start development with known good state

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

❌ **Creating topic branch without upstream tracking**
✅ **Always use git checkout -b branch-name origin/main**

❌ **Forgetting to pull after branch creation**
✅ **Always run git pull after creating topic branch**

❌ **Pushing directly to main**
✅ **Push topic branches and create PRs**

❌ **Creating PR through browser instead of CLI**
✅ **Use gh pr create for immediate PR creation**

❌ **Forgetting task references in commits**
✅ **Include Task: spec-name#task-number**

❌ **Pushing without immediately creating PR**
✅ **Always run gh pr create right after git push**

This Git workflow ensures high code quality, proper traceability, and smooth collaboration while preventing the most common integration issues through mandatory git pull requirements and streamlined PR creation via GitHub CLI.
