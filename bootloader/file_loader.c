// OceanOS
// Bootloader
// file_loader.c

#include <efi.h>
#include <efilib.h>
#include "elf64.h"

EFI_GUID gEfiFileInfoGuid = { 0x09576e92, 0x6d3f, 0x11d2,
                              { 0x8e, 0x39, 0x00, 0xa0, 0xc9,
                                0x69, 0x72, 0x3b } };


EFI_STATUS LoadKernel(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, void** kernel_entry) {
    EFI_LOADED_IMAGE* loaded_image;
    EFI_GUID loaded_image_protocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fs_protocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE* file_system;
    EFI_FILE_HANDLE root_dir, kernel_file;
    EFI_STATUS status;

    status = SystemTable->BootServices->HandleProtocol(
        ImageHandle, &loaded_image_protocol, (void**)&loaded_image);
    if (EFI_ERROR(status)) return status;

    status = SystemTable->BootServices->HandleProtocol(
        loaded_image->DeviceHandle, &fs_protocol, (void**)&file_system);
    if (EFI_ERROR(status)) return status;

    status = file_system->OpenVolume(file_system, &root_dir);
    if (EFI_ERROR(status)) return status;

    status = root_dir->Open(root_dir, &kernel_file, L"kernel.elf", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) return status;

    EFI_FILE_INFO* file_info;
    UINTN info_size = sizeof(EFI_FILE_INFO) + 1024;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, info_size, (void**)&file_info);
    kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &info_size, file_info);

    UINTN kernel_size = file_info->FileSize;
    void* kernel_buffer;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, kernel_size, &kernel_buffer);
    kernel_file->Read(kernel_file, &kernel_size, kernel_buffer);

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)kernel_buffer;
    Elf64_Phdr* phdrs = (Elf64_Phdr*)((uint8_t*)kernel_buffer + ehdr->e_phoff);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr* ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) continue;

        UINTN pages = (ph->p_memsz + 0xFFF) / 0x1000;
        EFI_PHYSICAL_ADDRESS segment_addr = ph->p_paddr;

        SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment_addr);
        void* src = (uint8_t*)kernel_buffer + ph->p_offset;
        void* dst = (void*)(uint64_t)segment_addr;

        for (uint64_t b = 0; b < ph->p_filesz; b++) {
            ((uint8_t*)dst)[b] = ((uint8_t*)src)[b];
        }

        for (uint64_t b = ph->p_filesz; b < ph->p_memsz; b++) {
            ((uint8_t*)dst)[b] = 0;
        }
    }

    *kernel_entry = (void*)(uintptr_t)ehdr->e_entry;
    return EFI_SUCCESS;
}
