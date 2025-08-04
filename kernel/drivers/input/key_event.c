// OceanOS
// Kernel Drivers
// key_event.c

#include "key_event.h"
#include "../../../libs/stdio.h"
#include "../../kernel.h"

// Keyboard state
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool caps_lock = false;

// Key event queue
#define KEY_QUEUE_SIZE 256
static KeyEvent key_queue[KEY_QUEUE_SIZE];
static int key_queue_head = 0;
static int key_queue_tail = 0;
static int key_queue_count = 0;

// PS/2 Keyboard I/O ports
#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64

// PS/2 Commands
#define PS2_CMD_READ_CONFIG  0x20
#define PS2_CMD_WRITE_CONFIG 0x60
#define PS2_CMD_DISABLE_PORT2 0xA7
#define PS2_CMD_ENABLE_PORT2  0xA8
#define PS2_CMD_TEST_PORT2    0xA9
#define PS2_CMD_TEST_CONTROLLER 0xAA
#define PS2_CMD_TEST_PORT1    0xAB
#define PS2_CMD_DISABLE_PORT1 0xAD
#define PS2_CMD_ENABLE_PORT1  0xAE
#define PS2_CMD_READ_OUTPUT   0xD0
#define PS2_CMD_WRITE_OUTPUT  0xD1

// PS/2 Responses
#define PS2_RESP_ACK      0xFA
#define PS2_RESP_RESEND   0xFE
#define PS2_RESP_ERROR    0xFC

// Keyboard scan codes
#define SCANCODE_ESCAPE    0x01
#define SCANCODE_BACKSPACE 0x0E
#define SCANCODE_TAB       0x0F
#define SCANCODE_ENTER     0x1C
#define SCANCODE_LSHIFT    0x2A
#define SCANCODE_RSHIFT    0x36
#define SCANCODE_LCTRL     0x1D
#define SCANCODE_LALT      0x38
#define SCANCODE_CAPS_LOCK 0x3A
#define SCANCODE_F1        0x3B
#define SCANCODE_F2        0x3C
#define SCANCODE_F3        0x3D
#define SCANCODE_F4        0x3E
#define SCANCODE_F5        0x3F
#define SCANCODE_F6        0x40
#define SCANCODE_F7        0x41
#define SCANCODE_F8        0x42
#define SCANCODE_F9        0x43
#define SCANCODE_F10       0x44
#define SCANCODE_F11       0x57
#define SCANCODE_F12       0x58

// ASCII conversion table (without shift)
static const char ascii_table[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
};

// ASCII conversion table (with shift)
static const char ascii_table_shift[128] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0
};

void key_event_init(void) {
    // Initialize PS/2 keyboard
    key_queue_head = 0;
    key_queue_tail = 0;
    key_queue_count = 0;
    
    // Clear keyboard state
    shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    caps_lock = false;
    
    // Disable interrupts temporarily
    __asm__ volatile("cli");
    
    // Wait for input buffer to be empty
    while (__inb(PS2_COMMAND_PORT) & 0x02);
    
    // Send command to disable first PS/2 port
    __outb(PS2_COMMAND_PORT, PS2_CMD_DISABLE_PORT1);
    
    // Wait for input buffer to be empty
    while (__inb(PS2_COMMAND_PORT) & 0x02);
    
    // Read configuration byte
    __outb(PS2_COMMAND_PORT, PS2_CMD_READ_CONFIG);
    while (!(__inb(PS2_COMMAND_PORT) & 0x01));
    uint8_t config = __inb(PS2_DATA_PORT);
    
    // Clear bits 0 and 1 (disable interrupts and translation)
    config &= ~0x03;
    
    // Write configuration back
    __outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_CONFIG);
    while (__inb(PS2_COMMAND_PORT) & 0x02);
    __outb(PS2_DATA_PORT, config);
    
    // Enable first PS/2 port
    __outb(PS2_COMMAND_PORT, PS2_CMD_ENABLE_PORT1);
    
    // Wait for input buffer to be empty
    while (__inb(PS2_COMMAND_PORT) & 0x02);
    
    // Set bits 0 and 1 (enable interrupts and translation)
    config |= 0x03;
    __outb(PS2_COMMAND_PORT, PS2_CMD_WRITE_CONFIG);
    while (__inb(PS2_COMMAND_PORT) & 0x02);
    __outb(PS2_DATA_PORT, config);
    
    // Re-enable interrupts
    __asm__ volatile("sti");
    
    console_puts("Keyboard initialized\n");
}

void key_event_process(void) {
    // Check if there's data available
    if (!(__inb(PS2_COMMAND_PORT) & 0x01)) {
        return;
    }
    
    // Read scan code
    uint8_t scancode = __inb(PS2_DATA_PORT);
    
    // Handle extended scancodes
    static bool extended = false;
    if (scancode == 0xE0) {
        extended = true;
        return;
    }
    
    // Create key event
    KeyEvent event;
    event.scancode = scancode;
    event.pressed = !(scancode & 0x80);
    event.extended = extended;
    
    // Clear extended flag
    extended = false;
    
    // Handle special keys
    switch (scancode & 0x7F) {
        case SCANCODE_LSHIFT:
        case SCANCODE_RSHIFT:
            shift_pressed = event.pressed;
            break;
        case SCANCODE_LCTRL:
            ctrl_pressed = event.pressed;
            break;
        case SCANCODE_LALT:
            alt_pressed = event.pressed;
            break;
        case SCANCODE_CAPS_LOCK:
            if (event.pressed) {
                caps_lock = !caps_lock;
            }
            break;
    }
    
    // Convert to ASCII
    if (event.pressed && scancode < 128) {
        bool use_shift = shift_pressed ^ caps_lock;
        if (use_shift) {
            event.ascii = ascii_table_shift[scancode];
        } else {
            event.ascii = ascii_table[scancode];
        }
    } else {
        event.ascii = 0;
    }
    
    // Add to queue
    key_event_enqueue(&event);
}

bool key_event_available(void) {
    return key_queue_count > 0;
}

KeyEvent key_event_dequeue(void) {
    KeyEvent event = {0};
    
    if (key_queue_count > 0) {
        event = key_queue[key_queue_head];
        key_queue_head = (key_queue_head + 1) % KEY_QUEUE_SIZE;
        key_queue_count--;
    }
    
    return event;
}

void key_event_enqueue(KeyEvent* event) {
    if (key_queue_count < KEY_QUEUE_SIZE) {
        key_queue[key_queue_tail] = *event;
        key_queue_tail = (key_queue_tail + 1) % KEY_QUEUE_SIZE;
        key_queue_count++;
    }
}

bool key_event_is_shift_pressed(void) {
    return shift_pressed;
}

bool key_event_is_ctrl_pressed(void) {
    return ctrl_pressed;
}

bool key_event_is_alt_pressed(void) {
    return alt_pressed;
}

bool key_event_is_caps_lock_on(void) {
    return caps_lock;
}

// I/O port functions
static inline uint8_t __inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void __outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}