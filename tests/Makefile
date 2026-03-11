# tests/Makefile
#
# Build and run the host-side unit tests for Thermo-meter-msp432-I2C.
#
# Usage:
#   make           - build all test executables
#   make test      - build and run all tests (non-zero exit on any failure)
#   make clean     - remove generated files
#
# Compiler
# --------
# C11 is used for broad standards compliance.
#
# Include-path strategy
# ----------------------
# -I.           makes "msp.h" resolve to tests/msp.h (our mock), NOT the
#               TI SDK header, because the tests/ directory is searched
#               first.
# -I..          allows source files to include sibling headers such as
#               I2C.h, libLCD.h, and diag.h.

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wno-unused-parameter \
           -I. -I.. -DUNIT_TESTING

SRCS_MOCK   := msp_mock.c

# test_I2C: tests the I2C driver (no delay_ms, weak mock_tick is fine)
SRCS_I2C    := $(SRCS_MOCK) test_I2C.c ../I2C.c

# test_libLCD: tests the LCD driver; provides strong mock_tick with
#              nibble capture + TA0_ISR_Handler advancement
SRCS_LCD    := $(SRCS_MOCK) test_libLCD.c ../libLCD.c

# test_diag_constants: only includes diag.h, no hardware needed
SRCS_DIAG   := test_diag_constants.c

TARGETS := test_I2C test_libLCD test_diag_constants

.PHONY: all test clean

all: $(TARGETS)

test_I2C: $(SRCS_I2C)
	$(CC) $(CFLAGS) -o $@ $^

test_libLCD: $(SRCS_LCD)
	$(CC) $(CFLAGS) -o $@ $^

test_diag_constants: $(SRCS_DIAG)
	$(CC) $(CFLAGS) -o $@ $^

# Run all tests and propagate failure
test: all
	@echo ""; echo "--- test_I2C ---";           ./test_I2C
	@echo ""; echo "--- test_libLCD ---";        ./test_libLCD
	@echo ""; echo "--- test_diag_constants ---"; ./test_diag_constants
	@echo ""; echo "All test suites passed."

clean:
	rm -f $(TARGETS)
