# Code Style Guidelines

This document establishes the code style guidelines for all FMI/OS development. These guidelines are enforced through EditorConfig and pre-commit hooks to ensure consistency across the entire codebase.

## Core Principles

1. **Consistency** - All code follows the same formatting and style patterns
2. **Readability** - Code style enhances readability and maintainability
3. **Tool Enforcement** - Style rules are enforced automatically through tooling
4. **Language-Specific** - Different file types have appropriate style rules

## Universal File Rules

All files in the FMI/OS project MUST follow these universal rules:

- **Character Encoding**: UTF-8 encoding for all files
- **Line Endings**: Unix-style line endings (LF) for all files
- **Final Newline**: All files must end with a newline character
- **Trailing Whitespace**: No trailing whitespace allowed (except markdown files)

## Language-Specific Style Rules

### C Source and Header Files (`*.c`, `*.h`)

C code follows traditional kernel coding style:

- **Indentation**: Tabs only, with tab size of 8 spaces
- **Line Length**: Maximum 100 characters per line
- **Brace Style**: K&R style bracing
- **Function Declarations**: One parameter per line for long parameter lists

```c
// CORRECT: Tab indentation, K&R braces
int function_name(int param1, int param2)
{
	if (condition) {
		do_something();
	}
	return 0;
}

// INCORRECT: Space indentation
int function_name(int param1, int param2)
{
    if (condition) {
        do_something();
    }
    return 0;
}
```

### Assembly Files (`*.s`, `*.S`)

Assembly code follows the same indentation rules as C:

- **Indentation**: Tabs only, with tab size of 8 spaces
- **Line Length**: Maximum 100 characters per line
- **Comments**: Use `#` for comments, align with code

```assembly
# CORRECT: Tab indentation
.global _start
_start:
	mov	$1, %rax
	mov	$message, %rsi
	syscall

# INCORRECT: Space indentation
.global _start
_start:
    mov    $1, %rax
    mov    $message, %rsi
    syscall
```

### Shell Scripts (`*.sh`, `*.bash`)

Shell scripts follow POSIX-compliant style with tab indentation:

- **Indentation**: Tabs only, with tab size of 8 spaces
- **Line Length**: Maximum 100 characters per line
- **POSIX Compliance**: Use POSIX shell syntax, not Bash extensions
- **Quoting**: Always quote variables: `"${variable}"`

```bash
#!/bin/bash
# CORRECT: Tab indentation, proper quoting
POSIXLY_CORRECT='No bashing shell'

check_file() {
	local file="$1"
	
	if test -f "${file}"; then
		echo "File exists: ${file}"
		return 0
	fi
	return 1
}

# INCORRECT: Space indentation
check_file() {
    local file="$1"
    
    if test -f "${file}"; then
        echo "File exists: ${file}"
        return 0
    fi
    return 1
}
```

### Makefiles and Build Files

Build system files use tab indentation as required by Make:

- **Indentation**: Tabs only, with tab size of 8 spaces
- **Recipe Lines**: Must use tabs (Make requirement)
- **Variable Assignments**: Align for readability

```makefile
# CORRECT: Tab indentation
CFLAGS		= -Wall -Wextra -Werror
KERNEL_OBJS	= main.o memory.o process.o

kernel: $(KERNEL_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# INCORRECT: Space indentation (will break Make)
CFLAGS      = -Wall -Wextra -Werror
KERNEL_OBJS = main.o memory.o process.o

kernel: $(KERNEL_OBJS)
    $(CC) $(CFLAGS) -o $@ $^
```

### Markdown Files (`*.md`)

Markdown files use space indentation for better readability:

- **Indentation**: 2 spaces for nested lists and code blocks
- **Line Length**: No maximum line length (long lines allowed)
- **Trailing Whitespace**: Preserved (required for markdown formatting)

```markdown
<!-- CORRECT: 2-space indentation -->
1. First item
  - Nested item
  - Another nested item
    - Deeply nested item

2. Second item
  ```bash
  # Code block with 2-space indentation
  ./configure --target=x86_64-fmios
  make
  ```

<!-- INCORRECT: Tab or 4-space indentation -->
1. First item
    - Nested item (4 spaces - wrong)
	- Another nested item (tab - wrong)
```

### YAML Files (`*.yml`, `*.yaml`)

YAML files require space indentation:

- **Indentation**: 2 spaces only (YAML requirement)
- **No Tabs**: Tabs are forbidden in YAML
- **Alignment**: Align values consistently

```yaml
# CORRECT: 2-space indentation
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer

# INCORRECT: Tab indentation (breaks YAML)
repos:
	- repo: https://github.com/pre-commit/pre-commit-hooks
	  rev: v4.5.0
	  hooks:
		- id: trailing-whitespace
```

### JSON Files (`*.json`)

JSON files use space indentation for consistency:

