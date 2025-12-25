# Top-level Makefile.am for FMI/OS Stage 1

# Subdirectories to build in order
SUBDIRS = include lib arch/x86_64 kernel tests

# Architecture symlink management for Stage 1 (x86_64 only)
all-local:
	@echo "Setting up architecture symlinks for x86_64..."
	@test -d include || mkdir -p include
	@test -d lib || mkdir -p lib
	@test -L include/arch || $(LN_S) ../arch/x86_64/include include/arch
	@test -L lib/arch || $(LN_S) ../arch/x86_64/lib lib/arch

clean-local:
	@echo "Cleaning architecture symlinks..."
	rm -f include/arch lib/arch

distclean-local: clean-local

# QEMU testing support for PVH ELF format
if HAVE_QEMU
qemu-test: kernel/fmi-kernel
	@echo "Running QEMU test with PVH ELF kernel..."
	timeout 30s qemu-system-x86_64 -kernel kernel/fmi-kernel -serial mon:stdio -nographic || \
	(echo "QEMU test timed out"; exit 1)
else
qemu-test:
	@echo "QEMU not available - skipping test"
endif

# Make qemu-test available as a target
.PHONY: qemu-test

# Distribution files
EXTRA_DIST = \
	README.md \
	LICENSE.txt \
	scripts/ \
	.kiro/ \
	.clang-format \
	.editorconfig \
	.gitignore \
	.pre-commit-config.yaml

# Configure-generated files that should be cleaned
CLEANFILES = include/arch lib/arch

# Ensure proper build order
kernel: include lib arch/x86_64
tests: kernel
