#include "shell.h"

void init_shell() {
    // Set up any necessary initialization
    setenv("SHELL", getcwd(NULL, 0), 1);
}

void print_prompt() {
    char cwd[MAX_LINE];
    getcwd(cwd, sizeof(cwd));
    printf("\033[1;32m%s\033[0m$ ", cwd);
    fflush(stdout);
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