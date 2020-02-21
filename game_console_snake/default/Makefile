###############################################################################
# Makefile for the project game_console
###############################################################################

## General Flags
PROJECT = game_console
MCU = atmega16
TARGET = LineTracer.elf
CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2          -DF_CPU=7379300UL -Os -fsigned-char -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = game_console.o snake.o snake_draw.o snake_game.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) LineTracer.hex LineTracer.eep## Compile
game_console.o: ../game_console.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

snake.o: ../snake.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

snake_draw.o: ../snake_draw.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

snake_game.o: ../snake_game.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) LineTracer.elf dep/* LineTracer.hex LineTracer.eep

## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)