- **Indentation**: 2 spaces for nested objects and arrays
- **No Trailing Commas**: JSON standard compliance
- **Quote Style**: Double quotes only

```json
{
  "default": false,
  "MD001": false,
  "MD013": {
    "line_length": 150
  },
  "rules": [
    "rule1",
    "rule2"
  ]
}
```

### Git Configuration Files

Git configuration files use space indentation:

- **Indentation**: 2 spaces for consistency with other configuration files
- **File Types**: `.gitignore`, `.gitattributes`, `.gitmessage`

```gitignore
# CORRECT: Simple format, no indentation needed
*.o
*.so
build/
.DS_Store

# Configuration sections (if applicable)
[core]
  autocrlf = false
  eol = lf
```

### Linker Scripts (`*.ld`)

Linker scripts follow assembly-style formatting:

- **Indentation**: Tabs only, with tab size of 8 spaces
- **Line Length**: Maximum 100 characters per line
- **Section Alignment**: Align section definitions

```ld
/* CORRECT: Tab indentation */
SECTIONS {
	.text : {
		*(.text)
		*(.text.*)
	} > RAM

	.data : {
		*(.data)
		*(.data.*)
	} > RAM
}

/* INCORRECT: Space indentation */
SECTIONS {
    .text : {
        *(.text)
        *(.text.*)
    } > RAM
}
```

## Tool Integration

### EditorConfig

The `.editorconfig` file enforces these rules automatically in most editors:

- Install EditorConfig plugin for your editor
- Rules are applied automatically when opening files
- No manual configuration needed

### Pre-commit Hooks

Pre-commit hooks validate style compliance:

- **editorconfig-checker**: Validates all EditorConfig rules
- **trailing-whitespace**: Removes trailing whitespace
- **end-of-file-fixer**: Ensures files end with newlines
- **mixed-line-ending**: Enforces LF line endings

### Clang-Format (C/C++ Code)

C code formatting is handled by clang-format:

- Configuration in `.clang-format` file
- Integrates with EditorConfig rules
- Enforces consistent C code style

## Common Violations and Fixes

### Mixed Indentation

```c
// WRONG: Mixing tabs and spaces
int function(void)
{
	int x = 1;		// Tab indentation
    int y = 2;		// Space indentation - WRONG
	return x + y;
}

// CORRECT: Consistent tab indentation
int function(void)
{
	int x = 1;		// Tab indentation
	int y = 2;		// Tab indentation
	return x + y;
}
```

### Incorrect Line Endings

```bash
# WRONG: Windows line endings (CRLF)
echo "Hello World"\r\n

# CORRECT: Unix line endings (LF)
echo "Hello World"\n
```

### Missing Final Newline

```c
// WRONG: File ends without newline
int main(void) {
    return 0;
}[EOF - no newline]

// CORRECT: File ends with newline
int main(void) {
    return 0;
}
[newline][EOF]
```

### Trailing Whitespace

```c
// WRONG: Trailing spaces after semicolon
int x = 1;   [trailing spaces]
int y = 2;

// CORRECT: No trailing whitespace
int x = 1;
int y = 2;
```

## Editor Configuration

### VS Code

Install the EditorConfig extension and add to `settings.json`:

```json
{
  "editor.insertSpaces": false,
  "editor.detectIndentation": false,
  "files.insertFinalNewline": true,
  "files.trimTrailingWhitespace": true
}
```

### Vim

Add to `.vimrc`:

```vim
set noexpandtab
set tabstop=8
set shiftwidth=8
set autoindent
```

### Emacs

Add to `.emacs`:

```elisp
(setq-default indent-tabs-mode t)
(setq-default tab-width 8)
(setq c-basic-offset 8)
```

## Compliance Verification

All code MUST pass these checks:

1. **EditorConfig Validation**: `editorconfig-checker` passes
2. **Pre-commit Hooks**: All formatting hooks pass
3. **Clang-Format**: C code follows `.clang-format` rules
4. **Manual Review**: Code follows language-specific guidelines

## Rationale

### Why Tabs for C/Assembly/Shell?

- **Tradition**: Follows established kernel coding conventions
- **Flexibility**: Developers can set their preferred tab width
- **Efficiency**: Tabs use less storage than multiple spaces
- **Tool Support**: Better support in traditional Unix tools

### Why Spaces for Configuration Files?

- **Standard Compliance**: YAML and JSON require spaces
- **Consistency**: Configuration files benefit from uniform indentation
- **Readability**: Spaces provide consistent visual alignment

### Why 100-Character Line Limit?

- **Modern Displays**: Accommodates modern wide screens
- **Side-by-Side**: Allows two files side-by-side on wide monitors
- **Readability**: Prevents excessively long lines while allowing reasonable length

This code style framework ensures consistent, readable, and maintainable code across all FMI/OS development while respecting the conventions and requirements of different file types and languages.