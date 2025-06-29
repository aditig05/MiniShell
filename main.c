#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include <sys/stat.h>

static int running = 1;

void handle_sigint(int sig) {
    (void)sig;  // Suppress unused parameter warning
    printf("\n");
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

// Command generator function for tab completion
char *command_generator(const char *text, int state) {
    static int list_index, len;
    const char *name;
    static const char *commands[] = {
        "cd", "pwd", "echo", "pinfo", "setenv", "unsetenv", "help",
        "ls", "grep", "cat", "mkdir", "rm", "cp", "mv", NULL
    };

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    // Check commands first
    while ((name = commands[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    // Then check files in current directory
    static DIR *dir = NULL;
    static struct dirent *entry = NULL;
    static char *dirname = NULL;
    
    if (!state) {
        // Find the last space in the line
        const char *last_space = strrchr(rl_line_buffer, ' ');
        if (last_space) {
            // If there's a space, we're completing a filename
            if (dirname) free(dirname);
            dirname = strndup(rl_line_buffer, last_space - rl_line_buffer + 1);
            text = last_space + 1;
            len = strlen(text);
            
            // Open the directory
            if (dir) closedir(dir);
            dir = opendir(".");
            if (!dir) return NULL;
        } else {
            // No space, we're completing a command
            return NULL;
        }
    }

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, text, len) == 0) {
                // If we have a directory, add a trailing slash
                struct stat st;
                if (stat(entry->d_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                    char *result = malloc(strlen(entry->d_name) + 2);
                    sprintf(result, "%s/", entry->d_name);
                    return result;
                } else {
                    return strdup(entry->d_name);
                }
            }
        }
        closedir(dir);
        dir = NULL;
    }

    return NULL;
}

// Attempt to complete on the contents of TEXT
char **command_completion(const char *text, int start, int end) {
    (void)start;  // Unused parameter
    (void)end;    // Unused parameter
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}

int main() {
    // Initialize shell
    init_shell();
    
    // Set up signal handling
    signal(SIGINT, handle_sigint);
    
    // Initialize readline
    using_history();
    
    // Set up tab completion
    rl_attempted_completion_function = command_completion;
    rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{(";
    rl_completion_append_character = '\0';
    rl_attempted_completion_over = 0;
    
    // Store the last command for suggestions
    char *last_command = NULL;
    
    // Main shell loop
    while (running) {
        // Show AI suggestions if we have command history
        if (last_command) {
            int suggestion_count = 0;
            char **suggestions = get_command_suggestions(last_command, &suggestion_count);
            
            if (suggestion_count > 0) {
                printf("\n\033[90mSuggestions: ");
                for (int i = 0; i < suggestion_count && i < 3; i++) {
                    printf("%s%s", i > 0 ? ", " : "", suggestions[i]);
                    free(suggestions[i]);
                }
                printf("\033[0m");  // Reset color
                free(suggestions);
                fflush(stdout);
            }
        }
        
        // Get input using readline
        char *input = readline(get_prompt());
        if (!input) {
            printf("\n");
            break;  // Handle Ctrl+D
        }
        
        // Skip empty input
        if (strlen(input) == 0) {
            free(input);
            continue;
        }
        
        // Add to history
        add_history(input);
        save_command_history();
        
        // Process natural language input
        char *processed_line = natural_to_shell_command(input);
        if (strlen(processed_line) > 0) {
            // Parse and execute the command
            Pipeline *pipeline = parse_line(processed_line);
            if (pipeline) {
                execute_pipeline(pipeline);
                
                // Update AI model with the new command sequence
                if (last_command) {
                    add_command_sequence(last_command, processed_line);
                    free(last_command);
                }
                last_command = strdup(processed_line);
                
                free_pipeline(pipeline);
            }
        }
        
        // Clean up
        free(input);
        free(processed_line);
    }
    
    // Clean up
    if (last_command) {
        free(last_command);
    }
    
    // Save command history before exiting
    save_command_history();
    
    return 0;
} 