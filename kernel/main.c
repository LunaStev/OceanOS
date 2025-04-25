#include <stdint.h>
#include "../bootloader/framebuffer.h"

void kernel_main(FramebufferInfo* fb) {
    volatile uint32_t* fb_ptr = (uint32_t*)fb->base;

    // 테스트: 주소 확인 (픽셀 하나만 찍기)
    fb_ptr[0] = 0x00FF0000;  // 빨간 점

    // 블럭 출력
    for (uint32_t y = 0; y < 100; y++) {
        for (uint32_t x = 0; x < 200; x++) {
            fb_ptr[y * fb->pixels_per_scanline + x] = 0x00FFFFFF;
        }
    }

    // 무한 루프
    while (1) {}
}