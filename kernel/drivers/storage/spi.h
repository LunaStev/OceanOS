#ifndef SPI_H
#define SPI_H

#include <stdint.h>

// SPI 초기화
void spi_init();

// SPI로 1바이트 보내기
void spi_send(uint8_t data);

// SPI로 1바이트 받기
uint8_t spi_recv();

#endif // SPI_H
