TARGET = OceanOS
BUILD_DIR = build
EFI_FILE = $(BUILD_DIR)/BOOTX64.EFI

KERNEL_SRC = kernel/main.c
KERNEL_OBJ = build/kernel.elf.o
KERNEL_ELF = build/kernel.elf

CC = x86_64-w64-mingw32-gcc
CFLAGS = -Ilibs -I/usr/include/efi -I/usr/include/efi/x86_64 \
         -ffreestanding -fno-stack-protector -O2 -Wall
LDFLAGS = -nostdlib -Wl,--entry,efi_main -Wl,--subsystem,10

KERNEL_CC = x86_64-elf-gcc
KERNEL_LD = x86_64-elf-ld
KERNEL_CFLAGS = -ffreestanding -mno-red-zone -O2 -Wall -Ikernel/include

SRC := $(shell find bootloader kernel libs -name "*.c")
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(EFI_FILE) $(KERNEL_ELF)

$(EFI_FILE): $(OBJ)
	@echo "Linking $@"
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(KERNEL_SRC)
	@echo "Building kernel.elf"
	@mkdir -p $(dir $@)
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c $(KERNEL_SRC) -o $(KERNEL_OBJ)
	$(KERNEL_LD) -T kernel/link.ld $(KERNEL_OBJ) -o $(KERNEL_ELF)

clean:
	rm -rf $(BUILD_DIR)/*.efi $(KERNEL_ELF) $(KERNEL_OBJ) $(OBJ)

.PHONY: all clean
