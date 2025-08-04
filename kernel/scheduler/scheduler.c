#include "scheduler.h"
#include "../../../libs/stdio.h"
#include "../memory/memory.h"

// Global scheduler state
static Thread* current_thread = NULL;
static Thread* ready_queue = NULL;
static Thread* waiting_queue = NULL;
static uint32_t next_tid = 1;
static uint32_t next_pid = 1;
static uint32_t timer_ticks = 0;
static SchedulerType scheduler_type = SCHEDULER_ROUND_ROBIN;

// Timer frequency (Hz)
#define TIMER_FREQUENCY 100

void scheduler_init(void) {
    // Initialize scheduler state
    current_thread = NULL;
    ready_queue = NULL;
    waiting_queue = NULL;
    next_tid = 1;
    next_pid = 1;
    timer_ticks = 0;
    
    // Initialize timer
    timer_init();
    timer_set_frequency(TIMER_FREQUENCY);
    
    // Create idle thread
    Process* idle_process = process_create("idle", PRIORITY_LOW);
    Thread* idle_thread = thread_create(idle_process, idle_task, PRIORITY_LOW);
    
    // Set as current thread
    current_thread = idle_thread;
    current_thread->state = THREAD_STATE_RUNNING;
    
    console_puts("Scheduler initialized\n");
}

void scheduler_run(void) {
    // Check if current thread has used its time slice
    if (current_thread && current_thread->time_used >= current_thread->time_slice) {
        scheduler_yield();
    }
    
    // Check for waiting threads that should be woken up
    Thread* thread = waiting_queue;
    while (thread) {
        Thread* next = thread->next;
        if (thread->sleep_until <= timer_ticks) {
            // Remove from waiting queue
            if (thread->prev) {
                thread->prev->next = thread->next;
            } else {
                waiting_queue = thread->next;
            }
            if (thread->next) {
                thread->next->prev = thread->prev;
            }
            
            // Add to ready queue
            thread->state = THREAD_STATE_READY;
            thread->next = ready_queue;
            thread->prev = NULL;
            if (ready_queue) {
                ready_queue->prev = thread;
            }
            ready_queue = thread;
        }
        thread = next;
    }
}

void scheduler_yield(void) {
    if (!current_thread) return;
    
    // Save current thread context
    context_save(&current_thread->esp, &current_thread->ebp);
    
    // Move current thread to ready queue if it's still running
    if (current_thread->state == THREAD_STATE_RUNNING) {
        current_thread->state = THREAD_STATE_READY;
        current_thread->time_used = 0;
        
        // Add to ready queue based on priority
        if (scheduler_type == SCHEDULER_PRIORITY) {
            // Insert based on priority (higher priority first)
            Thread* prev = NULL;
            Thread* curr = ready_queue;
            while (curr && curr->priority >= current_thread->priority) {
                prev = curr;
                curr = curr->next;
            }
            
            current_thread->next = curr;
            current_thread->prev = prev;
            if (prev) {
                prev->next = current_thread;
            } else {
                ready_queue = current_thread;
            }
            if (curr) {
                curr->prev = current_thread;
            }
        } else {
            // Round-robin: add to end of queue
            Thread* last = ready_queue;
            while (last && last->next) {
                last = last->next;
            }
            
            current_thread->next = NULL;
            current_thread->prev = last;
            if (last) {
                last->next = current_thread;
            } else {
                ready_queue = current_thread;
            }
        }
    }
    
    // Select next thread
    if (ready_queue) {
        current_thread = ready_queue;
        ready_queue = ready_queue->next;
        if (ready_queue) {
            ready_queue->prev = NULL;
        }
        
        current_thread->state = THREAD_STATE_RUNNING;
        current_thread->time_used = 0;
        
        // Restore thread context
        context_restore(current_thread->esp, current_thread->ebp);
    }
}

void scheduler_sleep(uint32_t milliseconds) {
    thread_sleep(milliseconds);
}

void scheduler_wake_up(Thread* thread) {
    if (!thread) return;
    
    // Remove from waiting queue
    if (thread->prev) {
        thread->prev->next = thread->next;
    } else {
        waiting_queue = thread->next;
    }
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    
    // Add to ready queue
    thread->state = THREAD_STATE_READY;
    thread->next = ready_queue;
    thread->prev = NULL;
    if (ready_queue) {
        ready_queue->prev = thread;
    }
    ready_queue = thread;
}

Process* process_create(const char* name, uint32_t priority) {
    Process* process = (Process*)memory_allocate(sizeof(Process));
    if (!process) return NULL;
    
    process->pid = next_pid++;
    strncpy(process->name, name, 31);
    process->name[31] = '\0';
    process->state = PROCESS_STATE_NEW;
    process->priority = priority;
    process->stack = NULL;
    process->stack_size = 0;
    process->next = NULL;
    process->prev = NULL;
    
    return process;
}

void process_destroy(Process* process) {
    if (!process) return;
    
    // Free process stack
    if (process->stack) {
        memory_free(process->stack);
    }
    
    // Free process structure
    memory_free(process);
}

Process* process_get_current(void) {
    if (current_thread) {
        return current_thread->process;
    }
    return NULL;
}

uint32_t process_get_pid(void) {
    Process* process = process_get_current();
    return process ? process->pid : 0;
}

