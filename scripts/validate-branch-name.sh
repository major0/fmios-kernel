#!/bin/bash
# Validate branch naming convention
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Get current branch name
BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD)

# Skip validation for main/master branches
case "${BRANCH_NAME}" in
    main|master)
        exit 0
        ;;
esac

# Valid branch types
VALID_TYPES="topic|fix|docs|refactor|test|chore"

# Check branch naming convention: type/description or type/description-with-task
if ! echo "${BRANCH_NAME}" | grep -qE "^(${VALID_TYPES})/[a-zA-Z0-9_-]+$"; then
    echo "ERROR: Branch name '${BRANCH_NAME}' doesn't follow naming convention"
    echo ""
    echo "Required format: <type>/<description>"
    echo "Valid types: topic, fix, docs, refactor, test, chore"
    echo ""
    echo "Examples:"
    echo "  topic/memory-allocator"
    echo "  fix/console-buffer-overflow"
    echo "  docs/build-instructions"
    echo "  refactor/process-creation"
    echo "  test/slab-allocator"
    echo "  chore/update-dependencies"
    echo ""
    echo "To fix this:"
    echo "  git checkout -b <type>/<description>"
    echo "  git branch -D ${BRANCH_NAME}  # Delete old branch"
    exit 1
fi

echo "✓ Branch name validated: ${BRANCH_NAME}"
exit 0