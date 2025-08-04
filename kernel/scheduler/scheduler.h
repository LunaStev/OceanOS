#ifndef OCEAN_SCHEDULER_H
#define OCEAN_SCHEDULER_H

#include "../../libs/types.h"
#include "../kernel.h"

// Process states
typedef enum {
    PROCESS_STATE_NEW,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED
} ProcessState;

// Thread states
typedef enum {
    THREAD_STATE_NEW,
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_WAITING,
    THREAD_STATE_TERMINATED
} ThreadState;

// Process structure
typedef struct Process {
    uint32_t pid;
    char name[32];
    ProcessState state;
    uint32_t priority;
    void* stack;
    size_t stack_size;
    struct Process* next;
    struct Process* prev;
} Process;

// Thread structure
typedef struct Thread {
    uint32_t tid;
    Process* process;
    ThreadState state;
    uint32_t priority;
    void* stack;
    size_t stack_size;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t time_slice;
    uint32_t time_used;
    struct Thread* next;
    struct Thread* prev;
} Thread;

// Scheduler types
typedef enum {
    SCHEDULER_ROUND_ROBIN,
    SCHEDULER_PRIORITY,
    SCHEDULER_MULTILEVEL_FEEDBACK
} SchedulerType;

// Scheduler functions
void scheduler_init(void);
void scheduler_run(void);
void scheduler_yield(void);
void scheduler_sleep(uint32_t milliseconds);
void scheduler_wake_up(Thread* thread);

// Process management
Process* process_create(const char* name, uint32_t priority);
void process_destroy(Process* process);
Process* process_get_current(void);
uint32_t process_get_pid(void);

// Thread management
Thread* thread_create(Process* process, void (*entry)(void), uint32_t priority);
void thread_destroy(Thread* thread);
Thread* thread_get_current(void);
uint32_t thread_get_tid(void);
void thread_yield(void);
void thread_sleep(uint32_t milliseconds);

// Context switching
void context_switch(Thread* from, Thread* to);
void context_save(uint32_t* esp, uint32_t* ebp);
void context_restore(uint32_t esp, uint32_t ebp);

// Timer functions
void timer_init(void);
void timer_set_frequency(uint32_t frequency);
uint32_t timer_get_ticks(void);
void timer_sleep(uint32_t milliseconds);

// Interrupt handling
void interrupt_handler_timer(void);
void interrupt_handler_keyboard(void);

// Priority levels
#define PRIORITY_LOW     1
#define PRIORITY_NORMAL  5
#define PRIORITY_HIGH    10
#define PRIORITY_CRITICAL 15

// Time slice values
#define TIME_SLICE_DEFAULT 10
#define TIME_SLICE_MIN     1
#define TIME_SLICE_MAX     100

// Maximum processes and threads
#define MAX_PROCESSES 64
#define MAX_THREADS_PER_PROCESS 16

#endif // OCEAN_SCHEDULER_H