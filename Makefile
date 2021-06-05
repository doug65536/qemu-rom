
# Turn all the built-in stuff off
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
SOURCE_NAMES_ALL = $(SOURCE_NAMES_CC) $(SOURCE_NAMES_S)

OBJECTS_CC = $(patsubst %.cc,obj/%.o,$(SOURCE_NAMES_CC))
OBJECTS_S = $(patsubst %.S,obj/%.o,$(SOURCE_NAMES_S))

OBJECTS_ALL = $(OBJECTS_CC) $(OBJECTS_S)

#SOURCE_PATHS_CC = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_CC))
#SOURCE_PATHS_S = $(patsubst %,${SRC_DIR}/%,$(SOURCE_NAMES_S))
#SOURCE_PATHS = $(SOURCE_PATHS_CC) $(SOURCE_PATHS_S)

# Generated dependencies
DEPFILES = $(patsubst %.o,%.d,$(patsubst $(SRC_DIR)/%,%,$(OBJECTS_ALL)))
$(DEPFILES):

clean:
	$(RM) -f emb-$(ARCH) emb-$(ARCH).rom $(OBJECTS_ALL) $(DEPFILES)
	$(RM) -rf obj

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

ARCH_FLAGS_i386 = \
	-m32 -Wa,--32

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

LDFLAGS = $(CXX_FLAGS_COMMON) \
	-o $@ \
	$(ARCH_FLAGS_$(ARCH)) \
	-Wl,-T,"${SRC_DIR}/arch/$(ARCH)/emb.ld" \
	-Wl,-z,max-page-size=64 \
	-Wl,-Map,$@.map \
	-fpie \
	$(MCPU_FLAG) \
	$(LIBGCC)

CXXFLAGS = \
	$(CXX_FLAGS_COMMON) \
	$(ARCH_FLAGS_$(ARCH)) \
	$(MCPU_FLAG)

ARCH_FLAGS_x86_64 = \
	-mno-red-zone

emb-$(ARCH): $(OBJECTS_ALL) Makefile config.mk
	$(CXX) -o $@ $(OBJECTS_ALL) $(LDFLAGS)

emb-$(ARCH): ${SRC_DIR}/arch/$(ARCH)/emb.ld | Makefile config.mk

emb-$(ARCH).rom: emb-$(ARCH)
	$(OBJCOPY) --strip-debug -Obinary $< $@
DEP_FROM_SOURCE = $(patsubst %.cc,%.d,$(patsubst %.S,%.d,$(1)))
OBJ_FROM_SOURCE = $(patsubst %.cc,%.o,$(patsubst %.S,%.o,$(1)))
OUT_FROM_SOURCE = $(call DEP_FROM_SOURCE,$(1)) $(call OBJ_FROM_SOURCE,$(1))

define compile_cc=

obj/$(patsubst %.cc,%.o,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -c $$< -MMD $(CXXFLAGS)

obj/$(patsubst %.cc,%.d,$(1)): $(patsubst %.cc,%.o,$(1))

endef

define compile_s=

obj/$(patsubst %.S,%.o,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -c $$< -MMD $(CXXFLAGS)

obj/$(patsubst %.S,%.d,$(1)): $(patsubst %.S,%.o,$(1))

endef

$(foreach file,$(SOURCE_NAMES_CC), \
	$(eval $(call compile_cc,$(file))))

$(foreach file,$(SOURCE_NAMES_S), \
	$(eval $(call compile_s,$(file))))

TRACEFLAGS = \
	-trace 'pci*' 

QEMUFLAGS = \
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
		-no-reboot -no-shutdown $(QEMUEXTRAFLAGS)

debug: emb-$(ARCH).rom
	$(QEMU) -s -S $(TRACEFLAGS) $(QEMUFLAGS)

.PHONY: debug-$(ARCH)

run: emb-$(ARCH).rom
	$(QEMU) -s $(QEMUFLAGS)

.PHONY: run

run-nogdb: emb-$(ARCH).rom
	$(QEMU) $(QEMUFLAGS)

.PHONY: run-nogdb

attach: emb-$(ARCH)
	$(GDB) \
		-ex 'source ../../dgos/src/gdbhelpers' \
		-ex 'target remote :1234' \
		-ex 'symbol-file $<' $(GDB_EXTRA_STARTUP_CMD)

.PHONY: attach

-include $(DEPFILES)
