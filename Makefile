
# Turn off all the built-in stuff
.SUFFIXES:

include config.mk

ifeq ($(ARCH),)

  $(error Invalid configuration, no ARCH)
  
  ifneq $($(SRC_DIR),)
    $(shell "$(SRC_DIR)/configure)
  endif

endif

all: emb-$(ARCH).rom

.PHONY: all

SOURCE_NAMES = \
	$(CONFIG_SOURCES) \
	arch/pci.cc \
	dispi.cc \
	assert.cc \
	debug.cc \
	string.cc \
	main.cc

SOURCE_NAMES_CC = $(filter %.cc,$(SOURCE_NAMES))
SOURCE_NAMES_S = $(filter %.S,$(SOURCE_NAMES))
SOURCE_NAMES_ALL = $(SOURCE_NAMES_CC) $(SOURCE_NAMES_S)

OBJECTS_CC = $(patsubst %.cc,obj/%.o,$(SOURCE_NAMES_CC))
OBJECTS_S = $(patsubst %.S,obj/%.o,$(SOURCE_NAMES_S))

OBJECTS_ALL = $(OBJECTS_CC) $(OBJECTS_S)

COMPILE_ONLY_ALL = $(patsubst %.o,%.S,$(OBJECTS_ALL))
PREPROCESS_ONLY_ALL = $(patsubst %.o,%.i,$(OBJECTS_ALL))

# Generated dependencies
DEPFILES = $(patsubst %.o,%.d,$(patsubst $(SRC_DIR)/%,%,$(OBJECTS_ALL)))
$(DEPFILES):

clean:
	$(RM) -f emb-$(ARCH) \
		emb-$(ARCH) \
		emb-$(ARCH).rom \
		emb-$(ARCH).map \
		rom \
		$(OBJECTS_ALL) \
		$(DEPFILES) \
		$(COMPILE_ONLY_ALL) \
		$(PREPROCESS_ONLY_ALL)
	$(RM) -rf obj

distclean: clean
	rm -f config.mk

.PHONY: clean

config:
	$(SRC_DIR)/configure 

.PHONY: config

reconfigure:
	$(RM) -f config.mk && unset \
		DEBUG_CON QEMU_MACHINE MARCH_FLAGS \
		&& \
		$(SRC_DIR)/configure

.PHONY: reconfigure

config.mk: $(SRC_DIR)/configure $(SRC_DIR)/Makefile
	$(SRC_DIR)/configure 

Makefile: $(SRC_DIR)/Makefile
	$(CP) -u $< $@
	$(TOUCH) -r $< $@

#-Wl,--orphan-handling,warn

ARCH_FLAGS_i386 = \
	-m32 -Wa,--32

ARCH_FLAGS_x86_64 = \
	-mno-red-zone

ARCH_FLAGS_aarch64 =

QEMU_RAM = 8G

CXX_FLAGS_COMMON = \
	-g \
	-I$(SRC_DIR) \
	-I$(BUILD_INCLUDES) \
	-W -Wall -Wextra -Wpedantic -Werror -O0 \
	-ffreestanding -fbuiltin \
	-Werror=format -Werror=return-type \
	-Wa,-g \
	-fno-exceptions -fno-asynchronous-unwind-tables \
	-nostdlib \
	-static \
	-fPIE \
	-Wl,--no-dynamic-linker \
	-Wl,-m$(LINKER_EMULATION) \
	-fno-common \
	$(ARCH_FLAGS_$(ARCH))

LINKFLAGS = $(CXX_FLAGS_COMMON) \
	-o $@ \
	-Wl,-T,"${SRC_DIR}/arch/$(ARCH)/rom_link_arch.ld" \
	-Wl,-z,max-page-size=64 \
	-Wl,-Map,$@.map \
	$(MARCH_FLAGS) \
	$(LIBGCC)

COMPILEFLAGS = \
	$(CXX_FLAGS_COMMON) \
	$(ARCH_FLAGS_$(ARCH)) \
	$(MARCH_FLAGS)

emb-$(ARCH): $(OBJECTS_ALL) Makefile config.mk
	$(CXX) -o $@ $(OBJECTS_ALL) $(LINKFLAGS) $(LDFLAGS)

emb-$(ARCH): ${SRC_DIR}/arch/$(ARCH)/rom_link_arch.ld Makefile config.mk

emb-$(ARCH).rom: emb-$(ARCH)
	$(OBJCOPY) --strip-debug -Obinary $< $@

define compile_extension=

# Preprocess, compile, and assemble to object file normally
obj/$(patsubst %.$(2),%.o,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -c $$<

# Preprocess, compile, but do not assemble, and do not create object file
obj/$(patsubst %.$(2),%.S,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -S $$<

# Preprocess only, do not compile, do not assemble, do not create object file
obj/$(patsubst %.$(2),%.i,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -E $$<

obj/$(patsubst %.$(2),%.d,$(1)): $(patsubst %.$(2),%.o,$(1))

endef

$(foreach file,$(SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc)))

$(foreach file,$(SOURCE_NAMES_S), \
	$(eval $(call compile_extension,$(file),S)))



QEMUSMP = 4

QEMUTRACEFLAGS = \
	-trace 'pci*' 

QEMUFLAGS = \
		-bios $< \
		$(QEMU_MACHINE) \
		$(QEMU_CPU) \
		-m $(QEMU_RAM) \
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
		$(DEBUG_CON) \
		\
		-smp $(QEMUSMP) \
		\
		-no-reboot -no-shutdown $(QEMUEXTRAFLAGS)

debug: emb-$(ARCH).rom
	$(QEMU) -s -S $(QEMUTRACEFLAGS) $(QEMUFLAGS)

.PHONY: debug-$(ARCH)

run: emb-$(ARCH).rom
	$(QEMU) -s $(QEMUFLAGS)

.PHONY: run

disassemble: emb-$(ARCH)
	$(OBJDUMP) -S $< | $(LESS)

.PHONY: disassemble

objdump: emb-$(ARCH)
	$(OBJDUMP) -x $< | $(LESS)

.PHONY: objdump

run-nogdb: emb-$(ARCH).rom
	$(QEMU) $(QEMUFLAGS)

.PHONY: run-nogdb

attach: emb-$(ARCH)
	$(GDB) $< \
		$(GDB_EXTRA_STARTUP_CMD) \
		-ex 'source ../../dgos/src/gdbhelpers' \
		-ex 'target remote :1234'

#-ex 'symbol-file $<'

.PHONY: attach

toolchain_hint:
	echo "$${CXX%/*}"

.PHONY: toolchain_hint

-include $(DEPFILES)
