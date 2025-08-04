#include "uefi.h"
#include "framebuffer.h"
#include "../libs/stdio.h"

// Global variables
EFI_SYSTEM_TABLE* g_st = NULL;
EFI_BOOT_SERVICES* g_bs = NULL;
EFI_GRAPHICS_OUTPUT_PROTOCOL* g_gop = NULL;

// Kernel entry point
typedef void (*KernelEntry)(FramebufferInfo*);

// Function prototypes
EFI_STATUS initialize_graphics(void);
EFI_STATUS load_kernel(void);
void jump_to_kernel(void* entry, FramebufferInfo* fb_info);
EFI_STATUS get_memory_map(EFI_MEMORY_DESCRIPTOR** map, uint64_t* map_size, uint64_t* map_key, uint64_t* descriptor_size, uint32_t* descriptor_version);

// Main entry point
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    EFI_STATUS status;
    
    // Store global references
    g_st = system_table;
    g_bs = system_table->boot_services;
    
    // Initialize console
    console_init();
    console_puts("OceanOS Bootloader Starting...\n");
    
    // Initialize graphics
    status = initialize_graphics();
    if (EFI_ERROR(status)) {
        console_puts("Failed to initialize graphics\n");
        return status;
    }
    
    // Load kernel
    status = load_kernel();
    if (EFI_ERROR(status)) {
        console_puts("Failed to load kernel\n");
        return status;
    }
    
    console_puts("Bootloader completed successfully\n");
    return EFI_SUCCESS;
}

