#include "kernel.h"
#include "../bootloader/framebuffer.h"
#include "memory/memory.h"
#include "drivers/display.h"
#include "drivers/input/key_event.h"
#include "scheduler/scheduler.h"
#include "userland/gui/gui.h"
#include "userland/shell/shell.h"

// Global kernel state
KernelState kernel_state;

void kernel_main(FramebufferInfo* fb_info) {
    // Initialize kernel state
    kernel_state.framebuffer = fb_info;
    kernel_state.memory_initialized = false;
    kernel_state.display_initialized = false;
    kernel_state.scheduler_initialized = false;
    
    // Initialize memory management
    memory_init();
    kernel_state.memory_initialized = true;
    
    // Initialize display driver
    display_init(fb_info);
    kernel_state.display_initialized = true;
    
    // Clear screen with blue background
    display_clear(COLOR_BLUE);
    
    // Draw welcome message
    display_draw_string(10, 10, "OceanOS Kernel Starting...", COLOR_WHITE);
    
    // Initialize input system
    key_event_init();
    
    // Initialize scheduler
    scheduler_init();
    kernel_state.scheduler_initialized = true;
    
    // Initialize GUI system
    gui_init();
    
    // Initialize shell
    shell_init();
    
    // Draw desktop
    gui_draw_desktop();
    
    // Main kernel loop
    while (1) {
        // Process input events
        key_event_process();
        
        // Run scheduler
        scheduler_run();
        
        // Update GUI
        gui_update();
        
        // Small delay to prevent excessive CPU usage
        for (volatile int i = 0; i < 1000000; i++) {}
    }
}

// Kernel panic function
void kernel_panic(const char* message) {
    display_clear(COLOR_RED);
    display_draw_string(10, 10, "KERNEL PANIC", COLOR_WHITE);
    display_draw_string(10, 30, message, COLOR_WHITE);
    
    while (1) {
        // Halt the system
        __asm__ volatile("hlt");
    }
}

// Get kernel state
KernelState* kernel_get_state(void) {
    return &kernel_state;
}