Thread* thread_create(Process* process, void (*entry)(void), uint32_t priority) {
    Thread* thread = (Thread*)memory_allocate(sizeof(Thread));
    if (!thread) return NULL;
    
    // Allocate stack
    size_t stack_size = 4096; // 4KB stack
    void* stack = memory_allocate(stack_size);
    if (!stack) {
        memory_free(thread);
        return NULL;
    }
    
    thread->tid = next_tid++;
    thread->process = process;
    thread->state = THREAD_STATE_NEW;
    thread->priority = priority;
    thread->stack = stack;
    thread->stack_size = stack_size;
    thread->time_slice = TIME_SLICE_DEFAULT;
    thread->time_used = 0;
    thread->next = NULL;
    thread->prev = NULL;
    
    // Initialize stack for thread entry
    uint32_t* stack_ptr = (uint32_t*)((uint8_t*)stack + stack_size - 16);
    
    // Push return address (thread exit function)
    *--stack_ptr = (uint32_t)thread_exit;
    
    // Push entry point
    *--stack_ptr = (uint32_t)entry;
    
    // Push flags
    *--stack_ptr = 0x202; // IF flag set
    
    // Push registers (eax, ecx, edx, ebx, esp, ebp, esi, edi)
    for (int i = 0; i < 8; i++) {
        *--stack_ptr = 0;
    }
    
    thread->esp = (uint32_t)stack_ptr;
    thread->ebp = thread->esp;
    thread->eip = (uint32_t)entry;
    thread->eflags = 0x202;
    
    return thread;
}

void thread_destroy(Thread* thread) {
    if (!thread) return;
    
    // Remove from any queue
    if (thread->prev) {
        thread->prev->next = thread->next;
    }
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    
    // Free stack
    if (thread->stack) {
        memory_free(thread->stack);
    }
    
    // Free thread structure
    memory_free(thread);
}

Thread* thread_get_current(void) {
    return current_thread;
}

uint32_t thread_get_tid(void) {
    return current_thread ? current_thread->tid : 0;
}

void thread_yield(void) {
    scheduler_yield();
}

void thread_sleep(uint32_t milliseconds) {
    if (!current_thread) return;
    
    // Calculate wake time
    uint32_t wake_ticks = timer_ticks + (milliseconds * TIMER_FREQUENCY / 1000);
    
    // Save context
    context_save(&current_thread->esp, &current_thread->ebp);
    
    // Move to waiting queue
    current_thread->state = THREAD_STATE_WAITING;
    current_thread->sleep_until = wake_ticks;
    
    // Add to waiting queue (sorted by wake time)
    Thread* prev = NULL;
    Thread* curr = waiting_queue;
    while (curr && curr->sleep_until <= wake_ticks) {
        prev = curr;
        curr = curr->next;
    }
    
    current_thread->next = curr;
    current_thread->prev = prev;
    if (prev) {
        prev->next = current_thread;
    } else {
        waiting_queue = current_thread;
    }
    if (curr) {
        curr->prev = current_thread;
    }
    
    // Switch to next ready thread
    scheduler_yield();
}

void context_switch(Thread* from, Thread* to) {
    if (from) {
        context_save(&from->esp, &from->ebp);
    }
    
    if (to) {
        context_restore(to->esp, to->ebp);
    }
}

void context_save(uint32_t* esp, uint32_t* ebp) {
    __asm__ volatile("mov %%esp, %0" : "=m"(*esp));
    __asm__ volatile("mov %%ebp, %0" : "=m"(*ebp));
}

void context_restore(uint32_t esp, uint32_t ebp) {
    __asm__ volatile("mov %0, %%esp" : : "r"(esp));
    __asm__ volatile("mov %0, %%ebp" : : "r"(ebp));
}

void timer_init(void) {
    // Program PIT (Programmable Interval Timer)
    uint32_t divisor = 1193180 / TIMER_FREQUENCY;
    
    // Send command byte
    __outb(0x43, 0x36);
    
    // Send divisor
    __outb(0x40, divisor & 0xFF);
    __outb(0x40, (divisor >> 8) & 0xFF);
}

void timer_set_frequency(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    
    __outb(0x43, 0x36);
    __outb(0x40, divisor & 0xFF);
    __outb(0x40, (divisor >> 8) & 0xFF);
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

void timer_sleep(uint32_t milliseconds) {
    uint32_t start_ticks = timer_ticks;
    uint32_t target_ticks = start_ticks + (milliseconds * TIMER_FREQUENCY / 1000);
    
    while (timer_ticks < target_ticks) {
        // Wait for timer interrupt
        __asm__ volatile("hlt");
    }
}

void interrupt_handler_timer(void) {
    timer_ticks++;
    
    if (current_thread) {
        current_thread->time_used++;
    }
    
    // Send EOI (End of Interrupt)
    __outb(0x20, 0x20);
}

void interrupt_handler_keyboard(void) {
    // Process keyboard input
    key_event_process();
    
    // Send EOI
    __outb(0x20, 0x20);
}

// Idle task
void idle_task(void) {
    while (1) {
        // Halt CPU to save power
        __asm__ volatile("hlt");
    }
}

// Thread exit function
void thread_exit(void) {
    // Mark thread as terminated
    if (current_thread) {
        current_thread->state = THREAD_STATE_TERMINATED;
    }
    
    // Switch to next thread
    scheduler_yield();
    
    // Should never reach here
    while (1) {
        __asm__ volatile("hlt");
    }
}

// I/O port functions
static inline void __outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t __inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
} 