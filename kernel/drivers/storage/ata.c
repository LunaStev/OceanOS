#include "ata.h"
#include "io.h"
#include "display.h"

#define SECTOR_SIZE 512

static void ata_wait_busy() {
    while (inb(ATA_PRIMARY_IO + 7) & 0x80) {
        // Wait while BSY (bit 7) is set
    }
}

static void ata_wait_drq() {
    while (!(inb(ATA_PRIMARY_IO + 7) & 0x08)) {
        // Wait until DRQ (bit 3) is set
    }
}

void ata_init() {
    outb(ATA_PRIMARY_CTRL, 0); // Disable IRQs (optional)
    print("ATA initialized.\n");
}

int ata_identify() {
    ata_wait_busy();

    outb(ATA_PRIMARY_IO + 6, 0xA0); // Select drive 0 (master)
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_PRIMARY_IO + 7);
    if (status == 0) {
        print("No ATA drive detected.\n");
        return -1;
    }

    ata_wait_busy();
    ata_wait_drq();

    uint16_t buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(ATA_PRIMARY_IO);
    }

    print("ATA drive identified.\n");
    return 0;
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    ata_wait_busy();

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); // Drive/Head
    outb(ATA_PRIMARY_IO + 2, 1);                           // Sector Count = 1
    outb(ATA_PRIMARY_IO + 3, (uint8_t)(lba & 0xFF));        // LBA Low
    outb(ATA_PRIMARY_IO + 4, (uint8_t)((lba >> 8) & 0xFF)); // LBA Mid
    outb(ATA_PRIMARY_IO + 5, (uint8_t)((lba >> 16) & 0xFF));// LBA High
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_READ_PIO);             // Command

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        uint16_t data = inw(ATA_PRIMARY_IO);
        buffer[i*2] = (uint8_t)(data & 0xFF);
        buffer[i*2+1] = (uint8_t)((data >> 8) & 0xFF);
    }

    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    ata_wait_busy();

    outb(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F)); // Drive/Head
    outb(ATA_PRIMARY_IO + 2, 1);                           // Sector Count = 1
    outb(ATA_PRIMARY_IO + 3, (uint8_t)(lba & 0xFF));        // LBA Low
    outb(ATA_PRIMARY_IO + 4, (uint8_t)((lba >> 8) & 0xFF)); // LBA Mid
    outb(ATA_PRIMARY_IO + 5, (uint8_t)((lba >> 16) & 0xFF));// LBA High
    outb(ATA_PRIMARY_IO + 7, ATA_CMD_WRITE_PIO);            // Command

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < SECTOR_SIZE / 2; i++) {
        uint16_t word = buffer[i*2] | (buffer[i*2+1] << 8);
        outw(ATA_PRIMARY_IO, word);
    }

    // Flush the cache
    outb(ATA_PRIMARY_IO + 7, 0xE7); // FLUSH CACHE command

    ata_wait_busy();
    return 0;
}
