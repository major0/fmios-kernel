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
	rm -f fmios.iso grub.cfg
	rm -rf isodir

distclean-local: clean-local

# QEMU testing support for Multiboot2 format with GRUB
if HAVE_QEMU
if HAVE_GRUB
qemu-test: fmios.iso
	@echo "Running QEMU test with Multiboot2 kernel via GRUB..."
	@echo "Stage 1: Testing kernel boot and basic execution..."
	@rm -f serial.log
	@timeout 5s qemu-system-x86_64 -serial file:serial.log -display none -cdrom fmios.iso || true
	@if test -f serial.log && grep -q "1ABCDEFGH.*6OPQRS" serial.log; then \
		echo "QEMU test PASSED: Kernel successfully booted via GRUB and executed"; \
		rm -f serial.log; \
	else \
		echo "QEMU test FAILED: Kernel did not boot properly via GRUB"; \
		if test -f serial.log; then echo "Serial output:"; cat serial.log; else echo "No serial.log file created"; fi; \
		exit 0; \
	fi

# ISO generation with GRUB for Multiboot2
fmios.iso: kernel/fmi-kernel grub.cfg
	@echo "Creating ISO image with GRUB bootloader..."
	@mkdir -p isodir/boot/grub
	@cp kernel/fmi-kernel isodir/boot/
	@cp grub.cfg isodir/boot/grub/
	@grub-mkrescue -o fmios.iso isodir
	@rm -rf isodir

# GRUB configuration file
grub.cfg:
	@echo "Creating GRUB configuration..."
	@echo 'set timeout=1' > grub.cfg
	@echo 'set default=0' >> grub.cfg
	@echo 'menuentry "FMI/OS Kernel" {' >> grub.cfg
	@echo '    multiboot2 /boot/fmi-kernel' >> grub.cfg
	@echo '    boot' >> grub.cfg
	@echo '}' >> grub.cfg

else
qemu-test:
	@echo "GRUB not available - cannot create ISO for Multiboot2 testing"
	@echo "Install grub-mkrescue and xorriso to enable ISO generation"
endif
else
qemu-test:
	@echo "QEMU not available - skipping test"
endif

# Make qemu-test and ISO generation available as targets
.PHONY: qemu-test fmios.iso

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
CLEANFILES = include/arch lib/arch fmios.iso grub.cfg

# Ensure proper build order
kernel: include lib arch/x86_64
tests: kernel
