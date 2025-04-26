#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// ATA I/O ports (Primary Bus)
#define ATA_PRIMARY_IO  0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

// ATA Command Set
#define ATA_CMD_READ_PIO      0x20
#define ATA_CMD_WRITE_PIO     0x30
#define ATA_CMD_IDENTIFY      0xEC

// Functions
void ata_init();
int ata_identify();
int ata_read_sector(uint32_t lba, uint8_t* buffer);
int ata_write_sector(uint32_t lba, const uint8_t* buffer);

#endif // ATA_H
