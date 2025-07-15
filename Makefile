# Micro Kernel Makefile
# 目标架构
ARCH := x86_64

# 工具链
CC := gcc
LD := ld
AS := nasm
OBJCOPY := objcopy
OBJDUMP := objdump

# 编译选项
CFLAGS := -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector
CFLAGS += -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS += -m64 -mcmodel=kernel -fno-pic -fno-pie
CFLAGS += -Wall -Wextra -Werror -O2 -g
CFLAGS += -Iinclude -std=gnu99

# 链接选项
LDFLAGS := -nostdlib -static -Wl,--build-id=none
LDFLAGS += -Wl,-z,max-page-size=0x1000 -Wl,-z,common-page-size=0x1000

# 汇编选项
ASFLAGS := -f elf64

# 目录
SRCDIR := src
INCDIR := include
OBJDIR := obj
BINDIR := bin
ARCHDIR := arch/$(ARCH)

# 源文件
KERNEL_SOURCES := $(SRCDIR)/kernel/main.c
KERNEL_SOURCES += $(SRCDIR)/kernel/sched.c
KERNEL_SOURCES += $(SRCDIR)/kernel/sched_fair.c
KERNEL_SOURCES += $(SRCDIR)/mm/buddy.c
KERNEL_SOURCES += $(SRCDIR)/fs/vfs.c
KERNEL_SOURCES += $(SRCDIR)/net/socket.c
KERNEL_SOURCES += $(SRCDIR)/ipc/msg.c
KERNEL_SOURCES += $(SRCDIR)/drivers/char.c

# 架构相关源文件
ARCH_SOURCES := $(ARCHDIR)/boot.S
ARCH_SOURCES += $(ARCHDIR)/entry.S
ARCH_SOURCES += $(ARCHDIR)/switch.S
ARCH_SOURCES += $(ARCHDIR)/interrupt.S

# 对象文件
KERNEL_OBJECTS := $(KERNEL_SOURCES:%.c=$(OBJDIR)/%.o)
ARCH_OBJECTS := $(ARCH_SOURCES:%.S=$(OBJDIR)/%.o)
ALL_OBJECTS := $(KERNEL_OBJECTS) $(ARCH_OBJECTS)

# 目标文件
KERNEL_ELF := $(BINDIR)/kernel.elf
KERNEL_BIN := $(BINDIR)/kernel.bin
KERNEL_ISO := $(BINDIR)/kernel.iso

# 默认目标
all: $(KERNEL_ELF) $(KERNEL_BIN)

