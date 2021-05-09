# Based off SYS86 project (https://github.oom/mfld-fr/sys86)
# Global Makefile

CC = ia16-elf-gcc
AS = ia16-elf-as
LD = ia16-elf-ld

BUILDSTAMP = $(shell date)
DEFINES += -DBUILD_TIMESTAMP='"$(BUILDSTAMP)"'

CFLAGS = -ffunction-sections -Werror -ffreestanding -O1 -std=gnu99 -mtune=i80186 -march=i80186 $(DEFINES)
ASFLAGS = -ffreestanding -O1 -std=gnu99 -nostdinc -mtune=i80186 -march=i80186
LDFLAGS = -nostdlib -T bios.ld
BOOT_LDFLAGS = -nostdlib -T boot.ld

ROMSIZE = 524288 #65536

EXE = _a.bin

BOOT_OBJ = \
	fakesector.o

OBJS = \
	bios.o \
	delay.o \
	serial.o \
	entry.o \
	int-handler.o \
	services_disk.o \
	services_video.o \
	services_keyboard.o \
	services_memory_size.o \
	services_bios_eq_flags.o \
	services_bios_async_comm.o \
	services_printer.o \
	services_system_bios.o \
	services_rtc.o \
	sd_elm.o \
	sd_asm.o \
	udivsi3.o \
	divsi3.o \
	ia16-ldivmodu.o \
	ia16-ldivmods.o \
	boot_fake.o \
	gcs.o \
	leds.o \
	pff.o \
	c-helpers.o \
	# end of list
.PHONY : all clean

all: $(EXE).hex $(EXE)-rom.bin

boot_fake.c: boot.bin
	xxd -i $< > $(@)

boot.bin: $(BOOT_OBJ)
	$(LD) $(BOOT_LDFLAGS) -M -o $(@) $< > $(@).map

$(EXE)-rom.bin: $(EXE)
	@echo "== creating ${ROMSIZE} bin image"
	@echo PAD=$(shell echo ${ROMSIZE}-$(shell stat --printf="%s" $(EXE)) | bc)
	@dd if=/dev/zero of=$(@) bs=$(shell echo ${ROMSIZE}-$(shell stat --printf="%s" $(EXE)) | bc) count=1
	@dd if=$(EXE) >> $(@)

$(EXE).hex: $(EXE)
	@echo "== BIN->HEX $< -> $(@)"
	@srec_cat $< -Binary -offset 0 -Output $@ -Intel

$(EXE): $(OBJS)
	$(LD) $(LDFLAGS) -M -o $(EXE) $(OBJS) > $(EXE).map

clean:
	rm -f $(EXE) $(OBJS) $(EXE).map boot.bin boot_fake.c
