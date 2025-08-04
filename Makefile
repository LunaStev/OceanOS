# OceanOS Makefile
TARGET = OceanOS
BUILD_DIR = build
EFI_FILE = $(BUILD_DIR)/BOOTX64.EFI
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

# Compilers and tools
CC = x86_64-w64-mingw32-gcc
KERNEL_CC = x86_64-elf-gcc
KERNEL_LD = x86_64-elf-ld
KERNEL_AS = x86_64-elf-as
OBJCOPY = x86_64-elf-objcopy

# Compiler flags
CFLAGS = -Ilibs -I/usr/include/efi -I/usr/include/efi/x86_64 \
         -ffreestanding -fno-stack-protector -O2 -Wall -std=c99
LDFLAGS = -nostdlib -Wl,--entry,efi_main -Wl,--subsystem,10

KERNEL_CFLAGS = -ffreestanding -mno-red-zone -O2 -Wall -std=c99 \
                -Ikernel -Ilibs -fno-builtin -fno-pie -no-pie
KERNEL_LDFLAGS = -nostdlib -T kernel/link.ld -z noexecstack

# Source files
BOOTLOADER_SRC = $(shell find bootloader -name "*.c")
KERNEL_SRC = $(shell find kernel -name "*.c")
LIBS_SRC = $(shell find libs -name "*.c")
USERLAND_SRC = $(shell find userland -name "*.c")

# Object files
BOOTLOADER_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(BOOTLOADER_SRC))
KERNEL_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_SRC))
LIBS_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(LIBS_SRC))
USERLAND_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(USERLAND_SRC))

# Assembly files
KERNEL_ASM = $(shell find kernel -name "*.s")
KERNEL_ASM_OBJ = $(patsubst %.s,$(BUILD_DIR)/%.o,$(KERNEL_ASM))

# All object files for kernel
ALL_KERNEL_OBJ = $(KERNEL_OBJ) $(LIBS_OBJ) $(USERLAND_OBJ) $(KERNEL_ASM_OBJ)

# Default target
all: $(EFI_FILE) $(KERNEL_ELF)

# Build UEFI bootloader
$(EFI_FILE): $(BOOTLOADER_OBJ) $(LIBS_OBJ)
	@echo "Linking UEFI bootloader..."
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

# Build kernel
$(KERNEL_ELF): $(ALL_KERNEL_OBJ)
	@echo "Linking kernel..."
	@mkdir -p $(dir $@)
	$(KERNEL_LD) $(KERNEL_LDFLAGS) $^ -o $@

# Compile C files for bootloader
$(BUILD_DIR)/bootloader/%.o: bootloader/%.c
	@echo "Compiling bootloader: $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C files for kernel
$(BUILD_DIR)/kernel/%.o: kernel/%.c
	@echo "Compiling kernel: $<"
	@mkdir -p $(dir $@)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

# Compile C files for libs
$(BUILD_DIR)/libs/%.o: libs/%.c
	@echo "Compiling libs: $<"
	@mkdir -p $(dir $@)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

# Compile C files for userland
$(BUILD_DIR)/userland/%.o: userland/%.c
	@echo "Compiling userland: $<"
	@mkdir -p $(dir $@)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $< -o $@

# Compile assembly files
$(BUILD_DIR)/kernel/%.o: kernel/%.s
	@echo "Assembling: $<"
	@mkdir -p $(dir $@)
	$(KERNEL_AS) $< -o $@

# Create ISO image
iso: $(EFI_FILE) $(KERNEL_ELF)
	@echo "Creating ISO image..."
	@mkdir -p $(BUILD_DIR)/iso/EFI/BOOT
	@cp $(EFI_FILE) $(BUILD_DIR)/iso/EFI/BOOT/
	@cp $(KERNEL_ELF) $(BUILD_DIR)/iso/
	@xorriso -as mkisofs -o $(BUILD_DIR)/$(TARGET).iso -b EFI/BOOT/BOOTX64.EFI \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot --bootable $(BUILD_DIR)/iso/

# Run in QEMU
qemu: $(EFI_FILE) $(KERNEL_ELF)
	@echo "Starting QEMU..."
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd \
		-drive file=$(BUILD_DIR)/$(TARGET).iso,format=raw,media=cdrom \
		-m 512M -smp 2 -serial stdio

# Run in QEMU with UEFI
qemu-uefi: $(EFI_FILE) $(KERNEL_ELF)
	@echo "Starting QEMU with UEFI..."
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd \
		-drive file=$(BUILD_DIR)/$(TARGET).iso,format=raw,media=cdrom \
		-m 512M -smp 2 -serial stdio -vga std

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: KERNEL_CFLAGS += -g -DDEBUG
debug: all

# Release build
release: CFLAGS += -DNDEBUG
release: KERNEL_CFLAGS += -DNDEBUG
release: all

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Install dependencies (Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential gcc-multilib gcc-mingw-w64 \
		binutils-mingw-w64 xorriso ovmf qemu-system-x86

# Install dependencies (Arch Linux)
install-deps-arch:
	@echo "Installing dependencies..."
	sudo pacman -S --needed base-devel mingw-w64-gcc mingw-w64-binutils \
		xorriso edk2-ovmf qemu

# Install dependencies (macOS)
install-deps-macos:
	@echo "Installing dependencies..."
	brew install gcc make xorriso qemu

# Check if required tools are available
check-tools:
	@echo "Checking required tools..."
	@which $(CC) > /dev/null || (echo "Error: $(CC) not found. Install mingw-w64-gcc." && exit 1)
	@which $(KERNEL_CC) > /dev/null || (echo "Error: $(KERNEL_CC) not found. Install gcc-multilib." && exit 1)
	@which $(KERNEL_LD) > /dev/null || (echo "Error: $(KERNEL_LD) not found. Install binutils." && exit 1)
	@which xorriso > /dev/null || (echo "Error: xorriso not found. Install xorriso." && exit 1)
	@echo "All required tools are available."

# Show help
help:
	@echo "Available targets:"
	@echo "  all          - Build bootloader and kernel"
	@echo "  iso          - Create bootable ISO image"
	@echo "  qemu         - Run in QEMU"
	@echo "  qemu-uefi    - Run in QEMU with UEFI"
	@echo "  debug        - Build with debug symbols"
	@echo "  release      - Build optimized release version"
	@echo "  clean        - Clean build directory"
	@echo "  install-deps - Install dependencies (Ubuntu/Debian)"
	@echo "  check-tools  - Check if required tools are available"
	@echo "  help         - Show this help message"

# Phony targets
.PHONY: all iso qemu qemu-uefi debug release clean install-deps install-deps-arch install-deps-macos check-tools help
