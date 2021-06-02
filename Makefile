
include config.mk

VPATH = ${SRC_DIR}

SOURCE_NAMES = \
	entry.S \
	pci.cc \
	bochs.cc \
	assert.cc \
	debug.cc \
	portio.cc \
	main.cc

SOURCE_NAMES_CC = $(filter %.cc,$(SOURCE_NAMES))
SOURCE_NAMES_S = $(filter %.S,$(SOURCE_NAMES))

OBJECTS_CC = $(patsubst %.cc,%.o,$(SOURCES_CC))
OBJECTS_S = $(patsubst %.S,%.o,$(SOURCE_S))

OBJECTS = $(OBJECTS_CC) $(OBJECTS_S)

SOURCES_CC = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_CC))
SOURCES_S = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_S))
SOURCES = $(SOURCES_CC) $(SOURCES_S)

all: emb.bin

.PHONY: all

clean:
	rm -f emb

.PHONY: clean

#-Wl,--orphan-handling,warn

emb: $(SOURCES)
	$(CXX) -m32 -Wl,-melf_i386 -g \
		-W -Wall -Wextra -Wpedantic -O0 \
		-ffreestanding -fno-builtin \
		-Werror=format -Werror=return-type \
		-o $@ \
		-Wa,-g \
		-fno-exceptions -fno-asynchronous-unwind-tables \
		-Wl,-T,"${SRC_DIR}/emb.ld" \
		-Wl,-z,max-page-size=64 \
		-nostdlib \
		-static \
		-Wl,--no-dynamic-linker \
		-fno-pic -fno-pie -fno-PIC -fno-PIE \
		-fno-common \
		$(SOURCES) \
		$(LIBGCC)

emb: Makefile config.mk ${SRC_DIR}/emb.ld

emb.bin: emb
	$(OBJCOPY) --strip-debug -Obinary $< $@

debug: emb.bin
	$(QEMU) \
		-bios $< \
		\
		-vga none \
		-device secondary-vga \
		-device secondary-vga \
		-device secondary-vga \
		-device secondary-vga \
		-device secondary-vga \
		-device secondary-vga \
		-device secondary-vga \
		\
		-chardev stdio,id=debug-out \
		-device isa-debugcon,chardev=debug-out \
		\
		-s -S -no-reboot -no-shutdown \
		\
		-trace 'vga*' -trace 'pci*' \
		-d int -trace 'pci*'

.PHONY: debug

attach: emb
	$(GDB) emb \
		-ex 'target remote :1234' \
		-ex 'source ../../dgos/src/gdbhelpers'

.PHONY: attach
