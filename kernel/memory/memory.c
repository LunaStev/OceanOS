#include "memory.h"
#include "../../libs/stdio.h"

// Global memory state
static MemoryBlock* memory_blocks = NULL;
static uint64_t total_memory = 0;
static uint64_t free_memory = 0;
static uint64_t used_memory = 0;

// Page directory and tables
static uint32_t* page_directory = NULL;
static uint32_t* page_tables[1024] = {0};

// Memory allocation bitmap
static uint8_t* memory_bitmap = NULL;
static uint64_t memory_bitmap_size = 0;

void memory_init(void) {
    // Initialize basic memory management
    total_memory = MEMORY_END_ADDRESS - MEMORY_START_ADDRESS;
    free_memory = total_memory;
    used_memory = 0;
    
    // Create initial memory block
    memory_blocks = (MemoryBlock*)MEMORY_START_ADDRESS;
    memory_blocks->start_address = MEMORY_START_ADDRESS + sizeof(MemoryBlock);
    memory_blocks->size = total_memory - sizeof(MemoryBlock);
    memory_blocks->type = MEMORY_TYPE_FREE;
    memory_blocks->next = NULL;
    memory_blocks->prev = NULL;
    
    // Initialize memory bitmap
    memory_bitmap_size = (total_memory / PAGE_SIZE) / 8;
    memory_bitmap = (uint8_t*)(memory_blocks->start_address);
    memory_blocks->start_address += memory_bitmap_size;
    memory_blocks->size -= memory_bitmap_size;
    
    // Clear bitmap
    memory_set(memory_bitmap, 0, memory_bitmap_size);
    
    // Reserve memory for kernel
    memory_map_region(MEMORY_START_ADDRESS, MEMORY_START_ADDRESS + 0x100000, MEMORY_TYPE_KERNEL);
    
    // Initialize paging
    paging_init();
    
    console_puts("Memory management initialized\n");
}

void* memory_allocate(size_t size) {
    if (size == 0) return NULL;
    
    // Align size to page boundary
    size = memory_align_size(size, PAGE_SIZE);
    
    // Find free block
    MemoryBlock* block = memory_blocks;
    while (block != NULL) {
        if (block->type == MEMORY_TYPE_FREE && block->size >= size) {
            // Split block if necessary
            if (block->size > size + sizeof(MemoryBlock)) {
                MemoryBlock* new_block = (MemoryBlock*)(block->start_address + size);
                new_block->start_address = block->start_address + size + sizeof(MemoryBlock);
                new_block->size = block->size - size - sizeof(MemoryBlock);
                new_block->type = MEMORY_TYPE_FREE;
                new_block->next = block->next;
                new_block->prev = block;
                
                if (block->next) {
                    block->next->prev = new_block;
                }
                block->next = new_block;
            }
            
            // Allocate from this block
            void* ptr = (void*)block->start_address;
            block->start_address += size;
            block->size -= size;
            block->type = MEMORY_TYPE_USED;
            
            // Update memory statistics
            free_memory -= size;
            used_memory += size;
            
            return ptr;
        }
        block = block->next;
    }
    
    return NULL; // Out of memory
}

void memory_free(void* ptr) {
    if (ptr == NULL) return;
    
    // Find the block containing this pointer
    MemoryBlock* block = memory_blocks;
    while (block != NULL) {
        if ((void*)block->start_address <= ptr && 
            (void*)(block->start_address + block->size) > ptr) {
            
            // Mark as free
            block->type = MEMORY_TYPE_FREE;
            
            // Update memory statistics
            used_memory -= block->size;
            free_memory += block->size;
            
            // Try to merge with adjacent free blocks
            memory_merge_blocks();
            return;
        }
        block = block->next;
    }
}

void* memory_allocate_pages(uint32_t num_pages) {
    size_t size = num_pages * PAGE_SIZE;
    return memory_allocate(size);
}

void memory_free_pages(void* ptr, uint32_t num_pages) {
    memory_free(ptr);
}

