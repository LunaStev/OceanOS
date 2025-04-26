// OceanOS
// Kernel Drivers Storage
// sdcard.c
#include "sdcard.h"
#include "spi.h"
#include "display.h"  // (print()를 위해)

#define CMD_FLAG 0x40

static uint8_t sdcard_send_command(uint8_t cmd, uint32_t arg, uint8_t crc) {
    uint8_t response;
    
    spi_send(CMD_FLAG | cmd);        // 명령
    spi_send((arg >> 24) & 0xFF);     // 인자 4바이트
    spi_send((arg >> 16) & 0xFF);
    spi_send((arg >> 8) & 0xFF);
    spi_send(arg & 0xFF);
    spi_send(crc);                   // CRC (초기화 명령 제외하면 대충 보내도 됨)

    for (int i = 0; i < 8; i++) {     // 최대 8번 대기
        response = spi_recv();
        if ((response & 0x80) == 0) { // 최상위 비트가 0이면 정상
            return response;
        }
    }
    return 0xFF; // 오류
}

static int sdcard_wait_ready() {
    uint8_t token;
    for (int i = 0; i < 50000; i++) {
        token = spi_recv();
        if (token == 0xFF)
            return 0;
    }
    return -1; // timeout
}

static int sdcard_read_data(uint8_t* buffer, uint16_t length) {
    uint8_t token;
    for (int i = 0; i < 10000; i++) {
        token = spi_recv();
        if (token == 0xFE) // Start block token
            break;
    }
    if (token != 0xFE) return -1; // Read error

    for (int i = 0; i < length; i++) {
        buffer[i] = spi_recv();
    }

    spi_recv(); // CRC 버림
    spi_recv();

    return 0;
}

static int sdcard_send_data(const uint8_t* buffer, uint8_t token) {
    spi_send(token);

    for (int i = 0; i < 512; i++) {
        spi_send(buffer[i]);
    }

    spi_send(0xFF); // CRC (dummy)
    spi_send(0xFF);

    uint8_t resp = spi_recv();
    if ((resp & 0x1F) != 0x05) {
        return -1; // data rejected
    }

    return 0;
}

int sdcard_init(sdcard_t* card) {
    card->initialized = 0;

    // SD 카드 파워온 후 80 클럭 보내기
    for (int i = 0; i < 10; i++) {
        spi_send(0xFF);
    }

    // CMD0: reset
    if (sdcard_send_command(CMD0, 0, 0x95) != R1_IDLE_STATE) {
        print("SD CMD0 failed.\n");
        return -1;
    }

    // CMD8: voltage check
    uint8_t resp = sdcard_send_command(CMD8, 0x1AA, 0x87);
    if (resp == R1_IDLE_STATE) {
        spi_recv(); // echo back R7 response
        spi_recv();
        spi_recv();
        spi_recv();
    } else {
        print("SD CMD8 failed or old card.\n");
        return -1;
    }

    // ACMD41: initialize (loop)
    while (1) {
        sdcard_send_command(CMD55, 0, 0x01);
        resp = sdcard_send_command(ACMD41, 0x40000000, 0x01);
        if (resp == R1_READY_STATE) break;
    }

    print("SD card initialized.\n");
    card->initialized = 1;
    return 0;
}

int sdcard_read_block(sdcard_t* card, uint32_t lba, uint8_t* buffer) {
    if (!card->initialized) return -1;

    if (sdcard_send_command(CMD17, lba * SECTOR_SIZE, 0x01) != 0) {
        print("SD CMD17 failed.\n");
        return -1;
    }

    if (sdcard_read_data(buffer, SECTOR_SIZE) != 0) {
        print("SD read block failed.\n");
        return -1;
    }

    sdcard_wait_ready();
    return 0;
}

int sdcard_write_block(sdcard_t* card, uint32_t lba, const uint8_t* buffer) {
    if (!card->initialized) return -1;

    if (sdcard_send_command(CMD24, lba * SECTOR_SIZE, 0x01) != 0) {
        print("SD CMD24 failed.\n");
        return -1;
    }

    if (sdcard_send_data(buffer, 0xFE) != 0) {
        print("SD write block failed.\n");
        return -1;
    }

    sdcard_wait_ready();
    return 0;
}

int sdcard_read_blocks(sdcard_t* card, uint32_t lba, uint32_t count, uint8_t* buffer) {
    for (uint32_t i = 0; i < count; i++) {
        if (sdcard_read_block(card, lba + i, buffer + (i * SECTOR_SIZE)) != 0)
            return -1;
    }
    return 0;
}

int sdcard_write_blocks(sdcard_t* card, uint32_t lba, uint32_t count, const uint8_t* buffer) {
    for (uint32_t i = 0; i < count; i++) {
        if (sdcard_write_block(card, lba + i, buffer + (i * SECTOR_SIZE)) != 0)
            return -1;
    }
    return 0;
}
