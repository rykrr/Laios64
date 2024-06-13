################################################################################

TOOLCHAIN	= aarch64-none-elf
TARGET		= aarch64-unknown-none

################################################################################

CC	= $(TOOLCHAIN)-gcc
CFLAGS	= -march=armv8-a+nofp -D__TARGET_QEMU_VIRT_ -ffreestanding -Tlinker.ld -nostdlib

################################################################################

AS	= $(TOOLCHAIN)-as
GDB	= $(TOOLCHAIN)-gdb
OBJCOPY	= $(TOOLCHAIN)-objcopy
OBJDUMP	= $(TOOLCHAIN)-objdump

################################################################################

BASEDIR = ../../../..
SOURCES	= $(shell find $(BASEDIR)/kernel -type f -regex '.*\.r?s')
SOURCES	= $(shell find $(BASEDIR)/kernel -type f -regex '.*\.[cs]')

################################################################################

OUTPUT	= kernel.o
KERNEL	= kernel.img
SYMBOLS	= kernel.sym
SERIAL	= serial.log

################################################################################

ifdef DISPLAY
override DISPLAY := DISPLAY=$(DISPLAY)
endif

QEMU		= $(DISPLAY) qemu-system-aarch64
QEMU_ARGS	= -M virt -m 1G -cpu cortex-a72 -kernel $(OUTPUT)

################################################################################

.PHONY: all clean init run debug gdb
all: $(KERNEL) $(SYMBOLS)

clean:
	rm -rf target $(OUTPUT) $(KERNEL) $(SYMBOLS) $(SERIAL)

################################################################################

$(KERNEL) $(SYMBOLS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT)
	$(OBJCOPY) --only-keep-debug $(OUTPUT) $(SYMBOLS)
	$(OBJCOPY) --strip-all -O binary $(OUTPUT) $(KERNEL)

debug: $(KERNEL)
	$(QEMU) $(QEMU_ARGS) -S -gdb tcp::1337 -serial file:$(SERIAL) -monitor stdio

run: $(KERNEL)
	$(QEMU) $(QEMU_ARGS) -serial stdio

gdb: $(SYMBOLS)
	LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib \
	$(GDB) $(SYMBOLS) \
		-ex 'dir kernel' \
		-ex 'target remote localhost:1337'

#init:
#	rustup default nightly
#	rustup target add $(TARGET)

################################################################################
