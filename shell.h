#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

#define MAX_LINE 80
#define MAX_ARGS 10
#define MAX_PIPES 10

// Structure to hold command information
typedef struct {
    char *command;
    char **args;
    int arg_count;
    char *input_file;
    char *output_file;
    int append_output;
} Command;

// Structure to hold pipeline information
typedef struct {
    Command *commands;
    int command_count;
} Pipeline;

// Function declarations
void init_shell();
char *get_prompt();
void save_command_history();
char *natural_to_shell_command(const char* input);

// AI command suggestion functions - Phase 1: Local Statistical Analysis
void init_ai_suggest();                          // Initialize the AI suggestion system
void add_command_sequence(const char *prev, const char *current); // Add command to history
char **get_command_suggestions(const char *prev_command, int *count); // Get suggestions
void free_ai_suggest();                         // Free AI resources

// Phase 2: External AI Integration (for future implementation)
typedef enum {
    AI_MODE_LOCAL,     // Use local statistical model (default)
    AI_MODE_LLM        // Use external LLM (not yet implemented)
} AIMode;

// Future function declarations for Phase 2
void set_ai_mode(AIMode mode);
int enable_llm_integration(const char *api_key);
void disable_llm_integration();

// Built-in command functions
void builtin_cd(Command *cmd);
void builtin_pwd();
void builtin_echo(Command *cmd);
void builtin_pinfo(Command *cmd);
void builtin_setenv(Command *cmd);
void builtin_unsetenv(Command *cmd);
void builtin_help(Command *cmd);

// Helper functions
void free_pipeline(Pipeline *pipeline);
void free_command(Command *cmd);
char *get_absolute_path(const char *path);

#endif // SHELL_H 