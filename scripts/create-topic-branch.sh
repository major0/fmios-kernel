#!/bin/bash
# Create topic branch script for FMI/OS development
# This script creates properly named topic branches linked to spec tasks

set -e

# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to show usage
show_usage() {
    echo "Usage: $0 <branch-type> <branch-name> [task-reference]"
    echo ""
    echo "Branch types:"
    echo "  topic     - New features or enhancements"
    echo "  fix       - Bug fixes"
    echo "  docs      - Documentation updates"
    echo "  refactor  - Code refactoring"
    echo "  test      - Test improvements"
    echo "  chore     - Maintenance tasks"
    echo ""
    echo "Examples:"
    echo "  $0 topic memory-allocator memory#2.1"
    echo "  $0 fix console-buffer-overflow console#4.2"
    echo "  $0 docs build-instructions build-system#1"
    echo "  $0 refactor process-management kernel#3.1"
    echo ""
    echo "Task reference format: spec-name#task-number"
    echo "Task reference is optional but recommended for traceability"
}

# Function to validate task reference exists
validate_task_reference() {
    local task_ref="$1"
    local spec_name=$(echo "$task_ref" | cut -d'#' -f1)
    local task_number=$(echo "$task_ref" | cut -d'#' -f2)
    
    local spec_file=".kiro/specs/$spec_name/tasks.md"
    
    if [ ! -f "$spec_file" ]; then
        echo -e "${YELLOW}⚠️  Warning: Spec file '$spec_file' not found${NC}"
        echo "Available specs:"
        if [ -d ".kiro/specs" ]; then
            find .kiro/specs -name "tasks.md" -exec dirname {} \; | sed 's|.kiro/specs/||' | sort
        fi
        return 1
    fi
    
    # Check if task exists in the spec file
    if ! grep -q "^- \[.\] $task_number\." "$spec_file" 2>/dev/null; then
        echo -e "${YELLOW}⚠️  Warning: Task '$task_number' not found in '$spec_name' spec${NC}"
        echo "Available tasks in $spec_name:"
        grep "^- \[.\] [0-9]" "$spec_file" | head -10 || echo "No tasks found"
        return 1
    fi
    
    return 0
}

# Function to validate branch name
validate_branch_name() {
    local branch_name="$1"
    
    # Check for valid characters (lowercase, numbers, hyphens)
    if ! echo "$branch_name" | grep -qE "^[a-z0-9-]+$"; then
        echo -e "${RED}❌ Invalid branch name: '$branch_name'${NC}"
        echo "Branch names should contain only lowercase letters, numbers, and hyphens"
        return 1
    fi
    
    # Check length (reasonable limit)
    if [ ${#branch_name} -gt 50 ]; then
        echo -e "${RED}❌ Branch name too long: ${#branch_name} characters (max 50)${NC}"
        return 1
    fi
    
    return 0
}

# Check if we're in a Git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}❌ Error: Not in a Git repository${NC}"
    exit 1
fi

# Check arguments
if [ $# -lt 2 ] || [ $# -gt 3 ]; then
    show_usage
    exit 1
fi

BRANCH_TYPE="$1"
BRANCH_NAME="$2"
TASK_REF="$3"

# Validate branch type
case "$BRANCH_TYPE" in
    topic|fix|docs|refactor|test|chore)
        ;;
    *)
        echo -e "${RED}❌ Invalid branch type: '$BRANCH_TYPE'${NC}"
        echo ""
        show_usage
        exit 1
        ;;
esac

# Validate branch name
if ! validate_branch_name "$BRANCH_NAME"; then
    exit 1
fi

# Construct full branch name
FULL_BRANCH_NAME="$BRANCH_TYPE/$BRANCH_NAME"

# Check if branch already exists
if git show-ref --verify --quiet "refs/heads/$FULL_BRANCH_NAME"; then
    echo -e "${RED}❌ Branch '$FULL_BRANCH_NAME' already exists${NC}"
    exit 1
fi

# Validate task reference if provided
if [ -n "$TASK_REF" ]; then
    if ! echo "$TASK_REF" | grep -qE "^[a-z0-9-]+#[0-9]+(\.[0-9]+)?$"; then
        echo -e "${RED}❌ Invalid task reference format: '$TASK_REF'${NC}"
        echo "Expected format: spec-name#task-number (e.g., memory#2.1)"
        exit 1
    fi
    
    # Validate task exists (warning only)
    validate_task_reference "$TASK_REF" || true
fi

# Ensure we're on main/master before creating branch
current_branch=$(git branch --show-current)
if [ "$current_branch" != "main" ] && [ "$current_branch" != "master" ]; then
    echo -e "${YELLOW}⚠️  Currently on branch '$current_branch'${NC}"
    echo -e "${BLUE}Switching to main branch before creating topic branch...${NC}"
    
    # Check for uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        echo -e "${RED}❌ You have uncommitted changes. Please commit or stash them first.${NC}"
        exit 1
    fi
    
    # Switch to main (or master if main doesn't exist)
    if git show-ref --verify --quiet refs/heads/main; then
        git checkout main
    elif git show-ref --verify --quiet refs/heads/master; then
        git checkout master
    else
        echo -e "${RED}❌ Neither 'main' nor 'master' branch found${NC}"
        exit 1
    fi
fi

# Pull latest changes
echo -e "${BLUE}Pulling latest changes...${NC}"
git pull origin "$(git branch --show-current)" || {
    echo -e "${YELLOW}⚠️  Could not pull latest changes. Continuing with local state.${NC}"
}

# Create and checkout the new branch with upstream tracking
echo -e "${GREEN}Creating topic branch: $FULL_BRANCH_NAME${NC}"
git checkout -b "$FULL_BRANCH_NAME" origin/main

# Branch is now tracking origin/main as upstream
echo -e "${BLUE}Branch '$FULL_BRANCH_NAME' created successfully with upstream tracking!${NC}"
echo ""
echo "Next steps:"
echo "1. Make your changes and commit them with conventional commit format"
echo "2. Push your branch: git push -u origin $FULL_BRANCH_NAME"
if [ -n "$TASK_REF" ]; then
    echo "3. Reference task '$TASK_REF' in your commit messages"
fi
echo "4. Create a pull request when ready for review"
echo ""
echo "Commit message template:"
if [ -n "$TASK_REF" ]; then
    echo "  $BRANCH_TYPE(scope): description of changes"
    echo ""
    echo "  Detailed description if needed."
    echo ""
    echo "  Task: $TASK_REF"
else
    echo "  $BRANCH_TYPE(scope): description of changes"
    echo ""
    echo "  Detailed description if needed."
fi
echo ""
echo -e "${GREEN}Happy coding! 🚀${NC}"