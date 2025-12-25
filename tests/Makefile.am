# Tests Makefile.am for Stage 1

# Test programs
check_PROGRAMS = test_build test_build_properties

# Basic build test
test_build_SOURCES = test_build.c
test_build_CFLAGS = $(TEST_CFLAGS) -I$(top_srcdir)/include
test_build_LDADD =

# Property-based build test
test_build_properties_SOURCES = test_build_properties.c
test_build_properties_CFLAGS = $(TEST_CFLAGS) -I$(top_srcdir)/include
test_build_properties_LDADD =

# Test execution
TESTS = test_build test_build_properties

# QEMU testing (if available)
if HAVE_QEMU
check-local: qemu-test-stage1

qemu-test-stage1:
	@echo "Running Stage 1 QEMU test..."
	cd $(top_builddir) && $(MAKE) qemu-test
else
check-local:
	@echo "QEMU not available - skipping QEMU tests"
endif

.PHONY: check-local qemu-test-stage1
