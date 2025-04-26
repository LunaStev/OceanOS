#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

// SD Card Command Set (SPI Mode)
#define CMD0   0   // GO_IDLE_STATE
#define CMD8   8   // SEND_IF_COND
#define CMD17 17   // READ_SINGLE_BLOCK
#define CMD18 18   // READ_MULTIPLE_BLOCK
#define CMD24 24   // WRITE_SINGLE_BLOCK
#define CMD25 25   // WRITE_MULTIPLE_BLOCK
#define CMD55 55   // APP_CMD
#define ACMD41 41  // SD_SEND_OP_COND (after CMD55)

// SD Card response types
#define R1_IDLE_STATE    0x01
#define R1_READY_STATE   0x00

// Sector size (standard for SD)
#define SECTOR_SIZE 512

// SD Card driver state
typedef struct {
    int initialized;
} sdcard_t;

// Functions
int sdcard_init(sdcard_t* card);
int sdcard_read_block(sdcard_t* card, uint32_t lba, uint8_t* buffer);
int sdcard_write_block(sdcard_t* card, uint32_t lba, const uint8_t* buffer);
int sdcard_read_blocks(sdcard_t* card, uint32_t lba, uint32_t count, uint8_t* buffer);
int sdcard_write_blocks(sdcard_t* card, uint32_t lba, uint32_t count, const uint8_t* buffer);

#endif // SDCARD_H
