#!/bin/bash
# Setup Git hooks for FMI/OS development workflow
# This script installs the necessary Git hooks for conventional commits and workflow enforcement

set -e

# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "Setting up Git hooks for FMI/OS development workflow..."

# Check if we're in a Git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}Error: Not in a Git repository${NC}"
    exit 1
fi

# Get the Git hooks directory
HOOKS_DIR="$(git rev-parse --git-dir)/hooks"

# Create hooks directory if it doesn't exist
mkdir -p "$HOOKS_DIR"

echo -e "${YELLOW}Installing Git hooks...${NC}"

# Note: Actual hook implementation will be done in subsequent tasks
# This script sets up the foundation for hook installation

echo -e "${GREEN}Git hooks setup completed!${NC}"
echo ""
echo "Next steps:"
echo "1. Implement commit-msg hook for conventional commits (Task 2.1)"
echo "2. Implement pre-push hook for branch protection (Task 2.2)"
echo "3. Create topic branch management script (Task 3)"
echo ""
echo "The foundational repository structure is now ready for development."