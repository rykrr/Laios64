################################################################################

TOOLCHAIN	= aarch64-elf
TARGET		= aarch64-unknown-none
ARCHDIR		= kernel/arch/qemu/virt

################################################################################

AS	:= $(TOOLCHAIN)-as
AR	:= $(TOOLCHAIN)-ar
CC	:= $(TOOLCHAIN)-gcc
GDB	:= $(TOOLCHAIN)-gdb
OBJCOPY	:= $(TOOLCHAIN)-objcopy
OBJDUMP	:= $(TOOLCHAIN)-objdump

################################################################################

SYSROOT	:= sysroot
LIBROOT	:= $(SYSROOT)/usr/lib

KERNEL_HEADERS	:= $(shell find kernel/include -type f)
KERNEL_SOURCES	:= $(shell find kernel/ -type f -regex '.*\.[Sc]' ! -path "kernel/arch/qemu/*")
KERNEL_OBJECTS	:= $(patsubst kernel/%.c,kernel/%.o,$(KERNEL_SOURCES))
KERNEL_OBJECTS	:= $(patsubst kernel/%.S,kernel/%.o,$(KERNEL_OBJECTS))
KERNEL_SYMBOLS	:= $(SYSROOT)/kernel.sym
KERNEL_OBJECT	:= $(SYSROOT)/kernel.o
KERNEL			:= $(SYSROOT)/kernel.img

LIBC_HEADERS	:= $(shell find libc/include -type f)
LIBC_SOURCES	:= $(shell find libc/ -type f -regex '.*\.[Sc]')
LIBC_OBJECTS	:= $(patsubst libc/%.c,libc/%.o,$(LIBC_SOURCES))
LIBC_OBJECTS	:= $(patsubst libc/%.S,libc/%.o,$(LIBC_OBJECTS))
LIBC			:= $(LIBROOT)/libc.a
LIBK			:= $(LIBROOT)/libk.a

SYSTEM_INCLUDE_DIR		:= $(SYSROOT)/usr/include
SYSTEM_KERNEL_HEADERS	:= $(patsubst kernel/include/%.h,kernel/%.h,$(KERNEL_HEADERS))
SYSTEM_KERNEL_HEADERS	:= $(addprefix $(SYSTEM_INCLUDE_DIR)/, $(SYSTEM_KERNEL_HEADERS))
SYSTEM_LIBC_HEADERS		:= $(patsubst libc/include/%.h,%.h,$(LIBC_HEADERS))
SYSTEM_LIBC_HEADERS		:= $(addprefix $(SYSTEM_INCLUDE_DIR)/, $(SYSTEM_LIBC_HEADERS))
SYSTEM_HEADERS			:= $(SYSTEM_KERNEL_HEADERS) $(SYSTEM_LIBC_HEADERS)

################################################################################

CFLAGS := $(CFLAGS) -ffreestanding --sysroot=$(SYSROOT) -isystem $(SYSTEM_INCLUDE_DIR)
CFLAGS := $(CFLAGS) -Wall
CFLAGS := $(CFLAGS) -g -D__TARGET_QEMU_VIRT__ -march=armv8-a+nofp -nostdlib

KERNEL_CFLAGS := $(CFLAGS) -T$(ARCHDIR)/linker.ld
LIBC_CFLAGS := $(CFLAGS) -D__LIBC__
LIBK_CFLAGS := $(CFLAGS) -D__LIBK__

KERNEL_LIBS := -lk

################################################################################

$(KERNEL): $(KERNEL_OBJECT)
	cp $< $@
	#$(OBJCOPY) --strip-all -O binary $^ $@

$(KERNEL_SYMBOLS): $(KERNEL_OBJECT)
	$(OBJCOPY) --only-keep-debug $< $@

$(KERNEL_OBJECT): $(KERNEL_OBJECTS) $(LIBK_TARGET) $(SYSTEM_HEADERS)
	$(CC) $(KERNEL_CFLAGS) $(KERNEL_OBJECTS) -o $@ $(KERNEL_LIBS)

kernel/%.o: kernel/%.c $(SYSTEM_HEADERS) $(LIBK)
	$(CC) -c $(KERNEL_CFLAGS) $< -o $@

kernel/%.o: kernel/%.S $(SYSTEM_HEADERS) $(LIBK)
	$(CC) -c $(KERNEL_CFLAGS) $< -o $@

libc/%.o: libc/%.c $(SYSTEM_HEADERS) $(LIBC_HEADERS)
	$(CC) -c $(LIBK_CFLAGS) $< -o $@

libc/%.o: libc/%.S $(SYSTEM_HEADERS) $(LIBC_HEADERS)
	$(CC) -c $(LIBK_CFLAGS) $< -o $@

$(LIBK): $(LIBC_OBJECTS)
	@echo $(LIBC_OBJECTS)
	mkdir -p $(LIBROOT)
	$(AR) rcs -o $@ $(LIBC_OBJECTS)

$(SYSTEM_INCLUDE_DIR)/kernel/%.h: kernel/include/%.h
	@[ -d $(SYSTEM_INCLUDE_DIR) ] || mkdir -p $(SYSTEM_INCLUDE_DIR)/kernel
	cp -r $< $@

$(SYSTEM_INCLUDE_DIR)/%.h: libc/include/%.h
	@[ -d $(SYSTEM_INCLUDE_DIR) ] || mkdir -p $(SYSTEM_INCLUDE_DIR)
	cp -r $< $@

################################################################################

DISPLAY	?= :0.0
SERIAL	= /tmp/serial

QEMU		= DISPLAY=$(DISPLAY) qemu-system-aarch64
QEMU_ARGS	= -M virt -m 1G -cpu cortex-a72 -kernel $(KERNEL)

################################################################################

.PHONY: clean all run

clean:
	find . -type f -name '*.o' -delete
	rm -rf sysroot

debug: $(KERNEL)
	$(QEMU) $(QEMU_ARGS) -S -gdb tcp::1337 -serial file:$(SERIAL) -monitor stdio

run: $(KERNEL)
	$(QEMU) $(QEMU_ARGS) -serial stdio

gdb: $(KERNEL_SYMBOLS)
	#LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib \
		#-ex 'dir kernel' \
	$(GDB) $(KERNEL_SYMBOLS) \
		-ex 'target remote localhost:1337'

################################################################################
