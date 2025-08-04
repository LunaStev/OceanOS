#ifndef OCEAN_MEMORY_H
#define OCEAN_MEMORY_H

#include "../../libs/types.h"
#include "../kernel.h"

// Memory management constants
#define MEMORY_START_ADDRESS    0x100000  // 1MB
#define MEMORY_END_ADDRESS      0x1000000 // 16MB
#define PAGE_SIZE               4096
#define PAGE_TABLE_SIZE         1024
#define PAGE_DIRECTORY_SIZE     1024

// Memory allocation types
typedef enum {
    MEMORY_TYPE_FREE,
    MEMORY_TYPE_USED,
    MEMORY_TYPE_RESERVED,
    MEMORY_TYPE_KERNEL,
    MEMORY_TYPE_USER
} MemoryType;

// Memory block structure
typedef struct MemoryBlock {
    uint64_t start_address;
    uint64_t size;
    MemoryType type;
    struct MemoryBlock* next;
    struct MemoryBlock* prev;
} MemoryBlock;

// Page table entry
typedef struct {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t frame_address : 20;
} PageTableEntry;

// Page directory entry
typedef struct {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t table_address : 20;
} PageDirectoryEntry;

// Memory management functions
void memory_init(void);
void* memory_allocate(size_t size);
void memory_free(void* ptr);
void* memory_allocate_pages(uint32_t num_pages);
void memory_free_pages(void* ptr, uint32_t num_pages);

// Paging functions
void paging_init(void);
void paging_enable(void);
void paging_disable(void);
bool paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint32_t flags);
bool paging_unmap_page(uint64_t virtual_address);
void paging_switch_directory(uint32_t* page_directory);

// Memory mapping functions
void memory_map_region(uint64_t start, uint64_t end, MemoryType type);
MemoryBlock* memory_find_block(uint64_t address);
void memory_merge_blocks(void);

// Memory information functions
uint64_t memory_get_total_size(void);
uint64_t memory_get_free_size(void);
uint64_t memory_get_used_size(void);
void memory_print_info(void);

// Memory protection functions
void memory_protect_region(uint64_t start, uint64_t end, bool read_only);
void memory_make_executable(uint64_t start, uint64_t end);

// Memory copy functions
void memory_copy(void* dest, const void* src, size_t size);
void memory_set(void* dest, uint8_t value, size_t size);
int memory_compare(const void* ptr1, const void* ptr2, size_t size);

// Memory alignment functions
void* memory_align(void* ptr, size_t alignment);
size_t memory_align_size(size_t size, size_t alignment);

// Memory pool functions
typedef struct MemoryPool {
    void* start;
    size_t size;
    size_t used;
    struct MemoryPool* next;
} MemoryPool;

MemoryPool* memory_pool_create(size_t size);
void* memory_pool_allocate(MemoryPool* pool, size_t size);
void memory_pool_free(MemoryPool* pool, void* ptr);
void memory_pool_destroy(MemoryPool* pool);

#endif // OCEAN_MEMORY_H