void paging_init(void) {
    // Allocate page directory
    page_directory = (uint32_t*)memory_allocate_pages(1);
    memory_set(page_directory, 0, PAGE_SIZE);
    
    // Identity map first 4MB for kernel
    for (uint32_t i = 0; i < 1024; i++) {
        uint32_t physical_address = i * PAGE_SIZE;
        paging_map_page(physical_address, physical_address, 
                       PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
    }
    
    // Map framebuffer
    FramebufferInfo* fb = kernel_get_state()->framebuffer;
    if (fb) {
        uint64_t fb_start = fb->base_address & 0xFFFFF000;
        uint64_t fb_end = (fb->base_address + fb->buffer_size + 0xFFF) & 0xFFFFF000;
        
        for (uint64_t addr = fb_start; addr < fb_end; addr += PAGE_SIZE) {
            paging_map_page(addr, addr, PAGE_PRESENT | PAGE_WRITE);
        }
    }
    
    console_puts("Paging initialized\n");
}

void paging_enable(void) {
    // Load page directory
    __asm__ volatile("mov %0, %%cr3" : : "r" (page_directory));
    
    // Enable paging
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000; // Set PG bit
    __asm__ volatile("mov %0, %%cr0" : : "r" (cr0));
    
    console_puts("Paging enabled\n");
}

void paging_disable(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 &= ~0x80000000; // Clear PG bit
    __asm__ volatile("mov %0, %%cr0" : : "r" (cr0));
    
    console_puts("Paging disabled\n");
}

bool paging_map_page(uint64_t virtual_address, uint64_t physical_address, uint32_t flags) {
    uint32_t page_dir_index = (virtual_address >> 22) & 0x3FF;
    uint32_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    // Check if page table exists
    if (!(page_directory[page_dir_index] & PAGE_PRESENT)) {
        // Create new page table
        page_tables[page_dir_index] = (uint32_t*)memory_allocate_pages(1);
        memory_set(page_tables[page_dir_index], 0, PAGE_SIZE);
        
        page_directory[page_dir_index] = (uint32_t)page_tables[page_dir_index] | 
                                       PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    // Map the page
    PageTableEntry* page_table = (PageTableEntry*)page_tables[page_dir_index];
    page_table[page_table_index].frame_address = physical_address >> 12;
    page_table[page_table_index].present = 1;
    page_table[page_table_index].read_write = (flags & PAGE_WRITE) ? 1 : 0;
    page_table[page_table_index].user_supervisor = (flags & PAGE_USER) ? 1 : 0;
    page_table[page_table_index].global = (flags & PAGE_GLOBAL) ? 1 : 0;
    
    // Invalidate TLB
    __asm__ volatile("invlpg (%0)" : : "r" (virtual_address) : "memory");
    
    return true;
}

bool paging_unmap_page(uint64_t virtual_address) {
    uint32_t page_dir_index = (virtual_address >> 22) & 0x3FF;
    uint32_t page_table_index = (virtual_address >> 12) & 0x3FF;
    
    if (!(page_directory[page_dir_index] & PAGE_PRESENT)) {
        return false;
    }
    
    PageTableEntry* page_table = (PageTableEntry*)page_tables[page_dir_index];
    page_table[page_table_index].present = 0;
    
    // Invalidate TLB
    __asm__ volatile("invlpg (%0)" : : "r" (virtual_address) : "memory");
    
    return true;
}

void memory_map_region(uint64_t start, uint64_t end, MemoryType type) {
    MemoryBlock* block = memory_blocks;
    while (block != NULL) {
        if (block->start_address <= start && 
            block->start_address + block->size >= end) {
            
            // Split block if necessary
            if (block->start_address < start) {
                MemoryBlock* new_block = (MemoryBlock*)(block->start_address + 
                                                       (start - block->start_address));
                new_block->start_address = start;
                new_block->size = block->size - (start - block->start_address);
                new_block->type = block->type;
                new_block->next = block->next;
                new_block->prev = block;
                
                if (block->next) {
                    block->next->prev = new_block;
                }
                block->next = new_block;
                block->size = start - block->start_address;
            }
            
            if (block->start_address + block->size > end) {
                MemoryBlock* new_block = (MemoryBlock*)(end);
                new_block->start_address = end;
                new_block->size = block->start_address + block->size - end;
                new_block->type = block->type;
                new_block->next = block->next;
                new_block->prev = block;
                
                if (block->next) {
                    block->next->prev = new_block;
                }
                block->next = new_block;
                block->size = end - block->start_address;
            }
            
            block->type = type;
            return;
        }
        block = block->next;
    }
}

MemoryBlock* memory_find_block(uint64_t address) {
    MemoryBlock* block = memory_blocks;
    while (block != NULL) {
        if (block->start_address <= address && 
            block->start_address + block->size > address) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

void memory_merge_blocks(void) {
    MemoryBlock* block = memory_blocks;
    while (block != NULL && block->next != NULL) {
        if (block->type == MEMORY_TYPE_FREE && 
            block->next->type == MEMORY_TYPE_FREE) {
            
            // Merge blocks
            block->size += block->next->size + sizeof(MemoryBlock);
            block->next = block->next->next;
            if (block->next) {
                block->next->prev = block;
            }
        } else {
            block = block->next;
        }
    }
}

uint64_t memory_get_total_size(void) {
    return total_memory;
}

uint64_t memory_get_free_size(void) {
    return free_memory;
}

uint64_t memory_get_used_size(void) {
    return used_memory;
}

void memory_print_info(void) {
    console_printf("Memory Info:\n");
    console_printf("  Total: %llu bytes\n", total_memory);
    console_printf("  Used: %llu bytes\n", used_memory);
    console_printf("  Free: %llu bytes\n", free_memory);
}

void memory_copy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    for (size_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

void memory_set(void* dest, uint8_t value, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    
    for (size_t i = 0; i < size; i++) {
        d[i] = value;
    }
}

int memory_compare(const void* ptr1, const void* ptr2, size_t size) {
    const uint8_t* p1 = (const uint8_t*)ptr1;
    const uint8_t* p2 = (const uint8_t*)ptr2;
    
    for (size_t i = 0; i < size; i++) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
}

void* memory_align(void* ptr, size_t alignment) {
    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    return (void*)aligned;
}

size_t memory_align_size(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
} 