#!/bin/bash
# QEMU test script for Stage 1 Multiboot2 kernel with GRUB

POSIXLY_CORRECT='No bashing shell'

echo "Running QEMU test with Multiboot2 kernel via GRUB..."
echo "Stage 1: Testing kernel boot and basic execution..."

# Check if ISO exists
if ! test -f fmios.iso; then
	echo "ERROR: fmios.iso not found - run 'make fmios.iso' first"
	exit 1
fi

# Run QEMU with ISO and capture output using character device with logging
rm -f serial.log
# timeout 10s qemu-system-x86_64 -chardev stdio,id=char0,mux=on,logfile=serial.log,signal=off \
#   -serial chardev:char0 -mon chardev=char0 -nographic -cdrom fmios.iso || true
timeout 5s qemu-system-x86_64 -serial file:serial.log -display none -cdrom fmios.iso || true

# Check if serial.log exists and contains our debug sequence
if test -f serial.log; then
	OUTPUT=$(cat serial.log)
	echo "Debug: Captured output:"
	echo "$OUTPUT"
	echo "Debug: End of output"

	# Check if we got the expected debug sequence for Multiboot2
	# 1ABCDEFGH = 32-bit setup and CPUID detection
	# 6OPQRS = 64-bit mode and kernel initialization
	if echo "$OUTPUT" | grep -q "1ABCDEFGH.*6OPQRS"; then
		echo "QEMU test PASSED: Kernel successfully booted via GRUB and executed"
		rm -f serial.log
		exit 0
	else
		echo "QEMU test FAILED: Kernel did not boot properly via GRUB"
		echo "Output was:"
		echo "$OUTPUT"
		# Keep serial.log for debugging
		exit 1
	fi
else
	echo "QEMU test FAILED: No serial output captured"
	exit 1
fi
