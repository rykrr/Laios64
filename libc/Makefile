################################################################################

TOOLCHAIN	= aarch64-elf
TARGET		= aarch64-unknown-none
SYSROOT		= ../sysroot

################################################################################

CC	= $(TOOLCHAIN)-gcc
CFLAGS	= --sysroot=$(SYSROOT) -isystem $(SYSROOT)/usr/include -nostdlib -march=armv8-a+nofp -ffreestanding

################################################################################

AS	= $(TOOLCHAIN)-as
GDB	= $(TOOLCHAIN)-gdb
OBJCOPY	= $(TOOLCHAIN)-objcopy
OBJDUMP	= $(TOOLCHAIN)-objdump

################################################################################

SOURCES	= $(shell find . -type f -regex '.*\.c')

################################################################################

$(SYSROOT)/libc.a: $(SOURCES)
	$(CC) -fPIC $(CFLAGS) $(SOURCES) -o $@

.PHONY: clean

clean:
	rm $(SYSROOT)/libc.so
