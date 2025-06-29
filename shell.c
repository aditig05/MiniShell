#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>
#include <errno.h>
#include <time.h>

#define MAX_HISTORY_SIZE 1000
#define HISTORY_FILE ".myshell_history"

static char *get_history_path() {
    static char path[1024];
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILE);
    return path;
}

void init_shell() {
    // Set up any necessary initialization
    setenv("SHELL", getcwd(NULL, 0), 1);
    
    // Set history file
    const char *histfile = get_history_path();
    
    // Read history if file exists
    if (access(histfile, F_OK) == 0) {
        if (read_history(histfile) != 0) {
            fprintf(stderr, "Warning: Could not read history from %s\n", histfile);
        } else {
            // Truncate history if needed
            int history_size = history_length;
            if (history_size > MAX_HISTORY_SIZE) {
                history_truncate_file(histfile, MAX_HISTORY_SIZE);
                clear_history();
                read_history(histfile);
            }
        }
    }
    
    // Set maximum history size
    stifle_history(MAX_HISTORY_SIZE);
    
    // Initialize AI suggestion system
    init_ai_suggest();
    
    // Analyze command history for patterns
    analyze_command_history();
}

void save_command_history() {
    const char *histfile = get_history_path();
    if (write_history(histfile) != 0) {
        fprintf(stderr, "Warning: Could not save history to %s: %s\n", 
                histfile, strerror(errno));
    }
    
    // Free AI suggestion resources
    free_ai_suggest();
}

char *get_prompt() {
    static char prompt[MAX_LINE * 2];  // Double the size to handle long paths
    char cwd[MAX_LINE];
    
    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }
    
    // Format prompt with color and path
    snprintf(prompt, sizeof(prompt), "\001\033[1;32m\002%s\001\033[0m\002$ ", cwd);
    
    // Ensure null termination
    prompt[sizeof(prompt) - 1] = '\0';
    return prompt;
}

char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t nread = getline(&line, &bufsize, stdin);
    
    if (nread == -1) {
        if (feof(stdin)) {
            printf("\n");
            exit(0);
        }
        perror("getline");
        return NULL;
    }
    
    // Remove newline
    if (line[nread - 1] == '\n') {
        line[nread - 1] = '\0';
    }
    
    return line;
}

void execute_pipeline(Pipeline *pipeline) {
    if (pipeline->command_count == 1) {
        execute_command(&pipeline->commands[0]);
        return;
    }
    
    int pipes[MAX_PIPES][2];
    pid_t pids[MAX_PIPES];
    
    // Create pipes
    for (int i = 0; i < pipeline->command_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }
    
    // Execute commands
    for (int i = 0; i < pipeline->command_count; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            return;
        }
        
        if (pids[i] == 0) {  // Child process
            // Set up input
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Set up output
            if (i < pipeline->command_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipes
            for (int j = 0; j < pipeline->command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            execute_command(&pipeline->commands[i]);
            exit(0);
        }
    }
    
    // Parent process
    // Close all pipes
    for (int i = 0; i < pipeline->command_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children
    for (int i = 0; i < pipeline->command_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

void execute_command(Command *cmd) {
    // Handle built-in commands
    if (strcmp(cmd->command, "cd") == 0) {
        builtin_cd(cmd);
        return;
    } else if (strcmp(cmd->command, "pwd") == 0) {
        builtin_pwd();
        return;
    } else if (strcmp(cmd->command, "echo") == 0) {
        builtin_echo(cmd);
        return;
    } else if (strcmp(cmd->command, "pinfo") == 0) {
        builtin_pinfo(cmd);
        return;
    } else if (strcmp(cmd->command, "setenv") == 0) {
        builtin_setenv(cmd);
        return;
    } else if (strcmp(cmd->command, "unsetenv") == 0) {
        builtin_unsetenv(cmd);
        return;
    } else if (strcmp(cmd->command, "help") == 0) {
        builtin_help(cmd);
        return;
    }
    
    // Handle redirection
    int stdin_fd = STDIN_FILENO;
    int stdout_fd = STDOUT_FILENO;
    
    if (cmd->input_file) {
        stdin_fd = open(cmd->input_file, O_RDONLY);
        if (stdin_fd == -1) {
            perror("open input file");
            return;
        }
    }
    
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append_output ? O_APPEND : O_TRUNC;
        stdout_fd = open(cmd->output_file, flags, 0644);
        if (stdout_fd == -1) {
            perror("open output file");
            if (stdin_fd != STDIN_FILENO) close(stdin_fd);
            return;
        }
    }
    
    // Execute external command
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {  // Child process
        if (stdin_fd != STDIN_FILENO) {
            dup2(stdin_fd, STDIN_FILENO);
            close(stdin_fd);
        }
        if (stdout_fd != STDOUT_FILENO) {
            dup2(stdout_fd, STDOUT_FILENO);
            close(stdout_fd);
        }
        
        execvp(cmd->command, cmd->args);
        perror("execvp");
        exit(1);
    } else {  // Parent process
        if (stdin_fd != STDIN_FILENO) close(stdin_fd);
        if (stdout_fd != STDOUT_FILENO) close(stdout_fd);
        waitpid(pid, NULL, 0);
    }
}

void free_command(Command *cmd) {
    if (cmd->command) free(cmd->command);
    if (cmd->args) {
        for (int i = 0; i < cmd->arg_count; i++) {
            free(cmd->args[i]);
        }
        free(cmd->args);
    }
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
}

void free_pipeline(Pipeline *pipeline) {
    if (pipeline->commands) {
        for (int i = 0; i < pipeline->command_count; i++) {
            free_command(&pipeline->commands[i]);
        }
        free(pipeline->commands);
    }
    free(pipeline);
} 