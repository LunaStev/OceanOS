#include "shell.h"
#include "../../libs/stdio.h"
#include "../../kernel/memory/memory.h"

// Global shell state
static ShellState shell_state;
static Command commands[32];
static int command_count = 0;

// Shell display position
static uint32_t shell_x = 0;
static uint32_t shell_y = 0;
static uint32_t shell_width = 80;
static uint32_t shell_height = 25;

void shell_init(void) {
    // Initialize shell state
    strcpy(shell_state.prompt, "OceanOS> ");
    strcpy(shell_state.current_directory, "/");
    shell_state.command_buffer[0] = '\0';
    shell_state.command_position = 0;
    shell_state.history_count = 0;
    shell_state.history_position = 0;
    shell_state.running = true;
    
    // Get display dimensions
    shell_width = display_get_width() / 8;  // 8 pixels per character
    shell_height = display_get_height() / 16; // 16 pixels per character
    
    // Register built-in commands
    shell_register_command(CMD_HELP, "Show available commands", cmd_help);
    shell_register_command(CMD_CLEAR, "Clear the screen", cmd_clear);
    shell_register_command(CMD_ECHO, "Display a line of text", cmd_echo);
    shell_register_command(CMD_LS, "List directory contents", cmd_ls);
    shell_register_command(CMD_CD, "Change directory", cmd_cd);
    shell_register_command(CMD_PWD, "Print working directory", cmd_pwd);
    shell_register_command(CMD_CAT, "Display file contents", cmd_cat);
    shell_register_command(CMD_MKDIR, "Create directory", cmd_mkdir);
    shell_register_command(CMD_RM, "Remove file or directory", cmd_rm);
    shell_register_command(CMD_MV, "Move or rename file", cmd_mv);
    shell_register_command(CMD_CP, "Copy file", cmd_cp);
    shell_register_command(CMD_DATE, "Show current date", cmd_date);
    shell_register_command(CMD_TIME, "Show current time", cmd_time);
    shell_register_command(CMD_WHOAMI, "Show current user", cmd_whoami);
    shell_register_command(CMD_PS, "Show process list", cmd_ps);
    shell_register_command(CMD_KILL, "Kill a process", cmd_kill);
    shell_register_command(CMD_MEMORY, "Show memory information", cmd_memory);
    shell_register_command(CMD_REBOOT, "Reboot the system", cmd_reboot);
    shell_register_command(CMD_SHUTDOWN, "Shutdown the system", cmd_shutdown);
    
    // Print welcome message
    shell_puts("OceanOS Shell v1.0\n");
    shell_puts("Type 'help' for available commands\n\n");
    shell_print_prompt();
    
    console_puts("Shell initialized\n");
}

void shell_run(void) {
    while (shell_state.running) {
        // Process input events
        while (key_event_available()) {
            KeyEvent event = key_event_dequeue();
            shell_handle_input(&event);
        }
        
        // Small delay to prevent excessive CPU usage
        for (volatile int i = 0; i < 100000; i++) {}
    }
}

void shell_update(void) {
    // Process input events
    while (key_event_available()) {
        KeyEvent event = key_event_dequeue();
        shell_handle_input(&event);
    }
}

void shell_handle_input(KeyEvent* event) {
    if (!event || !event->pressed) return;
    
    switch (event->scancode) {
        case KEY_BACKSPACE:
            shell_handle_backspace();
            break;
        case KEY_ENTER:
            shell_handle_enter();
            break;
        case KEY_UP:
            shell_handle_arrow_up();
            break;
        case KEY_DOWN:
            shell_handle_arrow_down();
            break;
        case KEY_LEFT:
            shell_handle_arrow_left();
            break;
        case KEY_RIGHT:
            shell_handle_arrow_right();
            break;
        default:
            if (event->ascii >= 32 && event->ascii <= 126) {
                shell_handle_character(event->ascii);
            }
            break;
    }
}

void shell_handle_character(char c) {
    if (shell_state.command_position < SHELL_MAX_COMMAND_LENGTH - 1) {
        // Insert character at current position
        for (int i = shell_state.command_position; i < strlen(shell_state.command_buffer); i++) {
            shell_state.command_buffer[i + 1] = shell_state.command_buffer[i];
        }
        shell_state.command_buffer[shell_state.command_position] = c;
        shell_state.command_position++;
        
        // Redraw command line
        shell_print_prompt();
        shell_puts(shell_state.command_buffer);
    }
}

