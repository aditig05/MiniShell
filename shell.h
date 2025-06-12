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
void print_prompt();
char *read_line();
Pipeline *parse_line(char *line);
void execute_pipeline(Pipeline *pipeline);
void execute_command(Command *cmd);
void handle_builtin(Command *cmd);

// Built-in command functions
void builtin_cd(Command *cmd);
void builtin_pwd();
void builtin_echo(Command *cmd);
void builtin_pinfo(Command *cmd);
void builtin_setenv(Command *cmd);
void builtin_unsetenv(Command *cmd);

// Helper functions
void free_pipeline(Pipeline *pipeline);
void free_command(Command *cmd);
char *get_absolute_path(const char *path);

#endif // SHELL_H 