
include config.mk

ifeq ($(ARCH),)

  $(error Invalid configuration, no ARCH)
  
  ifneq $($(SRC_DIR),)
    $(shell "$(SRC_DIR)/configure)
  endif

endif

SOURCE_NAMES = \
	arch/$(ARCH)/entry.S \
	arch/$(ARCH)/pci_arch.cc \
	arch/$(ARCH)/portio.cc \
	arch/pci.cc \
	dispi.cc \
	assert.cc \
	debug.cc \
	main.cc

SOURCE_NAMES_CC = $(filter %.cc,$(SOURCE_NAMES))
SOURCE_NAMES_S = $(filter %.S,$(SOURCE_NAMES))

OBJECTS_CC = $(patsubst %.cc,%.o,$(SOURCES_CC))
OBJECTS_S = $(patsubst %.S,%.o,$(SOURCE_S))

OBJECTS = $(OBJECTS_CC) $(OBJECTS_S)

SOURCES_CC = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_CC))
SOURCES_S = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_S))
SOURCES = $(SOURCES_CC) $(SOURCES_S)

all: emb-$(ARCH)

.PHONY: all

clean:
	rm -f emb-$(ARCH) emb-$(ARCH).rom

distclean: clean
	rm -f config.mk

.PHONY: clean

config:
	$(SRC_DIR)/configure 

.PHONY: config

reconfigure:
	( source config.mk ; $(RM) -f config.mk ; $(SRC_DIR)/configure )

.PHONY: reconfigure

config.mk: $(SRC_DIR)/configure
	$(SRC_DIR)/configure 

Makefile: $(SRC_DIR)/Makefile
	$(CP) -u $< $@
	$(TOUCH) -r $< $@

#-Wl,--orphan-handling,warn

CXX_FLAGS_COMMON = \
	-g \
	-I$(SRC_DIR) \
	-W -Wall -Wextra -Wpedantic -O0 \
	-ffreestanding -fbuiltin \
	-Werror=format -Werror=return-type \
	-Wa,-g \
	-fno-exceptions -fno-asynchronous-unwind-tables \
	-nostdlib \
	-static \
	-Wl,--no-dynamic-linker \
	-Wl,-m$(LINKER_EMULATION) \
	-fno-common

ARCH_FLAGS_x86_64 = \
	-mno-red-zone

ARCH_FLAGS_i386 = \
	-m32 -Wa,--32

emb-$(ARCH): $(SOURCES) Makefile config.mk
	$(CXX) $(CXX_FLAGS_COMMON) \
		-o $@ \
		$(ARCH_FLAGS_$(ARCH)) \
		-Wl,-T,"${SRC_DIR}/arch/$(ARCH)/emb.ld" \
		-Wl,-z,max-page-size=64 \
		-Wl,-Map,$@.map \
		-fpie \
		$(MCPU_FLAG) \
		$(SOURCES) \
		$(LIBGCC)

emb-$(ARCH): ${SRC_DIR}/arch/$(ARCH)/emb.ld | Makefile config.mk

emb-$(ARCH).rom: emb-$(ARCH)
	$(OBJCOPY) --strip-debug -Obinary $< $@

debug: emb-$(ARCH).rom
	$(QEMU) \
		-bios $< \
		$(QEMU_MACHINE) \
		-cpu max \
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
		$(DEBUG_CON) \
		\
		-smp 4 \
		\
		-s -S -no-reboot -no-shutdown

.PHONY: debug-$(ARCH)

run: emb-$(ARCH).rom
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
		$(DEBUG_CON) \
		\
		-no-reboot -no-shutdown

.PHONY: run

#-display sdl
#		-device pxb-pcie

#-trace 'vga*' -trace 'pci*'
#-d int -trace 'pci*'

attach: emb-$(ARCH)
	$(GDB) \
		-ex 'source ../../dgos/src/gdbhelpers' \
		-ex 'target remote :1234' \
		-ex 'file $<' $(GDB_EXTRA_STARTUP_CMD)

.PHONY: attach

allarch:
	unset ARCH \
		LINKER_EMULATION \
		QEMU_MACHINE \
		HOSTOS \
		SRC_DIR \
		COMPILER_PREFIX \
		LIBGCC \
		GDB_EXTRA_STARTUP_CMD \
		CXX \
		QEMU \
		GDB \
		OBJCOPY \
	&& \
	HOST_i386=x86_64-$(HOSTOS) \
		&& \
		HOST_x86_64=x86_64-$(HOSTOS) \
		&& \
		HOST_aarch64=aarch64-linux-gnu \
		&& \
		mkdir -p arch_i386 \
		&& \
		mkdir -p arch_x86_64 \
		&& \
		mkdir -p arch_aarch64 \
		&& ( \
			cd arch_i386 \
			&& \
			ARCH=i386 HOST=$$HOST_i386 ../$(SRC_DIR)/configure \
			&& \
			cat config.mk \
		) && ( \
			cd arch_x86_64 \
			&& \
			ARCH=x86_64 HOST=$$HOST_x86_64 ../$(SRC_DIR)/configure \
			&& \
			cat config.mk \
		) && ( \
			cd arch_aarch64 \
			&& \
			ARCH=aarch64 HOST=$$HOST_aarch64 ../$(SRC_DIR)/configure \
			&& \
			cat config.mk \
		)

.PHONY: allarch

