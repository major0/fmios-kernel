#!/bin/bash
# Install pre-commit hooks for FMI/OS development
# POSIX compliance
POSIXLY_CORRECT='No bashing shell'

echo "Installing pre-commit hooks for FMI/OS..."

# Check if pre-commit is installed
if ! command -v pre-commit >/dev/null 2>&1; then
    echo "ERROR: pre-commit is not installed"
    echo ""
    echo "Please install pre-commit first:"
    echo "  pip install pre-commit"
    echo "  # or"
    echo "  brew install pre-commit"
    echo "  # or"
    echo "  apt-get install pre-commit"
    exit 1
fi

# Install the hooks
echo "Installing pre-commit hooks..."
if pre-commit install --install-hooks; then
    echo "✓ Pre-commit hooks installed successfully"
else
    echo "✗ Failed to install pre-commit hooks"
    exit 1
fi

# Install commit-msg hook
echo "Installing commit-msg hook..."
if pre-commit install --hook-type commit-msg; then
    echo "✓ Commit-msg hook installed successfully"
else
    echo "✗ Failed to install commit-msg hook"
    exit 1
fi

# Install pre-push hook
echo "Installing pre-push hook..."
if pre-commit install --hook-type pre-push; then
    echo "✓ Pre-push hook installed successfully"
else
    echo "✗ Failed to install pre-push hook"
    exit 1
fi

echo ""
echo "Pre-commit hooks installation complete!"
echo ""
echo "The following validations will now run automatically:"
echo "  • EditorConfig compliance (indentation, line endings, etc.)"
echo "  • Code formatting (clang-format for C code)"
echo "  • Shell script linting (shellcheck)"
echo "  • Markdown formatting (markdownlint)"
echo "  • GPL v2 license header validation"
echo "  • Conventional commit message validation"
echo "  • Task reference validation"
echo "  • Branch name validation"
echo "  • Protection against direct commits to main/master"
echo "  • Standard file checks (trailing whitespace, merge conflicts, large files, etc.)"
echo ""
echo "To run checks manually: pre-commit run --all-files"