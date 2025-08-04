#ifndef OCEAN_KERNEL_H
#define OCEAN_KERNEL_H

#include "../libs/types.h"
#include "../bootloader/framebuffer.h"

// Kernel version
#define KERNEL_VERSION_MAJOR 1
#define KERNEL_VERSION_MINOR 0
#define KERNEL_VERSION_PATCH 0

// Kernel state structure
typedef struct {
    FramebufferInfo* framebuffer;
    bool memory_initialized;
    bool display_initialized;
    bool scheduler_initialized;
    bool gui_initialized;
    bool shell_initialized;
} KernelState;

// Kernel main function
void kernel_main(FramebufferInfo* fb_info);

// Kernel panic function
void kernel_panic(const char* message);

// Get kernel state
KernelState* kernel_get_state(void);

// System calls
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_EXEC    4
#define SYS_EXIT    5
#define SYS_FORK    6
#define SYS_GETPID  7
#define SYS_SLEEP   8
#define SYS_YIELD   9

// Interrupt numbers
#define INT_DIVIDE_ERROR     0
#define INT_DEBUG            1
#define INT_NMI              2
#define INT_BREAKPOINT       3
#define INT_OVERFLOW         4
#define INT_BOUND_RANGE      5
#define INT_INVALID_OPCODE   6
#define INT_DEVICE_NOT_AVAILABLE 7
#define INT_DOUBLE_FAULT     8
#define INT_COPROCESSOR_SEGMENT_OVERRUN 9
#define INT_INVALID_TSS      10
#define INT_SEGMENT_NOT_PRESENT 11
#define INT_STACK_SEGMENT_FAULT 12
#define INT_GENERAL_PROTECTION_FAULT 13
#define INT_PAGE_FAULT       14
#define INT_FPU_ERROR        16
#define INT_ALIGNMENT_CHECK   17
#define INT_MACHINE_CHECK     18
#define INT_SIMD_FPU_ERROR    19

// IRQ numbers
#define IRQ_TIMER            32
#define IRQ_KEYBOARD         33
#define IRQ_CASCADE          34
#define IRQ_COM2             35
#define IRQ_COM1             36
#define IRQ_LPT2             37
#define IRQ_FLOPPY           38
#define IRQ_LPT1             39
#define IRQ_RTC              40
#define IRQ_PS2_MOUSE        44
#define IRQ_FPU              45
#define IRQ_PRIMARY_ATA      46
#define IRQ_SECONDARY_ATA    47

// Memory management
#define PAGE_SIZE             4096
#define PAGE_PRESENT         0x001
#define PAGE_WRITE           0x002
#define PAGE_USER            0x004
#define PAGE_WRITETHROUGH    0x008
#define PAGE_NOT_CACHEABLE   0x010
#define PAGE_ACCESSED        0x020
#define PAGE_DIRTY           0x040
#define PAGE_PAT             0x080
#define PAGE_GLOBAL          0x100

// Process states
typedef enum {
    PROCESS_STATE_NEW,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED
} ProcessState;

// Process structure
typedef struct Process {
    uint32_t pid;
    char name[32];
    ProcessState state;
    uint32_t priority;
    void* stack;
    size_t stack_size;
    struct Process* next;
} Process;

// Thread structure
typedef struct Thread {
    uint32_t tid;
    Process* process;
    void* stack;
    size_t stack_size;
    uint32_t esp;
    uint32_t ebp;
    struct Thread* next;
} Thread;

// File system
typedef struct {
    char name[256];
    uint32_t size;
    uint32_t type;
    uint32_t permissions;
    uint32_t created_time;
    uint32_t modified_time;
} FileInfo;

// Device types
typedef enum {
    DEVICE_TYPE_CHAR,
    DEVICE_TYPE_BLOCK,
    DEVICE_TYPE_NETWORK
} DeviceType;

// Device structure
typedef struct Device {
    char name[32];
    DeviceType type;
    uint32_t major;
    uint32_t minor;
    void* driver_data;
    struct Device* next;
} Device;

// Error codes
#define ERROR_SUCCESS        0
#define ERROR_INVALID_PARAMETER -1
#define ERROR_OUT_OF_MEMORY  -2
#define ERROR_FILE_NOT_FOUND -3
#define ERROR_ACCESS_DENIED  -4
#define ERROR_DEVICE_NOT_READY -5
#define ERROR_TIMEOUT        -6
#define ERROR_NOT_SUPPORTED  -7

// Utility macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))

// Debug macros
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) kernel_debug_print(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

// Debug function
void kernel_debug_print(const char* format, ...);

#endif // OCEAN_KERNEL_H