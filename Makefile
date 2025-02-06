BOOTLOADER_SRC = boot/bootloader.asm
BOOTLOADER_BIN = boot/bootloader.bin
OS_IMAGE = OceanOS.img  # 출력 파일 이름 수정

all: $(OS_IMAGE)

$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	nasm -f bin $(BOOTLOADER_SRC) -o $(BOOTLOADER_BIN)

$(OS_IMAGE): $(BOOTLOADER_BIN)
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880   # OceanOS.img로 출력 파일 수정
	dd if=$(BOOTLOADER_BIN) of=$(OS_IMAGE) bs=512 count=1 conv=notrunc

.PHONY: clean
clean:
	rm -f $(BOOTLOADER_BIN) $(OS_IMAGE)