#include "shell.h"
#include <string.h>
#include <ctype.h>

// Structure to map natural language patterns to commands
typedef struct {
    const char *pattern;
    const char *command;
} CommandMapping;

// Common natural language patterns and their corresponding commands
static const CommandMapping command_mappings[] = {
    {"list files", "ls"},
    {"show files", "ls"},
    {"what's here", "ls"},
    {"show me what's in this folder", "ls"},
    
    {"go to ", "cd "},  // Note the space after cd
    {"change to ", "cd "},
    {"navigate to ", "cd "},
    
    {"where am i", "pwd"},
    {"current directory", "pwd"},
    
    {"show content of ", "cat "},
    {"display ", "cat "},
    {"open ", "cat "},
    
    {"search for ", "grep "},
    {"find ", "grep "},
    
    {"count lines in ", "wc -l "},
    {"word count of ", "wc -w "},
    
    {"make directory ", "mkdir "},
    {"create folder ", "mkdir "},
    
    {"remove ", "rm "},
    {"delete ", "rm "},
    
    {NULL, NULL}  // End of array marker
};

// Convert natural language to shell command
char* natural_to_shell_command(const char* input) {
    if (!input) return NULL;
    
    // Convert input to lowercase for case-insensitive matching
    char* lower_input = strdup(input);
    for (int i = 0; lower_input[i]; i++) {
        lower_input[i] = tolower(lower_input[i]);
    }
    
    // Try to match patterns
    for (int i = 0; command_mappings[i].pattern != NULL; i++) {
        size_t pattern_len = strlen(command_mappings[i].pattern);
        if (strncmp(lower_input, command_mappings[i].pattern, pattern_len) == 0) {
            // Allocate space for the command
            size_t cmd_len = strlen(command_mappings[i].command);
            size_t remaining_input = strlen(lower_input + pattern_len);
            char* result = malloc(cmd_len + remaining_input + 2);  // +2 for space and null terminator
            
            if (result) {
                strcpy(result, command_mappings[i].command);
                // If there's remaining input after the pattern, append it
                if (remaining_input > 0) {
                    strcat(result, " ");
                    strcat(result, input + pattern_len);
                }
                free(lower_input);
                return result;
            }
        }
    }
    
    // No match found, return the original input
    free(lower_input);
    return strdup(input);
}
