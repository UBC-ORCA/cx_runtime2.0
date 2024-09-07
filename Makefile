.PHONY: clean all qemu

# CC = ${RISCV}/llvm/build-linux/bin/clang
CC = ${RISCV}riscv32-unknown-linux-gnu-gcc
AR = ${RISCV}riscv32-unknown-linux-gnu-ar

CCX86 = gcc
ARX86 = ar

BDIR := build
LDIR := $(BDIR)/lib
SRC  := src

QEMU-BDIR := build-qemu
QEMU-LDIR := $(QEMU-BDIR)/lib

ZOO-DIR := zoo

cx_objects := $(BDIR)/ci.o
qemu_objects := $(QEMU-BDIR)/exports.o
cx_libraries := $(BDIR)/addsub.o $(BDIR)/muldiv.o $(BDIR)/mulacc.o $(BDIR)/p-ext.o
cx_helpers := $(QEMU-BDIR)/addsub_func.o $(QEMU-BDIR)/muldiv_func.o $(QEMU-BDIR)/mulacc_func.o $(QEMU-BDIR)/p-ext_func.o 

all: $(QEMU-LDIR)/libcx_index.so $(LDIR)/libci.a

########### Building the library QEMU needs ###########

$(QEMU-LDIR)/libcx_index.so: $(qemu_objects) $(cx_helpers) | $(QEMU-LDIR)
	$(ARX86) -rcs $@ $^

$(QEMU-BDIR)/exports.o : $(ZOO-DIR)/exports.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-LDIR):
	mkdir -p $(QEMU-LDIR)

########### Building the Extension Functionalities ###########

$(QEMU-BDIR)/addsub_func.o : $(ZOO-DIR)/addsub/addsub_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/muldiv_func.o : $(ZOO-DIR)/muldiv/muldiv_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/mulacc_func.o : $(ZOO-DIR)/mulacc/mulacc_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@

$(QEMU-BDIR)/p-ext_func.o : $(ZOO-DIR)/p-ext/p-ext_func.c | $(QEMU-LDIR)
	$(CCX86) -c $< -o $@


########### Building Extension Object Files ###########


$(BDIR)/addsub.o: $(ZOO-DIR)/addsub/addsub.c $(ZOO-DIR)/addsub/addsub.h | $(BDIR)
	$(CC) -c $< -o $@

$(BDIR)/muldiv.o: $(ZOO-DIR)/muldiv/muldiv.c $(ZOO-DIR)/muldiv/muldiv.h | $(BDIR)
	$(CC) -c $< -o $@

$(BDIR)/mulacc.o: $(ZOO-DIR)/mulacc/mulacc.c $(ZOO-DIR)/mulacc/mulacc.h | $(BDIR)
	$(CC) -c $< -o $@

$(BDIR)/p-ext.o: $(ZOO-DIR)/p-ext/p-ext.c $(ZOO-DIR)/p-ext/p-ext.h | $(BDIR)
	$(CC) -c $< -o $@

$(BDIR):
	mkdir -p $(BDIR)

########### Building Extension Library ###########

# $(LDIR)/libci_m.a: $(cx_objects_m) $(cx_libraries) | $(LDIR)
# 	$(AR) -rcs $@ $(cx_objects_m) $(cx_libraries)

$(LDIR)/libci.a: $(cx_objects) $(cx_libraries) | $(LDIR)
	$(AR) -rcs $@ $(cx_objects) $(cx_libraries)

$(BDIR)/%.o : $(SRC)/%.c | $(LDIR)
	$(CC) -c $< -o $@

$(LDIR):
	mkdir -p $(LDIR)

###########   Running on QEMU   ###########
qemu:
	./qemu_cx/build/qemu-system-riscv32 -nographic -machine virt \
	-kernel linux_cx/arch/riscv/boot/Image \
	-initrd utils/initramfs/initramfs.cpio.gz \
	-append "console=ttyS0"

clean:
	rm -rf build/
	rm -rf build-qemu/
	rm -f example