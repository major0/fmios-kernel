#!/bin/bash
# Protect main/master branch from direct commits
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Get current branch name
BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD)

# Check if we're on a protected branch
case "${BRANCH_NAME}" in
    main|master)
        echo "ERROR: Direct commits to '${BRANCH_NAME}' branch are not allowed"
        echo ""
        echo "To contribute changes:"
        echo "1. Create a topic branch: git checkout -b topic/your-feature"
        echo "2. Make your changes and commit them"
        echo "3. Push your topic branch: git push origin topic/your-feature"
        echo "4. Create a pull request for review"
        echo ""
        echo "If you need to switch to a topic branch now:"
        echo "  git stash                              # Save your changes"
        echo "  git checkout -b topic/your-feature     # Create topic branch"
        echo "  git stash pop                          # Restore your changes"
        exit 1
        ;;
esac

exit 0