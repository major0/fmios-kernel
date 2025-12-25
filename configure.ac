AC_INIT([fmios], [0.1.0], [support@fmios.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Require autoconf 2.69 or later
AC_PREREQ([2.69])

# Check for programs
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AM_PROG_AS
AC_PROG_LN_S

# Disable default CFLAGS to prevent PIC mode issues
: ${CFLAGS=""}
: ${CXXFLAGS=""}

# Stage 1: Only support x86_64 architecture
AC_ARG_WITH([target],
	[AS_HELP_STRING([--with-target=ARCH], [Target architecture (x86_64 only for Stage 1)])],
	[target_arch=$withval],
	[target_arch=x86_64])

# Validate target architecture - Stage 1 only supports x86_64
case "$target_arch" in
	x86_64)
		AC_MSG_NOTICE([Building for x86_64 architecture])
		;;
	*)
		AC_MSG_ERROR([Stage 1 only supports x86_64 architecture. Use --with-target=x86_64])
		;;
esac

# Set architecture-specific compiler flags for x86_64 (Stage 1: simplified)
ARCH_CFLAGS="-mno-red-zone -mno-mmx -mno-sse -mno-sse2"
ARCH_LDFLAGS="-z max-page-size=0x1000"

# Kernel build flags - simplified for Stage 1
KERNEL_CFLAGS="-std=c11 -Wall -Wextra -Werror -ffreestanding"
KERNEL_CFLAGS="$KERNEL_CFLAGS -nostdlib -fno-builtin"
KERNEL_CFLAGS="$KERNEL_CFLAGS $ARCH_CFLAGS"

# Check for compiler support for security mitigations (disabled for Stage 1)
# AC_MSG_CHECKING([for compiler support of retpoline])
# old_CFLAGS="$CFLAGS"
# CFLAGS="$CFLAGS -mretpoline"
# AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
#	[AC_MSG_RESULT([yes]); KERNEL_CFLAGS="$KERNEL_CFLAGS -mretpoline"],
#	[AC_MSG_RESULT([no])])
# CFLAGS="$old_CFLAGS"

# Disable CET for Stage 1 due to kernel code model conflicts
# AC_MSG_CHECKING([for compiler support of CET])
# old_CFLAGS="$CFLAGS"
# CFLAGS="$CFLAGS -fcf-protection=full"
# AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])],
#	[AC_MSG_RESULT([yes]); KERNEL_CFLAGS="$KERNEL_CFLAGS -fcf-protection=full"],
#	[AC_MSG_RESULT([no])])
# CFLAGS="$old_CFLAGS"

# Add basic security flags that work with kernel code model
# KERNEL_CFLAGS="$KERNEL_CFLAGS -fstack-protector-strong"
# KERNEL_CFLAGS="$KERNEL_CFLAGS -Warray-bounds -Wstringop-overflow -fstack-clash-protection"
# KERNEL_CFLAGS="$KERNEL_CFLAGS -Wformat-overflow -Wformat-truncation"

# Kernel linker flags
KERNEL_LDFLAGS="-nostdlib -static $ARCH_LDFLAGS"

# Test build flags (hosted environment)
TEST_CFLAGS="-std=c11 -Wall -Wextra -Werror -O0 -g -D_POSIX_C_SOURCE=200809L"

# Substitute variables for Makefiles
AC_SUBST([target_arch])
AC_SUBST([ARCH_CFLAGS])
AC_SUBST([ARCH_LDFLAGS])
AC_SUBST([KERNEL_CFLAGS])
AC_SUBST([KERNEL_LDFLAGS])
AC_SUBST([TEST_CFLAGS])

# Check for required tools
AC_CHECK_PROG([QEMU], [qemu-system-x86_64], [qemu-system-x86_64])
if test "x$QEMU" = "x"; then
	AC_MSG_WARN([qemu-system-x86_64 not found - QEMU testing will not be available])
fi

# Check for GRUB tools for Multiboot2 ISO generation
AC_CHECK_PROG([GRUB_MKRESCUE], [grub-mkrescue], [grub-mkrescue])
if test "x$GRUB_MKRESCUE" = "x"; then
	AC_MSG_WARN([grub-mkrescue not found - ISO generation will not be available])
fi

AC_CHECK_PROG([XORRISO], [xorriso], [xorriso])
if test "x$XORRISO" = "x"; then
	AC_MSG_WARN([xorriso not found - ISO generation may not work properly])
fi

# Conditional for QEMU and GRUB availability
AM_CONDITIONAL([HAVE_QEMU], [test "x$QEMU" != "x"])
AM_CONDITIONAL([HAVE_GRUB], [test "x$GRUB_MKRESCUE" != "x"])

# Output files
AC_CONFIG_FILES([
	Makefile
	kernel/Makefile
	arch/x86_64/Makefile
	arch/x86_64/include/Makefile
	arch/x86_64/lib/Makefile
	include/Makefile
	lib/Makefile
	lib/c/Makefile
	tests/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([
FMI/OS Stage 1 Build Configuration:
	Target Architecture: $target_arch
	Kernel CFLAGS: $KERNEL_CFLAGS
	Kernel LDFLAGS: $KERNEL_LDFLAGS
	QEMU Available: ${QEMU:-no}
	GRUB Available: ${GRUB_MKRESCUE:-no}
	XORRISO Available: ${XORRISO:-no}
])
