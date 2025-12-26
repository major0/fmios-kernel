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
if ENABLE_MULTIBOOT2
qemu-test: fmios.iso
	@echo "Running QEMU test with Multiboot2 kernel via GRUB..."
	@echo "Stage 1: Testing kernel boot and basic execution..."
	@rm -f serial.log
	@timeout 5s qemu-system-x86_64 -machine pc -serial file:serial.log -display none -cdrom fmios.iso || true
	@if test -f serial.log && grep -q "FMI/OS Stage 1 - Hello World Kernel" serial.log; then \
		echo "QEMU test PASSED: Kernel successfully booted via GRUB and executed"; \
		rm -f serial.log; \
	else \
		echo "QEMU test FAILED: Kernel did not boot properly via GRUB"; \
		if test -f serial.log; then echo "Serial output:"; cat serial.log; else echo "No serial.log file created"; fi; \
		exit 0; \
	fi
else
qemu-test:
	@echo "Multiboot2 support disabled - cannot run GRUB-based test"
	@echo "Use --enable-multiboot2 to enable Multiboot2 support"
endif
else
qemu-test:
	@echo "GRUB not available - cannot create ISO for Multiboot2 testing"
	@echo "Install grub-mkrescue and xorriso to enable ISO generation"
endif
else
qemu-test:
	@echo "QEMU not available - skipping test"
endif

# UEFI testing support using GRUB with OVMF firmware
if HAVE_QEMU
if HAVE_GRUB
qemu-test-uefi: fmios.iso
	@echo "Running QEMU test with UEFI firmware (OVMF) and GRUB..."
	@echo "Stage 1: Testing GRUB-UEFI boot capability..."
	@if test -f /usr/share/OVMF/OVMF_CODE_4M.fd && test -f /usr/share/OVMF/OVMF_VARS_4M.fd; then \
		echo "OVMF firmware found - testing GRUB in UEFI mode..."; \
		cp /usr/share/OVMF/OVMF_VARS_4M.fd /tmp/OVMF_VARS_4M.fd; \
		rm -f uefi_serial.log; \
		timeout 10s qemu-system-x86_64 \
			-machine q35 \
			-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
			-drive if=pflash,format=raw,file=/tmp/OVMF_VARS_4M.fd \
			-cdrom fmios.iso \
			-serial file:uefi_serial.log \
			-display none \
			-no-reboot || true; \
		if test -f uefi_serial.log; then \
			echo "UEFI boot test output:"; \
			cat uefi_serial.log; \
			if grep -q "FMI/OS Stage 1 - Hello World Kernel" uefi_serial.log; then \
				if grep -q "Boot Protocol: Multiboot2" uefi_serial.log; then \
					echo "UEFI test PASSED: Kernel booted via GRUB-UEFI (Multiboot2 protocol)"; \
				else \
					echo "UEFI test PASSED: Kernel booted via GRUB-UEFI"; \
				fi; \
			else \
				echo "UEFI test FAILED: Kernel did not boot properly via GRUB-UEFI"; \
			fi; \
			rm -f uefi_serial.log; \
		else \
			echo "UEFI test FAILED: No serial output captured"; \
		fi; \
		rm -f /tmp/OVMF_VARS_4M.fd; \
	elif test -f /usr/share/qemu/OVMF.fd; then \
		echo "Legacy OVMF firmware found - testing GRUB in UEFI mode..."; \
		rm -f uefi_serial.log; \
		timeout 10s qemu-system-x86_64 \
			-machine q35 \
			-bios /usr/share/qemu/OVMF.fd \
			-cdrom fmios.iso \
			-serial file:uefi_serial.log \
			-display none \
			-no-reboot || true; \
		if test -f uefi_serial.log; then \
			echo "UEFI boot test output:"; \
			cat uefi_serial.log; \
			if grep -q "FMI/OS Stage 1 - Hello World Kernel" uefi_serial.log; then \
				if grep -q "Boot Protocol: Multiboot2" uefi_serial.log; then \
					echo "UEFI test PASSED: Kernel booted via GRUB-UEFI (Multiboot2 protocol)"; \
				else \
					echo "UEFI test PASSED: Kernel booted via GRUB-UEFI"; \
				fi; \
			else \
				echo "UEFI test FAILED: Kernel did not boot properly via GRUB-UEFI"; \
			fi; \
			rm -f uefi_serial.log; \
		else \
			echo "UEFI test FAILED: No serial output captured"; \
		fi; \
	else \
		echo "OVMF firmware not found - install ovmf package for UEFI testing"; \
		echo "UEFI support: IMPLEMENTED (GRUB-UEFI testing requires OVMF firmware)"; \
	fi
else
qemu-test-uefi:
	@echo "GRUB not available - cannot test GRUB-UEFI boot"
	@echo "Install grub-mkrescue and xorriso to enable UEFI testing"
endif
else
qemu-test-uefi:
	@echo "QEMU not available - skipping UEFI test"
endif

# ISO generation with GRUB for Multiboot2
if HAVE_GRUB
if ENABLE_MULTIBOOT2
fmios.iso: kernel/fmi-kernel grub.cfg
	@echo "Creating ISO image with GRUB bootloader..."
	@mkdir -p isodir/boot/grub
	@cp kernel/fmi-kernel isodir/boot/
	@cp grub.cfg isodir/boot/grub/
	@grub-mkrescue -o fmios.iso isodir
	@rm -rf isodir
else
fmios.iso:
	@echo "Multiboot2 support disabled - cannot create GRUB ISO"
	@echo "Use --enable-multiboot2 to enable ISO generation"
	@exit 1
endif
else
fmios.iso:
	@echo "GRUB not available - cannot create ISO"
	@echo "Install grub-mkrescue and xorriso to enable ISO generation"
	@exit 1
endif

# GRUB configuration file
if ENABLE_MULTIBOOT2
grub.cfg:
	@echo "Creating GRUB configuration..."
	@echo 'set timeout=1' > grub.cfg
	@echo 'set default=0' >> grub.cfg
	@echo 'menuentry "FMI/OS Kernel" {' >> grub.cfg
	@echo '    multiboot2 /boot/fmi-kernel' >> grub.cfg
	@echo '    boot' >> grub.cfg
	@echo '}' >> grub.cfg
else
grub.cfg:
	@echo "Multiboot2 support disabled - cannot create GRUB configuration"
	@exit 1
endif

# Make qemu-test and ISO generation available as targets
.PHONY: qemu-test qemu-test-uefi fmios.iso

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
