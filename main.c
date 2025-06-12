#include "shell.h"

static int running = 1;

void handle_sigint(int sig) {
    (void)sig;  // Suppress unused parameter warning
    printf("\n");
    print_prompt();
    fflush(stdout);
}

int main() {
    // Initialize shell
    init_shell();
    
    // Set up signal handling
    signal(SIGINT, handle_sigint);
    
    // Main shell loop
    while (running) {
        print_prompt();
        
        // Read command line
        char *line = read_line();
        if (line == NULL) {
            continue;
        }
        
        // Parse and execute command
        Pipeline *pipeline = parse_line(line);
        if (pipeline != NULL) {
            execute_pipeline(pipeline);
            free_pipeline(pipeline);
        }
        
        free(line);
    }
    
    return 0;
} 