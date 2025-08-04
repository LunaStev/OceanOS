#ifndef OCEAN_SHELL_H
#define OCEAN_SHELL_H

#include "../../libs/types.h"
#include "../../kernel/drivers/display.h"
#include "../../kernel/drivers/input/key_event.h"

// Command structure
typedef struct {
    char name[32];
    char description[128];
    int (*execute)(int argc, char* argv[]);
} Command;

// Shell state
typedef struct {
    char prompt[64];
    char current_directory[256];
    char command_buffer[1024];
    int command_position;
    int command_history[10][1024];
    int history_count;
    int history_position;
    bool running;
} ShellState;

// Shell functions
void shell_init(void);
void shell_run(void);
void shell_update(void);
void shell_handle_input(KeyEvent* event);
void shell_execute_command(const char* command);
void shell_print_prompt(void);
void shell_print_error(const char* message);
void shell_print_help(void);

// Command functions
int cmd_help(int argc, char* argv[]);
int cmd_clear(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_ls(int argc, char* argv[]);
int cmd_cd(int argc, char* argv[]);
int cmd_pwd(int argc, char* argv[]);
int cmd_cat(int argc, char* argv[]);
int cmd_mkdir(int argc, char* argv[]);
int cmd_rm(int argc, char* argv[]);
int cmd_mv(int argc, char* argv[]);
int cmd_cp(int argc, char* argv[]);
int cmd_date(int argc, char* argv[]);
int cmd_time(int argc, char* argv[]);
int cmd_whoami(int argc, char* argv[]);
int cmd_ps(int argc, char* argv[]);
int cmd_kill(int argc, char* argv[]);
int cmd_memory(int argc, char* argv[]);
int cmd_reboot(int argc, char* argv[]);
int cmd_shutdown(int argc, char* argv[]);

// Command registration
void shell_register_command(const char* name, const char* description, int (*execute)(int argc, char* argv[]));
Command* shell_find_command(const char* name);

// Input handling
void shell_handle_backspace(void);
void shell_handle_enter(void);
void shell_handle_character(char c);
void shell_handle_arrow_up(void);
void shell_handle_arrow_down(void);
void shell_handle_arrow_left(void);
void shell_handle_arrow_right(void);

// Output functions
void shell_putchar(char c);
void shell_puts(const char* str);
void shell_printf(const char* format, ...);

// File system commands
int shell_file_exists(const char* path);
int shell_is_directory(const char* path);
int shell_list_directory(const char* path);
int shell_change_directory(const char* path);
int shell_read_file(const char* path, char* buffer, size_t size);
int shell_write_file(const char* path, const char* buffer, size_t size);
int shell_create_directory(const char* path);
int shell_delete_file(const char* path);

// Process management commands
int shell_get_process_list(void);
int shell_kill_process(uint32_t pid);
int shell_get_memory_info(void);

// Utility functions
void shell_parse_command(const char* command, int* argc, char* argv[]);
char* shell_get_current_directory(void);
void shell_set_current_directory(const char* path);
void shell_add_to_history(const char* command);
const char* shell_get_from_history(int index);

// Shell colors
#define SHELL_COLOR_PROMPT     0x00FFFFFF
#define SHELL_COLOR_TEXT       0x00FFFFFF
#define SHELL_COLOR_ERROR      0x00FF0000
#define SHELL_COLOR_SUCCESS    0x0000FF00
#define SHELL_COLOR_WARNING    0x00FFFF00
#define SHELL_COLOR_INFO       0x0000FFFF

// Shell constants
#define SHELL_MAX_COMMAND_LENGTH 1024
#define SHELL_MAX_ARGS           32
#define SHELL_HISTORY_SIZE       10
#define SHELL_PROMPT_LENGTH      64

// Built-in commands
#define CMD_HELP     "help"
#define CMD_CLEAR    "clear"
#define CMD_ECHO     "echo"
#define CMD_LS       "ls"
#define CMD_CD       "cd"
#define CMD_PWD      "pwd"
#define CMD_CAT      "cat"
#define CMD_MKDIR    "mkdir"
#define CMD_RM       "rm"
#define CMD_MV       "mv"
#define CMD_CP       "cp"
#define CMD_DATE     "date"
#define CMD_TIME     "time"
#define CMD_WHOAMI   "whoami"
#define CMD_PS       "ps"
#define CMD_KILL     "kill"
#define CMD_MEMORY   "memory"
#define CMD_REBOOT   "reboot"
#define CMD_SHUTDOWN "shutdown"

#endif // OCEAN_SHELL_H