# 创建必要的目录
$(OBJDIR):
	mkdir -p $(OBJDIR)/$(SRCDIR)/kernel
	mkdir -p $(OBJDIR)/$(SRCDIR)/mm
	mkdir -p $(OBJDIR)/$(SRCDIR)/fs
	mkdir -p $(OBJDIR)/$(SRCDIR)/net
	mkdir -p $(OBJDIR)/$(SRCDIR)/ipc
	mkdir -p $(OBJDIR)/$(SRCDIR)/drivers
	mkdir -p $(OBJDIR)/$(ARCHDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# 编译C源文件
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编文件
$(OBJDIR)/%.o: %.S | $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

# 链接内核
$(KERNEL_ELF): $(ALL_OBJECTS) $(ARCHDIR)/kernel.ld | $(BINDIR)
	$(LD) $(LDFLAGS) -T $(ARCHDIR)/kernel.ld -o $@ $(ALL_OBJECTS)

# 生成二进制文件
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# 生成ISO镜像
$(KERNEL_ISO): $(KERNEL_BIN)
	mkdir -p iso/boot/grub
	cp $(KERNEL_BIN) iso/boot/kernel.bin
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "Micro Kernel" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ iso/

# 运行QEMU
qemu: $(KERNEL_BIN)
	qemu-system-x86_64 -kernel $(KERNEL_BIN) -m 512M -serial stdio

# 调试运行
debug: $(KERNEL_ELF)
	qemu-system-x86_64 -kernel $(KERNEL_ELF) -m 512M -serial stdio -s -S

# 反汇编
disasm: $(KERNEL_ELF)
	$(OBJDUMP) -d $< > $(BINDIR)/kernel.dis

# 生成符号表
symbols: $(KERNEL_ELF)
	$(OBJDUMP) -t $< > $(BINDIR)/kernel.sym

# 检查内核大小
size: $(KERNEL_ELF)
	size $<

# 清理
clean:
	rm -rf $(OBJDIR) $(BINDIR) iso/

# 深度清理
distclean: clean
	rm -f *~

# 帮助信息
help:
	@echo "Available targets:"
	@echo "  all       - Build kernel ELF and binary"
	@echo "  qemu      - Run kernel in QEMU"
	@echo "  debug     - Run kernel in QEMU with GDB support"
	@echo "  disasm    - Generate disassembly"
	@echo "  symbols   - Generate symbol table"
	@echo "  size      - Show kernel size"
	@echo "  clean     - Clean build files"
	@echo "  distclean - Deep clean"
	@echo "  help      - Show this help"

# 检查工具链
check-tools:
	@which $(CC) > /dev/null || (echo "$(CC) not found" && exit 1)
	@which $(LD) > /dev/null || (echo "$(LD) not found" && exit 1)
	@which $(AS) > /dev/null || (echo "$(AS) not found" && exit 1)
	@which $(OBJCOPY) > /dev/null || (echo "$(OBJCOPY) not found" && exit 1)
	@which $(OBJDUMP) > /dev/null || (echo "$(OBJDUMP) not found" && exit 1)
	@echo "All tools found"

# 代码统计
stats:
	@echo "Lines of code:"
	@find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
	@echo "Assembly lines:"
	@find $(ARCHDIR) -name "*.S" | xargs wc -l | tail -1

# 格式化代码
format:
	@find $(SRCDIR) $(INCDIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

# 静态分析
analyze:
	@find $(SRCDIR) -name "*.c" | xargs cppcheck --enable=all --std=c99

# 安装到USB
install-usb: $(KERNEL_BIN)
	@echo "Installing kernel to USB device..."
	@echo "WARNING: This will overwrite the USB device!"
	@read -p "USB device (e.g., /dev/sdb): " USB_DEV && \
	sudo dd if=$(KERNEL_BIN) of=$$USB_DEV bs=512 && \
	sudo sync && \
	echo "Kernel installed to $$USB_DEV"

# 测试套件
test: $(KERNEL_ELF)
	@echo "Running kernel tests..."
	@echo "Test 1: Check kernel size"
	@test `stat -c%s $(KERNEL_BIN)` -lt 1048576 || (echo "Kernel too large" && exit 1)
	@echo "Test 2: Check symbols"
	@$(OBJDUMP) -t $(KERNEL_ELF) | grep -q "kernel_main" || (echo "Missing kernel_main" && exit 1)
	@echo "Test 3: Check sections"
	@$(OBJDUMP) -h $(KERNEL_ELF) | grep -q ".text" || (echo "Missing .text section" && exit 1)
	@echo "All tests passed"

# 创建发布包
release: $(KERNEL_BIN) $(KERNEL_ISO)
	@mkdir -p release
	@cp $(KERNEL_BIN) release/
	@cp $(KERNEL_ISO) release/
	@cp README.md release/
	@tar -czf release/microkernel-$(shell date +%Y%m%d).tar.gz release/
	@echo "Release package created in release/"

# 依赖关系检查
depends:
	@echo "Checking dependencies..."
	@echo "Required packages:"
	@echo "  - gcc (cross-compiler for x86_64)"
	@echo "  - nasm (assembler)"
	@echo "  - binutils (linker, objcopy, objdump)"
	@echo "  - qemu-system-x86_64 (emulator)"
	@echo "  - grub-mkrescue (for ISO creation)"
	@echo "  - xorriso (ISO creation dependency)"

# 配置检查
config:
	@echo "Kernel configuration:"
	@echo "  Architecture: $(ARCH)"
	@echo "  Compiler: $(CC)"
	@echo "  CFLAGS: $(CFLAGS)"
	@echo "  LDFLAGS: $(LDFLAGS)"

# 伪目标
.PHONY: all clean distclean help check-tools stats format analyze install-usb test release depends config qemu debug disasm symbols size
