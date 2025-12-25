#!/bin/bash
# ISO generation script for FMI/OS with GRUB bootloader

POSIXLY_CORRECT='No bashing shell'

# Check if required tools are available
if ! command -v grub-mkrescue >/dev/null 2>&1; then
	echo "ERROR: grub-mkrescue not found"
	echo "Install GRUB tools: sudo apt-get install grub-pc-bin grub-common"
	exit 1
fi

if ! command -v xorriso >/dev/null 2>&1; then
	echo "ERROR: xorriso not found"
	echo "Install xorriso: sudo apt-get install xorriso"
	exit 1
fi

# Check if kernel exists
if ! test -f kernel/fmi-kernel; then
	echo "ERROR: kernel/fmi-kernel not found - run 'make' first"
	exit 1
fi

echo "Creating ISO image with GRUB bootloader..."

# Create directory structure
mkdir -p isodir/boot/grub

# Copy kernel
cp kernel/fmi-kernel isodir/boot/

# Create GRUB configuration
cat > isodir/boot/grub/grub.cfg << 'EOF'
set timeout=1
set default=0
menuentry "FMI/OS Kernel" {
	multiboot2 /boot/fmi-kernel
	boot
}
EOF

# Generate ISO
grub-mkrescue -o fmios.iso isodir

# Clean up
rm -rf isodir

echo "ISO image created: fmios.iso"
echo "Test with: qemu-system-x86_64 -serial file:serial.log -display none -cdrom fmios.iso"
