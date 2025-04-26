#include "spi.h"
#include "io.h"

#define SPI_PORT_BASE 0x1F0 // 예시: (진짜는 네가 사용하는 SPI 컨트롤러 주소에 따라 다름)

// SPI 포트 레지스터 (예시용)
#define SPI_REG_DATA    (SPI_PORT_BASE + 0)
#define SPI_REG_STATUS  (SPI_PORT_BASE + 4)
#define SPI_REG_CONTROL (SPI_PORT_BASE + 8)

// 간단 SPI 초기화 (제어 레지스터 설정)
void spi_init() {
    outb(SPI_REG_CONTROL, 0x50); // 예: Master Mode, Clock Polarity=0, Phase=0
}

// SPI로 1바이트 보내기
void spi_send(uint8_t data) {
    outb(SPI_REG_DATA, data);

    // 송신 완료될 때까지 기다리기
    while (!(inb(SPI_REG_STATUS) & (1 << 0))) {
        // 대기 (Bit 0 == Transfer Complete)
    }
}

// SPI로 1바이트 받기
uint8_t spi_recv() {
    // dummy write to start clock
    outb(SPI_REG_DATA, 0xFF);

    // 수신 완료될 때까지 기다리기
    while (!(inb(SPI_REG_STATUS) & (1 << 0))) {
        // 대기
    }

    return inb(SPI_REG_DATA);
}
