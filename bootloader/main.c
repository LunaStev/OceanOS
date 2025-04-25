#include "framebuffer.h"

typedef void (*KernelEntry)(FramebufferInfo*);

void JumpToKernel(void* entry, FramebufferInfo* fb_info) {
    KernelEntry kernel = (KernelEntry)entry;
    kernel(fb_info);
}