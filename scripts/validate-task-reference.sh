#!/bin/bash
# Validate task references in commit messages
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

# Read commit message from file (passed as argument by pre-commit)
COMMIT_MSG_FILE="$1"

if test -z "${COMMIT_MSG_FILE}" || ! test -f "${COMMIT_MSG_FILE}"; then
    echo "ERROR: Commit message file not provided or doesn't exist"
    exit 1
fi

# Read the commit message
COMMIT_MSG=$(cat "${COMMIT_MSG_FILE}")

# Skip merge commits and revert commits
if echo "${COMMIT_MSG}" | grep -q "^Merge\|^Revert"; then
    exit 0
fi

# Check if commit message contains task reference
if ! echo "${COMMIT_MSG}" | grep -q "^Task: [a-zA-Z0-9_-]\+#[0-9]\+\(\.[0-9]\+\)*"; then
    echo "ERROR: Commit message must include a task reference"
    echo ""
    echo "Format: Task: spec-name#task-number"
    echo "Examples:"
    echo "  Task: memory#2.1"
    echo "  Task: git-workflow#6"
    echo "  Task: console#3.2"
    echo ""
    echo "Your commit message:"
    echo "${COMMIT_MSG}"
    exit 1
fi

# Extract task reference
TASK_REF=$(echo "${COMMIT_MSG}" | grep "^Task: " | sed 's/^Task: //')
SPEC_NAME=$(echo "${TASK_REF}" | cut -d'#' -f1)
TASK_NUMBER=$(echo "${TASK_REF}" | cut -d'#' -f2)

# Check if spec exists
SPEC_DIR=".kiro/specs/${SPEC_NAME}"
if ! test -d "${SPEC_DIR}"; then
    echo "ERROR: Spec '${SPEC_NAME}' not found in ${SPEC_DIR}"
    echo "Available specs:"
    if test -d ".kiro/specs"; then
        ls -1 .kiro/specs/ | sed 's/^/  /'
    fi
    exit 1
fi

# Check if tasks.md exists in spec
TASKS_FILE="${SPEC_DIR}/tasks.md"
if ! test -f "${TASKS_FILE}"; then
    echo "ERROR: tasks.md not found in spec '${SPEC_NAME}'"
    exit 1
fi

# Check if task number exists in tasks.md
if ! grep -q "^- \[.\] ${TASK_NUMBER}" "${TASKS_FILE}"; then
    echo "ERROR: Task '${TASK_NUMBER}' not found in ${TASKS_FILE}"
    echo ""
    echo "Available tasks:"
    grep "^- \[.\] [0-9]" "${TASKS_FILE}" | sed 's/^/  /'
    exit 1
fi

echo "✓ Task reference validated: ${TASK_REF}"
exit 0