EFI_STATUS initialize_graphics(void) {
    EFI_STATUS status;
    EFI_HANDLE* handles = NULL;
    uint64_t num_handles = 0;
    
    // Locate Graphics Output Protocol
    status = g_bs->locate_handle_buffer(ByProtocol, 
                                       &(EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
                                       NULL, &num_handles, &handles);
    if (EFI_ERROR(status)) {
        console_puts("No Graphics Output Protocol found\n");
        return status;
    }
    
    // Get the first GOP handle
    status = g_bs->handle_protocol(handles[0], 
                                  &(EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
                                  (void**)&g_gop);
    if (EFI_ERROR(status)) {
        console_puts("Failed to get Graphics Output Protocol\n");
        return status;
    }
    
    // Set to highest resolution
    uint32_t max_mode = g_gop->mode->max_mode;
    uint32_t best_mode = 0;
    uint32_t best_width = 0;
    uint32_t best_height = 0;
    
    for (uint32_t i = 0; i < max_mode; i++) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
        size_t info_size;
        
        status = g_gop->query_mode(g_gop, i, &info_size, &info);
        if (EFI_ERROR(status)) continue;
        
        if (info->horizontal_resolution >= best_width && 
            info->vertical_resolution >= best_height) {
            best_width = info->horizontal_resolution;
            best_height = info->vertical_resolution;
            best_mode = i;
        }
    }
    
    // Set the best mode
    status = g_gop->set_mode(g_gop, best_mode);
    if (EFI_ERROR(status)) {
        console_puts("Failed to set graphics mode\n");
        return status;
    }
    
    console_printf("Graphics initialized: %dx%d\n", best_width, best_height);
    return EFI_SUCCESS;
}

EFI_STATUS load_kernel(void) {
    EFI_STATUS status;
    EFI_FILE_PROTOCOL* root_dir = NULL;
    EFI_FILE_PROTOCOL* kernel_file = NULL;
    uint64_t kernel_size = 0;
    void* kernel_buffer = NULL;
    
    // Open kernel file
    status = g_bs->locate_protocol(&(EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
                                  NULL, (void**)&root_dir);
    if (EFI_ERROR(status)) {
        console_puts("No file system found\n");
        return status;
    }
    
    status = root_dir->open(root_dir, &kernel_file, L"kernel.elf",
                           EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        console_puts("Failed to open kernel.elf\n");
        return status;
    }
    
    // Get file size
    EFI_FILE_INFO* file_info = NULL;
    uint64_t info_size = 0;
    
    status = kernel_file->get_info(kernel_file, &(EFI_GUID)EFI_FILE_INFO_GUID,
                                  &info_size, file_info);
    if (EFI_ERROR(status)) {
        console_puts("Failed to get file info\n");
        return status;
    }
    
    file_info = g_bs->allocate_pool(EFI_LOADER_DATA, info_size);
    status = kernel_file->get_info(kernel_file, &(EFI_GUID)EFI_FILE_INFO_GUID,
                                  &info_size, file_info);
    if (EFI_ERROR(status)) {
        console_puts("Failed to get file info\n");
        return status;
    }
    
    kernel_size = file_info->file_size;
    g_bs->free_pool(file_info);
    
    // Allocate memory for kernel
    status = g_bs->allocate_pages(AllocateAnyPages, EFI_LOADER_DATA,
                                 (kernel_size + 0xFFF) >> 12, (uint64_t*)&kernel_buffer);
    if (EFI_ERROR(status)) {
        console_puts("Failed to allocate kernel memory\n");
        return status;
    }
    
    // Read kernel file
    status = kernel_file->read(kernel_file, &kernel_size, kernel_buffer);
    if (EFI_ERROR(status)) {
        console_puts("Failed to read kernel file\n");
        return status;
    }
    
    kernel_file->close(kernel_file);
    
    console_printf("Kernel loaded at 0x%llx, size: %llu bytes\n", 
                   (uint64_t)kernel_buffer, kernel_size);
    
    // Prepare framebuffer info
    FramebufferInfo fb_info;
    fb_info.base_address = g_gop->mode->frame_buffer_base;
    fb_info.buffer_size = g_gop->mode->frame_buffer_size;
    fb_info.width = g_gop->mode->info->horizontal_resolution;
    fb_info.height = g_gop->mode->info->vertical_resolution;
    fb_info.pixels_per_scan_line = g_gop->mode->info->pixels_per_scan_line;
    fb_info.bits_per_pixel = 32; // Assuming 32-bit color
    
    // Exit boot services
    EFI_MEMORY_DESCRIPTOR* memory_map = NULL;
    uint64_t map_size = 0;
    uint64_t map_key = 0;
    uint64_t descriptor_size = 0;
    uint32_t descriptor_version = 0;
    
    status = get_memory_map(&memory_map, &map_size, &map_key, 
                           &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status)) {
        console_puts("Failed to get memory map\n");
        return status;
    }
    
    status = g_bs->exit_boot_services(image_handle, map_key);
    if (EFI_ERROR(status)) {
        console_puts("Failed to exit boot services\n");
        return status;
    }
    
    // Jump to kernel
    jump_to_kernel(kernel_buffer, &fb_info);
    
    // Should never reach here
    return EFI_SUCCESS;
}

EFI_STATUS get_memory_map(EFI_MEMORY_DESCRIPTOR** map, uint64_t* map_size, 
                         uint64_t* map_key, uint64_t* descriptor_size, 
                         uint32_t* descriptor_version) {
    EFI_STATUS status;
    uint64_t local_map_size = 0;
    uint64_t local_map_key = 0;
    uint64_t local_descriptor_size = 0;
    uint32_t local_descriptor_version = 0;
    
    // Get required size
    status = g_bs->get_memory_map(&local_map_size, *map, map_key,
                                 &local_descriptor_size, &local_descriptor_version);
    if (status != EFI_BUFFER_TOO_SMALL) {
        return status;
    }
    
    // Allocate memory for map
    status = g_bs->allocate_pool(EFI_LOADER_DATA, local_map_size, (void**)map);
    if (EFI_ERROR(status)) {
        return status;
    }
    
    // Get actual memory map
    status = g_bs->get_memory_map(&local_map_size, *map, &local_map_key,
                                 &local_descriptor_size, &local_descriptor_version);
    if (EFI_ERROR(status)) {
        g_bs->free_pool(*map);
        return status;
    }
    
    *map_size = local_map_size;
    *map_key = local_map_key;
    *descriptor_size = local_descriptor_size;
    *descriptor_version = local_descriptor_version;
    
    return EFI_SUCCESS;
}

void jump_to_kernel(void* entry, FramebufferInfo* fb_info) {
    KernelEntry kernel = (KernelEntry)entry;
    kernel(fb_info);
}