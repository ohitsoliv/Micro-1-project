# ==============================================================================
# Makefile — Snake Game for ATmega328P
# EE3463 Final Project, UTSA — Team 4
# ==============================================================================

# ---- MCU and clock ----
MCU        = atmega328p
F_CPU      = 16000000UL

# ---- Toolchain ----
CC         = avr-gcc
OBJCOPY    = avr-objcopy
SIZE       = avr-size
AVRDUDE    = avrdude

# ---- Programmer (uncomment ONE) ----
# Option A: USBasp
PROGRAMMER = usbasp
AVRDUDE_PORT =
# Option B: Arduino as ISP (uncomment these two, comment out Option A)
# PROGRAMMER = stk500v1
# AVRDUDE_PORT = -P COM3 -b 19200

# ---- Directories ----
SRCDIR     = src
INCDIR     = include
BUILDDIR   = build

# ---- Compiler / linker flags ----
CFLAGS     = -Os -Wall -Wextra -std=c11 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INCDIR)
LDFLAGS    = -mmcu=$(MCU)

# ---- Source / object lists ----
SRCS       = $(wildcard $(SRCDIR)/*.c)
OBJS       = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
TARGET     = $(BUILDDIR)/snake

# ==============================================================================
# Targets
# ==============================================================================

.PHONY: all flash clean size

all: $(TARGET).hex size

# Link object files into ELF
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Convert ELF to Intel HEX for flashing
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Compile each .c to .o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Create build directory if it doesn't exist
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Flash the HEX file to the MCU
flash: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) $(AVRDUDE_PORT) -U flash:w:$<:i

# Print code size
size: $(TARGET).elf
	$(SIZE) --mcu=$(MCU) --format=avr $<

# Remove all build artifacts
clean:
	rm -rf $(BUILDDIR)
