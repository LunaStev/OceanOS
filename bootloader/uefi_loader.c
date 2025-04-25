#include <efi.h>
#include <efilib.h>
#include "framebuffer.h"

extern EFI_STATUS LoadKernel(EFI_HANDLE, EFI_SYSTEM_TABLE*, void**);
extern void JumpToKernel(void* kernel_entry, FramebufferInfo* fb_info);

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    void* kernel_entry = NULL;
    EFI_STATUS status = LoadKernel(ImageHandle, SystemTable, &kernel_entry);
    if (EFI_ERROR(status)) return status;

    // Get GOP info
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
    status = SystemTable->BootServices->LocateProtocol(&gop_guid, NULL, (void**)&gop);
    if (EFI_ERROR(status)) return status;

    FramebufferInfo fb_info;
    fb_info.base = (void*)gop->Mode->FrameBufferBase;
    fb_info.width = gop->Mode->Info->HorizontalResolution;
    fb_info.height = gop->Mode->Info->VerticalResolution;
    fb_info.pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

    // ExitBootServices
    UINTN map_size = 0, map_key, desc_size;
    UINT32 desc_ver;
    SystemTable->BootServices->GetMemoryMap(&map_size, NULL, &map_key, &desc_size, &desc_ver);
    map_size += desc_size * 8;

    EFI_MEMORY_DESCRIPTOR* mem_map;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&mem_map);
    SystemTable->BootServices->GetMemoryMap(&map_size, mem_map, &map_key, &desc_size, &desc_ver);
    SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);

    // Jump
    JumpToKernel(kernel_entry, &fb_info);
    return EFI_SUCCESS;
}