void shell_handle_backspace(void) {
    if (shell_state.command_position > 0) {
        // Remove character at current position
        for (int i = shell_state.command_position - 1; i < strlen(shell_state.command_buffer); i++) {
            shell_state.command_buffer[i] = shell_state.command_buffer[i + 1];
        }
        shell_state.command_position--;
        
        // Redraw command line
        shell_print_prompt();
        shell_puts(shell_state.command_buffer);
    }
}

void shell_handle_enter(void) {
    shell_puts("\n");
    
    if (strlen(shell_state.command_buffer) > 0) {
        // Add to history
        shell_add_to_history(shell_state.command_buffer);
        
        // Execute command
        shell_execute_command(shell_state.command_buffer);
    }
    
    // Clear command buffer
    shell_state.command_buffer[0] = '\0';
    shell_state.command_position = 0;
    
    // Print new prompt
    shell_print_prompt();
}

void shell_execute_command(const char* command) {
    int argc;
    char* argv[SHELL_MAX_ARGS];
    
    shell_parse_command(command, &argc, argv);
    
    if (argc > 0) {
        Command* cmd = shell_find_command(argv[0]);
        if (cmd) {
            int result = cmd->execute(argc, argv);
            if (result != 0) {
                shell_print_error("Command failed\n");
            }
        } else {
            shell_printf("Command not found: %s\n", argv[0]);
        }
    }
}

void shell_parse_command(const char* command, int* argc, char* argv[]) {
    *argc = 0;
    const char* p = command;
    
    while (*p && *argc < SHELL_MAX_ARGS) {
        // Skip whitespace
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        
        // Find end of argument
        const char* start = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        
        // Copy argument
        int len = p - start;
        if (len > 0) {
            strncpy(argv[*argc], start, len);
            argv[*argc][len] = '\0';
            (*argc)++;
        }
    }
}

void shell_register_command(const char* name, const char* description, int (*execute)(int argc, char* argv[])) {
    if (command_count < 32) {
        strcpy(commands[command_count].name, name);
        strcpy(commands[command_count].description, description);
        commands[command_count].execute = execute;
        command_count++;
    }
}

Command* shell_find_command(const char* name) {
    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void shell_print_prompt(void) {
    shell_puts(shell_state.prompt);
}

void shell_print_error(const char* message) {
    shell_puts(message);
}

void shell_print_help(void) {
    shell_puts("OceanOS Shell Help\n");
    shell_puts("Available commands:\n");
    for (int i = 0; i < command_count; i++) {
        shell_printf("  %-10s - %s\n", commands[i].name, commands[i].description);
    }
}

void shell_putchar(char c) {
    if (c == '\n') {
        shell_x = 0;
        shell_y++;
        if (shell_y >= shell_height) {
            shell_y = shell_height - 1;
            // Scroll up
            // This would be implemented with display scrolling
        }
    } else {
        display_draw_char(shell_x * 8, shell_y * 16, c, SHELL_COLOR_TEXT);
        shell_x++;
        if (shell_x >= shell_width) {
            shell_x = 0;
            shell_y++;
        }
    }
}

void shell_puts(const char* str) {
    while (*str) {
        shell_putchar(*str++);
    }
}

void shell_printf(const char* format, ...) {
    // Simple implementation - just print the format string
    shell_puts(format);
}

// Built-in command implementations
int cmd_help(int argc, char* argv[]) {
    shell_puts("Available commands:\n");
    for (int i = 0; i < command_count; i++) {
        shell_printf("  %-10s - %s\n", commands[i].name, commands[i].description);
    }
    return 0;
}

int cmd_clear(int argc, char* argv[]) {
    display_clear(COLOR_BLACK);
    shell_x = 0;
    shell_y = 0;
    return 0;
}

int cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        shell_puts(argv[i]);
        if (i < argc - 1) shell_puts(" ");
    }
    shell_puts("\n");
    return 0;
}

int cmd_ls(int argc, char* argv[]) {
    const char* path = (argc > 1) ? argv[1] : shell_state.current_directory;
    shell_printf("Listing directory: %s\n", path);
    // This would list actual files
    shell_puts("(File system not implemented yet)\n");
    return 0;
}

int cmd_cd(int argc, char* argv[]) {
    const char* path = (argc > 1) ? argv[1] : "/";
    shell_printf("Changing directory to: %s\n", path);
    strcpy(shell_state.current_directory, path);
    return 0;
}

int cmd_pwd(int argc, char* argv[]) {
    shell_printf("Current directory: %s\n", shell_state.current_directory);
    return 0;
}

int cmd_whoami(int argc, char* argv[]) {
    shell_puts("Current user: root\n");
    return 0;
}

