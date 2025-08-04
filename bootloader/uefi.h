#ifndef OCEAN_UEFI_H
#define OCEAN_UEFI_H

#include "../libs/types.h"

// UEFI System Table
typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
} EFI_TABLE_HEADER;

typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
} EFI_BOOT_SERVICES_TABLE_HEADER;

typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
} EFI_RUNTIME_SERVICES_TABLE_HEADER;

typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
} EFI_CONFIGURATION_TABLE;

typedef struct {
    EFI_TABLE_HEADER hdr;
    char* firmware_vendor;
    uint32_t firmware_revision;
    EFI_HANDLE console_in_handle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* con_in;
    EFI_HANDLE console_out_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* con_out;
    EFI_HANDLE standard_error_handle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* std_err;
    EFI_RUNTIME_SERVICES* runtime_services;
    EFI_BOOT_SERVICES* boot_services;
    uint64_t number_of_table_entries;
    EFI_CONFIGURATION_TABLE* configuration_table;
} EFI_SYSTEM_TABLE;

// UEFI Handle
typedef void* EFI_HANDLE;

// UEFI Status
typedef uint64_t EFI_STATUS;

#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR 0x8000000000000001
#define EFI_INVALID_PARAMETER 0x8000000000000002
#define EFI_UNSUPPORTED 0x8000000000000003
#define EFI_BAD_BUFFER_SIZE 0x8000000000000004
#define EFI_BUFFER_TOO_SMALL 0x8000000000000005
#define EFI_NOT_READY 0x8000000000000006
#define EFI_DEVICE_ERROR 0x8000000000000007
#define EFI_WRITE_PROTECTED 0x8000000000000008
#define EFI_OUT_OF_RESOURCES 0x8000000000000009
#define EFI_VOLUME_CORRUPTED 0x800000000000000A
#define EFI_VOLUME_FULL 0x800000000000000B
#define EFI_NO_MEDIA 0x800000000000000C
#define EFI_MEDIA_CHANGED 0x800000000000000D
#define EFI_NOT_FOUND 0x800000000000000E
#define EFI_ACCESS_DENIED 0x800000000000000F
#define EFI_NO_RESPONSE 0x8000000000000010
#define EFI_NO_MAPPING 0x8000000000000011
#define EFI_TIMEOUT 0x8000000000000012
#define EFI_NOT_STARTED 0x8000000000000013
#define EFI_ALREADY_STARTED 0x8000000000000014
#define EFI_ABORTED 0x8000000000000015
#define EFI_ICMP_ERROR 0x8000000000000016
#define EFI_TFTP_ERROR 0x8000000000000017
#define EFI_PROTOCOL_ERROR 0x8000000000000018
#define EFI_INCOMPATIBLE_VERSION 0x8000000000000019
#define EFI_SECURITY_VIOLATION 0x800000000000001A
#define EFI_CRC_ERROR 0x800000000000001B
#define EFI_END_OF_MEDIA 0x800000000000001C
#define EFI_END_OF_FILE 0x800000000000001D
#define EFI_INVALID_LANGUAGE 0x800000000000001E
#define EFI_COMPROMISED_DATA 0x800000000000001F
#define EFI_IP_ADDRESS_CONFLICT 0x8000000000000020
#define EFI_HTTP_ERROR 0x8000000000000021

// UEFI Memory Types
#define EFI_RESERVED_MEMORY_TYPE 0
#define EFI_LOADER_CODE 1
#define EFI_LOADER_DATA 2
#define EFI_BOOT_SERVICES_CODE 3
#define EFI_BOOT_SERVICES_DATA 4
#define EFI_RUNTIME_SERVICES_CODE 5
#define EFI_RUNTIME_SERVICES_DATA 6
#define EFI_CONVENTIONAL_MEMORY 7
#define EFI_UNUSABLE_MEMORY 8
#define EFI_ACPI_RECLAIM_MEMORY 9
#define EFI_ACPI_MEMORY_NVS 10
#define EFI_MEMORY_MAPPED_IO 11
#define EFI_MEMORY_MAPPED_IO_PORT_SPACE 12
#define EFI_PAL_CODE 13
#define EFI_PERSISTENT_MEMORY 14
#define EFI_MAX_MEMORY_TYPE 15

// UEFI Memory Descriptor
typedef struct {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} EFI_MEMORY_DESCRIPTOR;

