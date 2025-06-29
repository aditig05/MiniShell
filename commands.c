#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>  // For PATH_MAX

// Structure to hold command help information
typedef struct {
    const char *name;
    const char *usage;
    const char *description;
} CommandHelp;

// Command help information
static const CommandHelp command_help[] = {
    {"cd", "cd [directory]", "Change the current directory to 'directory'. If no directory is specified, changes to the home directory."},
    {"pwd", "pwd", "Print the current working directory."},
    {"echo", "echo [text...]", "Display a line of text."},
    {"pinfo", "pinfo [pid]", "Display information about a process. If no PID is provided, shows information about the shell process."},
    {"setenv", "setenv VAR [value]", "Set an environment variable. If no value is provided, sets it to an empty string."},
    {"unsetenv", "unsetenv VAR", "Remove an environment variable."},
    {"help", "help [command]", "Display help information. If no command is specified, lists all available commands."},
    
    // Common external commands
    {"ls", "ls [options] [file...]", "List directory contents."},
    {"grep", "grep [options] pattern [file...]", "Search for patterns in files."},
    {"cat", "cat [file...]", "Concatenate and display file contents."},
    {"mkdir", "mkdir [options] directory...", "Create directories."},
    {"rm", "rm [options] file...", "Remove files or directories."},
    {"cp", "cp [options] source... destination", "Copy files and directories."},
    {"mv", "mv [options] source... destination", "Move or rename files and directories."},
    
    {NULL, NULL, NULL}  // End marker
};

// Display help for all commands or a specific command
void builtin_help(Command *cmd) {
    if (cmd->arg_count > 1) {
        // Show help for a specific command
        const char *target_cmd = cmd->args[1];
        int found = 0;
        
        // Print a separator line
        printf("\n\033[1;34m=== Help for: %s ===\033[0m\n\n", target_cmd);
        
        // Search for the command in the help list
        for (int i = 0; command_help[i].name != NULL; i++) {
            if (strcmp(command_help[i].name, target_cmd) == 0) {
                printf("\033[1;33m%s\033[0m - %s\n\n", 
                       command_help[i].name, command_help[i].description);
                printf("\033[1;37mUsage:\033[0m %s\n", command_help[i].usage);
                found = 1;
                break;
            }
        }
        
        if (!found) {
            printf("No help found for command: \033[1;31m%s\033[0m\n", target_cmd);
            printf("Type 'help' to see a list of available commands.\n");
        }
        printf("\n");
    } else {
        // Show help for all commands
        printf("\n\033[1;34m=== MiniShell - Available Commands ===\033[0m\n\n");
        printf("Type 'help [command]' for more information about a specific command.\n\n");
        
        // Print built-in commands
        printf("\033[1;32mBuilt-in Commands:\033[0m\n");
        for (int i = 0; command_help[i].name != NULL; i++) {
            if (i == 7) {  // After built-in commands, print external commands
                printf("\n\033[1;32mCommon External Commands:\033[0m\n");
            }
            printf("  \033[1;33m%-10s\033[0m - %s\n", 
                   command_help[i].name, command_help[i].description);
        }
        
        printf("\n\033[1;36mNatural Language Support:\033[0m\n");
        printf("  You can use natural language commands like:\n");
        printf("  - 'list files' instead of 'ls'\n");
        printf("  - 'show content of file.txt' instead of 'cat file.txt'\n");
        printf("  - 'go to folder' instead of 'cd folder'\n\n");
    }
}

void builtin_cd(Command *cmd) {
    char *path = NULL;
    
    // Handle case where cmd or cmd->args is NULL
    if (cmd == NULL || cmd->args == NULL) {
        path = getenv("HOME");
    } else if (cmd->arg_count <= 1 || cmd->args[1] == NULL) {
        // No directory provided, use HOME
        path = getenv("HOME");
        if (path == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    } else {
        // Use the provided directory
        path = cmd->args[1];
    }
    
    // Check if path is still NULL (shouldn't happen, but better safe than sorry)
    if (path == NULL) {
        fprintf(stderr, "cd: No directory specified and HOME not set\n");
        return;
    }
    
    // Attempt to change directory
    if (chdir(path) != 0) {
        perror("cd");
    } else {
        // Update PWD environment variable
        char *cwd = getcwd(NULL, 0);
        if (cwd != NULL) {
            setenv("PWD", cwd, 1);
            free(cwd);
        }
    }
}

void builtin_pwd() {
    char *cwd = getcwd(NULL, 0);
    if (cwd != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}

void builtin_echo(Command *cmd) {
    for (int i = 1; i < cmd->arg_count; i++) {
        printf("%s", cmd->args[i]);
        if (i < cmd->arg_count - 1) printf(" ");
    }
    printf("\n");
}

void builtin_pinfo(Command *cmd) {
    pid_t pid;
    if (cmd->arg_count > 1) {
        pid = atoi(cmd->args[1]);
    } else {
        pid = getpid();
    }
    
    char proc_path[256];
    char exe_path[256];
    char status;
    long vm_size;
    
    // Read process status
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/stat", pid);
    FILE *stat_file = fopen(proc_path, "r");
    if (stat_file) {
        fscanf(stat_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %ld", &status, &vm_size);
        fclose(stat_file);
    }
    
    // Read executable path
    snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);
    ssize_t len = readlink(proc_path, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    } else {
        strcpy(exe_path, "Unknown");
    }
    
    printf("pid -- %d\n", pid);
    printf("Process Status -- %c\n", status);
    printf("memory -- %ld\n", vm_size);
    printf("Executable Path -- %s\n", exe_path);
}

void builtin_setenv(Command *cmd) {
    if (cmd->arg_count < 2) {
        fprintf(stderr, "setenv: too few arguments\n");
        return;
    }
    
    char *value = cmd->arg_count > 2 ? cmd->args[2] : "";
    if (setenv(cmd->args[1], value, 1) != 0) {
        perror("setenv");
    }
}

void builtin_unsetenv(Command *cmd) {
    if (cmd->arg_count < 2) {
        fprintf(stderr, "unsetenv: too few arguments\n");
        return;
    }
    
    if (unsetenv(cmd->args[1]) != 0) {
        perror("unsetenv");
    }
}