int cmd_memory(int argc, char* argv[]) {
    shell_puts("Memory information:\n");
    shell_puts("(Memory info not implemented yet)\n");
    return 0;
}

int cmd_reboot(int argc, char* argv[]) {
    shell_puts("Rebooting system...\n");
    // This would actually reboot the system
    return 0;
}

int cmd_shutdown(int argc, char* argv[]) {
    shell_puts("Shutting down system...\n");
    // This would actually shutdown the system
    return 0;
}

// Placeholder implementations for other commands
int cmd_cat(int argc, char* argv[]) { shell_puts("cat: not implemented\n"); return 0; }
int cmd_mkdir(int argc, char* argv[]) { shell_puts("mkdir: not implemented\n"); return 0; }
int cmd_rm(int argc, char* argv[]) { shell_puts("rm: not implemented\n"); return 0; }
int cmd_mv(int argc, char* argv[]) { shell_puts("mv: not implemented\n"); return 0; }
int cmd_cp(int argc, char* argv[]) { shell_puts("cp: not implemented\n"); return 0; }
int cmd_date(int argc, char* argv[]) { shell_puts("date: not implemented\n"); return 0; }
int cmd_time(int argc, char* argv[]) { shell_puts("time: not implemented\n"); return 0; }
int cmd_ps(int argc, char* argv[]) { shell_puts("ps: not implemented\n"); return 0; }
int cmd_kill(int argc, char* argv[]) { shell_puts("kill: not implemented\n"); return 0; }

void shell_add_to_history(const char* command) {
    if (shell_state.history_count < SHELL_HISTORY_SIZE) {
        strcpy(shell_state.command_history[shell_state.history_count], command);
        shell_state.history_count++;
    } else {
        // Shift history
        for (int i = 0; i < SHELL_HISTORY_SIZE - 1; i++) {
            strcpy(shell_state.command_history[i], shell_state.command_history[i + 1]);
        }
        strcpy(shell_state.command_history[SHELL_HISTORY_SIZE - 1], command);
    }
    shell_state.history_position = shell_state.history_count;
}

void shell_handle_arrow_up(void) {
    if (shell_state.history_position > 0) {
        shell_state.history_position--;
        strcpy(shell_state.command_buffer, shell_state.command_history[shell_state.history_position]);
        shell_state.command_position = strlen(shell_state.command_buffer);
        
        shell_print_prompt();
        shell_puts(shell_state.command_buffer);
    }
}

void shell_handle_arrow_down(void) {
    if (shell_state.history_position < shell_state.history_count) {
        shell_state.history_position++;
        if (shell_state.history_position < shell_state.history_count) {
            strcpy(shell_state.command_buffer, shell_state.command_history[shell_state.history_position]);
        } else {
            shell_state.command_buffer[0] = '\0';
        }
        shell_state.command_position = strlen(shell_state.command_buffer);
        
        shell_print_prompt();
        shell_puts(shell_state.command_buffer);
    }
}

void shell_handle_arrow_left(void) {
    if (shell_state.command_position > 0) {
        shell_state.command_position--;
    }
}

void shell_handle_arrow_right(void) {
    if (shell_state.command_position < strlen(shell_state.command_buffer)) {
        shell_state.command_position++;
    }
}

// File system commands (placeholder implementations)
int shell_file_exists(const char* path) {
    return 0; // Not implemented
}

int shell_is_directory(const char* path) {
    return 0; // Not implemented
}

int shell_list_directory(const char* path) {
    return 0; // Not implemented
}

int shell_change_directory(const char* path) {
    return 0; // Not implemented
}

int shell_read_file(const char* path, char* buffer, size_t size) {
    return 0; // Not implemented
}

int shell_write_file(const char* path, const char* buffer, size_t size) {
    return 0; // Not implemented
}

int shell_create_directory(const char* path) {
    return 0; // Not implemented
}

int shell_delete_file(const char* path) {
    return 0; // Not implemented
}

// Process management commands (placeholder implementations)
int shell_get_process_list(void) {
    return 0; // Not implemented
}

int shell_kill_process(uint32_t pid) {
    return 0; // Not implemented
}

int shell_get_memory_info(void) {
    return 0; // Not implemented
}

// Utility functions
char* shell_get_current_directory(void) {
    return shell_state.current_directory;
}

void shell_set_current_directory(const char* path) {
    strcpy(shell_state.current_directory, path);
}

const char* shell_get_from_history(int index) {
    if (index >= 0 && index < shell_state.history_count) {
        return shell_state.command_history[index];
    }
    return NULL;
}