// UEFI Graphics Output Protocol
typedef struct {
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t reserved_mask;
} EFI_PIXEL_BITMASK;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    EFI_GRAPHICS_PIXEL_FORMAT pixel_format;
    EFI_PIXEL_BITMASK pixel_information;
    uint32_t pixels_per_scan_line;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    uint32_t max_mode;
    uint32_t mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
    size_t size_of_info;
    uint64_t frame_buffer_base;
    size_t frame_buffer_size;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct {
    uint64_t query_mode;
    uint64_t set_mode;
    uint64_t blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// UEFI Simple Text Input Protocol
typedef struct {
    uint16_t scan_code;
    uint16_t unicode_char;
} EFI_INPUT_KEY;

typedef struct {
    uint64_t reset;
    uint64_t read_key_stroke;
    EFI_EVENT wait_for_key;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// UEFI Simple Text Output Protocol
typedef struct {
    int32_t max_mode;
    int32_t mode;
    int32_t attribute;
    int32_t cursor_column;
    int32_t cursor_row;
    bool cursor_visible;
} EFI_SIMPLE_TEXT_OUTPUT_MODE;

typedef struct {
    uint64_t reset;
    uint64_t output_string;
    uint64_t test_string;
    uint64_t query_mode;
    uint64_t set_mode;
    uint64_t set_attribute;
    uint64_t clear_screen;
    uint64_t set_cursor_position;
    uint64_t enable_cursor;
    EFI_SIMPLE_TEXT_OUTPUT_MODE* mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// UEFI Boot Services
typedef struct {
    EFI_BOOT_SERVICES_TABLE_HEADER hdr;
    uint64_t raise_tpl;
    uint64_t restore_tpl;
    uint64_t allocate_pages;
    uint64_t free_pages;
    uint64_t get_memory_map;
    uint64_t allocate_pool;
    uint64_t free_pool;
    uint64_t create_event;
    uint64_t set_timer;
    uint64_t wait_for_event;
    uint64_t signal_event;
    uint64_t close_event;
    uint64_t check_event;
    uint64_t install_protocol_interface;
    uint64_t reinstall_protocol_interface;
    uint64_t uninstall_protocol_interface;
    uint64_t handle_protocol;
    uint64_t reserved;
    uint64_t register_protocol_notify;
    uint64_t locate_handle;
    uint64_t locate_device_path;
    uint64_t install_configuration_table;
    uint64_t load_image;
    uint64_t start_image;
    uint64_t exit;
    uint64_t unload_image;
    uint64_t exit_boot_services;
    uint64_t get_next_monotonic_count;
    uint64_t stall;
    uint64_t set_watchdog_timer;
    uint64_t connect_controller;
    uint64_t disconnect_controller;
    uint64_t open_protocol;
    uint64_t close_protocol;
    uint64_t open_protocol_information;
    uint64_t protocols_per_handle;
    uint64_t locate_handle_buffer;
    uint64_t locate_protocol;
    uint64_t install_multiple_protocol_interfaces;
    uint64_t uninstall_multiple_protocol_interfaces;
    uint64_t calculate_crc32;
    uint64_t copy_mem;
    uint64_t set_mem;
    uint64_t create_event_ex;
} EFI_BOOT_SERVICES;

// UEFI Runtime Services
typedef struct {
    EFI_RUNTIME_SERVICES_TABLE_HEADER hdr;
    uint64_t get_time;
    uint64_t set_time;
    uint64_t get_wakeup_time;
    uint64_t set_wakeup_time;
    uint64_t set_virtual_address_map;
    uint64_t convert_pointer;
    uint64_t get_variable;
    uint64_t get_next_variable_name;
    uint64_t set_variable;
    uint64_t get_next_high_monotonic_count;
    uint64_t reset_system;
    uint64_t update_capsule;
    uint64_t query_capsule_capabilities;
    uint64_t query_variable_info;
} EFI_RUNTIME_SERVICES;

// UEFI Event
typedef void* EFI_EVENT;

// UEFI TPL (Task Priority Level)
typedef uint64_t EFI_TPL;

#define TPL_APPLICATION 4
#define TPL_CALLBACK 8
#define TPL_NOTIFY 16
#define TPL_HIGH_LEVEL 31

#endif // OCEAN_UEFI_H 