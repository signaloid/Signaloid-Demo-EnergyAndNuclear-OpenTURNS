#
#	Top-level Makefile for native Monte Carlo builds.
#
#	Build the native Monte Carlo executable with:
#
#		make local-build
#
#	This builds the demo against the host toolchain and the UxHw
#	compatibility shim in `submodules/compat`, rather than against the
#	Signaloid cloud compiler.
#
#	Copyright (c) 2026, Signaloid.
#
#	Targets:
#	  local-build - Build for native execution with the UxHw compat layer
#	  local-run   - Build and run the Monte Carlo engine for every output
#	  clean       - Remove build artifacts
#

#
#	Include `config.mk` first, so that the variables it sets cannot
#	overwrite the ones this Makefile defines below. `config.mk` is the
#	source list shared with the Signaloid cloud build.
#
include src/config.mk

CC		= gcc

#
#	Use `gnu11` rather than `c11`: the UxHw compatibility shim calls the
#	POSIX `random()` and `srandom()`, which a strict ISO C dialect does
#	not declare. Under `c11` they are implicitly declared as returning
#	`int`, while they in fact return `long`.
#
CFLAGS		= -std=gnu11 -Wall -Wextra -O2 -Isrc/
LDFLAGS		=
LIBS		= -lgsl -lgslcblas -lm

#
#	Platform-specific paths (macOS with MacPorts or Homebrew).
#
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    ifneq ($(wildcard /opt/local/include/gsl),)
        CFLAGS  += -I/opt/local/include
        LDFLAGS += -L/opt/local/lib
    else ifneq ($(wildcard /opt/homebrew/include/gsl),)
        CFLAGS  += -I/opt/homebrew/include
        LDFLAGS += -L/opt/homebrew/lib
    else ifneq ($(wildcard /usr/local/include/gsl),)
        CFLAGS  += -I/usr/local/include
        LDFLAGS += -L/usr/local/lib
    endif
endif

BINARY		= demo-native-mc
SRC_DIR		= src
BUILD_DIR	= build

#
#	`config.mk` omits `uxhw.c`, because the Signaloid cloud compiler
#	provides the UxHw API natively. The native Monte Carlo build has to
#	compile the compatibility shim in explicitly.
#
PROJECT_C	= $(SOURCES) uxhw.c

PROJECT_C_OBJ	= $(addprefix $(BUILD_DIR)/,$(PROJECT_C:.c=.o))

.PHONY: local-build local-run clean

local-build: $(BINARY)

$(BINARY): $(PROJECT_C_OBJ)
	$(CC) $(LDFLAGS) -o $@ $(PROJECT_C_OBJ) $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

local-run: local-build
	@echo "=== Transmission-line power (S=0) ==="
	./$(BINARY) -M 10000 -S 0 -T
	@echo ""
	@echo "=== Total decay heat (S=1) ==="
	./$(BINARY) -M 10000 -S 1 -T
	@echo ""
	@echo "=== Neutron reaction rate (S=2) ==="
	./$(BINARY) -M 10000 -S 2 -T
	@echo ""
	@echo "=== DNBR safety margin (S=3) ==="
	./$(BINARY) -M 10000 -S 3 -T
	@echo ""
	@echo "=== Grid power (S=4) ==="
	./$(BINARY) -M 10000 -S 4 -T
	@echo ""
	@echo "=== Flood overflow (S=5) ==="
	./$(BINARY) -M 10000 -S 5 -T
	@echo ""
	@echo "=== Fission gas release (S=6) ==="
	./$(BINARY) -M 10000 -S 6 -T

clean:
	rm -rf $(BUILD_DIR) $(BINARY)
