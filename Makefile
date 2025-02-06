BOOTLOADER_SRC = boot/bootloader.asm
BOOTLOADER_BIN = boot/bootloader.bin
BOOTLOADER_IMG = boot/bootloader.img

all: $(BOOTLOADER_IMG)

$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	nasm -f bin $(BOOTLOADER_SRC) -o $(BOOTLOADER_BIN)

$(BOOTLOADER_IMG): $(BOOTLOADER_BIN)
	dd if=$(BOOTLOADER_BIN) of=$(BOOTLOADER_IMG) bs=512 count=1 conv=notrunc

.PHONY: clean
clean:
	rm -f $(BOOTLOADER_BIN) $(BOOTLOADER_IMG)
