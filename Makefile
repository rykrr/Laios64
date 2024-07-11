################################################################################

TOOLCHAIN	= aarch64-elf
TARGET		= aarch64-unknown-none
ARCHDIR		= kernel/arch/qemu/virt

################################################################################

AS	:= $(TOOLCHAIN)-as
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
CFLAGS := $(CFLAGS) -D__TARGET_QEMU_VIRT__ -march=armv8-a+nofp

KERNEL_CFLAGS := $(CFLAGS) -T$(ARCHDIR)/linker.ld -nostdlib -static -lk
LIBC_CFLAGS := $(CFLAGS) -D__LIBC__ -nostdlib -fPIC -shared
LIBK_CFLAGS := $(CFLAGS) -D__LIBK__ -nostdlib -fPIC -shared

################################################################################

$(KERNEL): $(KERNEL_OBJECT)
	$(OBJCOPY) --strip-all -O binary $^ $@

$(KERNEL_SYMBOLS): $(KERNEL_OBJECT)
	$(OBJCOPY) --only-keep-debug $< $@

$(KERNEL_OBJECT): $(KERNEL_OBJECTS) $(LIBK_TARGET) $(SYSTEM_HEADERS)
	$(CC) $(KERNEL_CFLAGS) $(KERNEL_OBJECTS) -o $@

kernel/%.o: kernel/%.c $(SYSTEM_HEADERS) $(LIBK)
	$(CC) -c $(KERNEL_CFLAGS) $< -o $@

kernel/%.o: kernel/%.S $(SYSTEM_HEADERS) $(LIBK)
	$(CC) -c $(KERNEL_CFLAGS) $< -o $@

$(LIBC): $(LIBC_SOURCES) $(SYSTEM_HEADERS)
	mkdir -p $(LIBROOT)
	$(CC) $(LIBC_CFLAGS) $(LIBC_SOURCES) -o $@

$(LIBK): $(LIBC_SOURCES) $(SYSTEM_HEADERS)
	mkdir -p $(LIBROOT)
	$(CC) $(LIBK_CFLAGS) $(LIBC_SOURCES) -o $@

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
