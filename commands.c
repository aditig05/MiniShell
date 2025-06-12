#include "shell.h"

void builtin_cd(Command *cmd) {
    char *path = cmd->arg_count > 1 ? cmd->args[1] : getenv("HOME");
    if (chdir(path) != 0) {
        perror("cd");
    }
}

void builtin_pwd() {
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
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