#!/bin/bash

set -e

# Paths
BUILD_DIR=build
EFI_DIR=$BUILD_DIR/iso/EFI/BOOT
DISK_IMG=$BUILD_DIR/os_disk.img

# Make directories
mkdir -p "$EFI_DIR"

# Copy files
cp $BUILD_DIR/BOOTX64.EFI $EFI_DIR/
cp $BUILD_DIR/kernel.elf $BUILD_DIR/iso/

# Create FAT disk image
rm -f "$DISK_IMG"
mkfs.vfat -n "OCEANOS" -C "$DISK_IMG" 32768
mmd -i "$DISK_IMG" ::/EFI
mmd -i "$DISK_IMG" ::/EFI/BOOT
mcopy -i "$DISK_IMG" $EFI_DIR/BOOTX64.EFI ::/EFI/BOOT/
mcopy -i "$DISK_IMG" $BUILD_DIR/iso/kernel.elf ::/

# Run QEMU with OVMF
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
  -drive format=raw,file="$DISK_IMG" \
  -m 512M
  -vga std