
# Turn off all the built-in stuff
.SUFFIXES:

include config.mk

ifeq ($(ARCH),)

  $(error Invalid configuration, no ARCH)
  
  ifneq $($(SRC_DIR),)
	$(shell "$(SRC_DIR)/configure)
  endif

endif

BIOS_FILENAME = emb-$(ARCH).rom

ifneq ($(BIOS_IS_ELF),0)
BIOS_FILENAME = emb-$(ARCH)
endif

$(info BIOS_FILENAME=$(BIOS_FILENAME))
$(info BIOS_IS_ELF=$(BIOS_IS_ELF))

all: $(BIOS_FILENAME)

.PHONY: all

GDB_EXTRA_STARTUP_CMD=

MATH_SOURCE_NAMES = \
    math/sincos.cc \
    math/truncf.cc

ARCH_SOURCE_NAMES = \
    assert.cc \
    debug.cc \
    string.cc \
    vec.cc \
    render.cc \
    polygon.cc \
    malloc.cc \
    main.cc

DISPI_SOURCE_NAMES = \
    dispi.cc

PCIMMIO_SOURCE_NAMES = \
    arch/pci.cc \

ARCH_SOURCE_NAMES_x86_64 = \
    machine/x86/entry_arch.S \
    arch/x86_64/exception_arch.S \
    arch/pci.cc \
    driver/pci/port_io/pci_arch.cc \
    machine/x86/halt_arch.cc \
    machine/x86/debug_arch.cc \
    driver/display/dispi/dispi.cc \
    driver/display/dispi/dispi_pci.cc

ARCH_SOURCE_NAMES_i386 = \
    machine/x86/entry_arch.S \
    arch/i386/context_arch.cc \
    arch/pci.cc \
    driver/pci/port_io/pci_arch.cc \
    machine/x86/halt_arch.cc \
    machine/x86/debug_arch.cc \
    driver/display/dispi/dispi.cc \
    driver/display/dispi/dispi_pci.cc

ARCH_SOURCE_NAMES_aarch64 = \
    arch/aarch64/entry_arch.S \
    arch/aarch64/halt_arch.cc \
    arch/aarch64/exception_arch.S \
    machine/virt/debug_arch.cc \
    arch/pci.cc \
    driver/pci/ecam/pci_arch.cc \
    driver/display/dispi/dispi.cc \
    driver/display/dispi/dispi_pci.cc

ARCH_SOURCE_NAMES_ppc = \
    arch/ppc/entry_arch_s.S \
    arch/ppc/halt_arch.cc \
    arch/pci.cc \
    driver/debug/pci_serial.cc \
    driver/pci/indexed_io/pci_arch.cc \
    driver/display/dispi/dispi.cc \
    driver/display/dispi/dispi_pci.cc

ARCH_SOURCE_NAMES_mips64el = \
    arch/mips64el/entry_arch.S \
    arch/mips64el/halt_arch.cc \
    arch/pci.cc \
    driver/debug/pci_serial.cc \
    driver/pci/indexed_io/pci_arch.cc \
    driver/display/dispi/dispi.cc \
    driver/display/dispi/dispi_pci.cc

ARCH_SOURCE_NAMES_riscv64 = \
    arch/pci_null.cc \
    machine/sifive/halt_arch.cc \
    machine/virt/debug_arch.cc \
    driver/display/dispi/dispi.cc

ARCH_SOURCE_NAMES_m68k =

#MACHINE_SOURCE_NAMES_x86 = \
#    machine/x86/entry_arch.S \
#    machine/x86/halt_arch.cc \
#    machine/x86/pci_arch.cc \
#    machine/x86/portio_arch.cc \
#    machine/virt/debug_arch.cc
#
#MACHINE_SOURCE_NAMES_virt = \
#    machine/virt/debug_arch.cc \
#    machine/virt/pci_arch.cc \
#    machine/virt/portio_arch.cc
#
#MACHINE_SOURCE_NAMES_sifive = \
#    machine/sifive/pci_arch.cc \
#    machine/sifive/halt_arch.cc \
#    machine/virt/debug_arch.cc

SOURCE_NAMES = \
    $(ARCH_SOURCE_NAMES) \
    $(ARCH_SOURCE_NAMES_$(ARCH))

#$(MACHINE_SOURCE_NAMES_$(MACHINE))

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
		emb-$(ARCH).sym \
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
	-mno-red-zone -fPIE

ARCH_FLAGS_aarch64 =

QEMU_RAM ?= 1536M

CXX_FLAGS_COMMON = \
	-g \
	-I$(SRC_DIR) \
	-I$(BUILD_INCLUDES) \
	-W -Wall -Wextra -Wpedantic -Werror -O0 \
	-Wdouble-promotion -Wmissing-declarations \
	-ffreestanding -fbuiltin \
	-Werror=format -Werror=return-type \
	-Wa,-g \
	-fno-exceptions -fno-asynchronous-unwind-tables \
	-nostdlib \
	-static \
	-Wl,--no-relax \
	-Wl,-m$(LINKER_EMULATION) \
	-fno-common \
	$(ARCH_FLAGS_$(ARCH))

#-Wl,--no-dynamic-linker
#-fPIE

LINKFLAGS = $(CXX_FLAGS_COMMON) \
	-o $@ \
	-Wl,-T,"${SRC_DIR}/arch/$(ARCH)/rom_link_arch.ld" \
	-Wl,-z,max-page-size=64 \
	-Wl,-Map,$@.map \
	$(MARCH_FLAGS) \
	$(LIBGCC)

#-Wl,--orphan-handling,warn

COMPILEFLAGS = \
	$(CXX_FLAGS_COMMON) \
	$(ARCH_FLAGS_$(ARCH)) \
	$(MARCH_FLAGS)

emb-$(ARCH).sym: emb-$(ARCH)
	$(NM) --demangle $< > $@

emb-$(ARCH): $(OBJECTS_ALL) Makefile config.mk
	$(CXX) -o $@ $(OBJECTS_ALL) $(LINKFLAGS) $(CXXFLAGS) $(LDFLAGS)

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
	$(LESS) $$@

# Preprocess only, do not compile, do not assemble, do not create object file
obj/$(patsubst %.$(2),%.i,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -E $$<
	$(LESS) $$@

obj/$(patsubst %.$(2),%.d,$(1)): $(patsubst %.$(2),%.o,$(1))

endef

$(foreach file,$(SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc)))

$(foreach file,$(SOURCE_NAMES_S), \
	$(eval $(call compile_extension,$(file),S)))

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
		-smp $(QEMU_SMP) \
		\
		-d guest_errors,cpu_reset \
		\
		-no-reboot -no-shutdown $(QEMUEXTRAFLAGS)

debug: $(BIOS_FILENAME)
	$(QEMU) -s -S $(QEMUTRACEFLAGS) $(QEMUFLAGS)

.PHONY: debug-$(ARCH)

debug-bochs: $(BIOS_FILENAME) emb-$(ARCH).sym
	$(BOCHS) -f $(SRC_DIR)/machine/x86/bochs-debug.bxrc \
		-rc $(SRC_DIR)/machine/x86/bochs-debugger-commands

.PHONY: debug-bochs

run: $(BIOS_FILENAME)
	$(QEMU) -s $(QEMUFLAGS)

.PHONY: run

run-kvm: $(BIOS_FILENAME)
	$(QEMU) -s $(QEMUFLAGS) -enable-kvm

.PHONY: run-kvm

disassemble: emb-$(ARCH)
	$(OBJDUMP) -S $< | $(LESS)

.PHONY: disassemble

objdump: emb-$(ARCH)
	$(OBJDUMP) -x $< | $(LESS)

.PHONY: objdump

run-nogdb: $(BIOS_FILENAME)
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
	@echo "$${CXX%/*}"

.PHONY: toolchain_hint

toolchain_path:
	@echo "PATH=$${CXX%/*}:\$$PATH"

.PHONY: toolchain_path

-include $(DEPFILES)
