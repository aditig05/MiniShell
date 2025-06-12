#include "shell.h"

// Helper function to trim whitespace
static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

// Helper function to split string by delimiter
static char **split_string(char *str, const char *delim, int *count) {
    char **result = NULL;
    char *token;
    int i = 0;
    
    // First pass: count tokens
    char *str_copy = strdup(str);
    token = strtok(str_copy, delim);
    while (token) {
        i++;
        token = strtok(NULL, delim);
    }
    free(str_copy);
    
    // Allocate array
    result = malloc((i + 1) * sizeof(char *));
    if (!result) return NULL;
    
    // Second pass: fill array
    i = 0;
    token = strtok(str, delim);
    while (token) {
        result[i++] = strdup(trim(token));
        token = strtok(NULL, delim);
    }
    result[i] = NULL;
    
    if (count) *count = i;
    return result;
}

// Initialize a new command structure
static Command *init_command() {
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;
    
    cmd->command = NULL;
    cmd->args = malloc((MAX_ARGS + 1) * sizeof(char *));
    cmd->arg_count = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;
    
    return cmd;
}

// Parse a single command
static Command *parse_command(char *cmd_str) {
    Command *cmd = init_command();
    if (!cmd) return NULL;
    
    // Split command and arguments
    char **parts = split_string(cmd_str, " \t", &cmd->arg_count);
    if (!parts) {
        free(cmd);
        return NULL;
    }
    
    if (cmd->arg_count > 0) {
        cmd->command = strdup(parts[0]);
        cmd->args[0] = strdup(parts[0]);
        
        for (int i = 1; i < cmd->arg_count; i++) {
            if (strcmp(parts[i], "<") == 0) {
                if (i + 1 < cmd->arg_count) {
                    cmd->input_file = strdup(parts[i + 1]);
                    i++;
                }
            } else if (strcmp(parts[i], ">") == 0) {
                if (i + 1 < cmd->arg_count) {
                    cmd->output_file = strdup(parts[i + 1]);
                    cmd->append_output = 0;
                    i++;
                }
            } else if (strcmp(parts[i], ">>") == 0) {
                if (i + 1 < cmd->arg_count) {
                    cmd->output_file = strdup(parts[i + 1]);
                    cmd->append_output = 1;
                    i++;
                }
            } else {
                cmd->args[i] = strdup(parts[i]);
            }
        }
        cmd->args[cmd->arg_count] = NULL;
    }
    
    // Free temporary array
    for (int i = 0; parts[i]; i++) {
        free(parts[i]);
    }
    free(parts);
    
    return cmd;
}

Pipeline *parse_line(char *line) {
    if (!line || strlen(line) == 0) return NULL;
    
    // Split by pipe
    int pipe_count;
    char **pipe_parts = split_string(line, "|", &pipe_count);
    if (!pipe_parts) return NULL;
    
    // Create pipeline
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) {
        for (int i = 0; pipe_parts[i]; i++) free(pipe_parts[i]);
        free(pipe_parts);
        return NULL;
    }
    
    pipeline->commands = malloc(pipe_count * sizeof(Command));
    pipeline->command_count = pipe_count;
    
    // Parse each command
    for (int i = 0; i < pipe_count; i++) {
        pipeline->commands[i] = *parse_command(pipe_parts[i]);
        free(pipe_parts[i]);
    }
    free(pipe_parts);
    
    return pipeline;
} 