#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

// Configuration
#define MAX_NGRAM_ORDER 3
#define MAX_SUGGESTIONS 5
#define MAX_COMMAND_LENGTH 1024
#define MAX_HISTORY_SIZE 1000
#define SMOOTHING_FACTOR 0.1

// N-gram model structure
typedef struct {
    char **context;      // Array of context commands (n-1 commands)
    char *next_command;  // The command that follows the context
    int count;           // How many times this sequence was observed
    double probability;  // Calculated probability
} NGram;

typedef struct {
    NGram *ngrams;       // Array of all n-grams
    int size;            // Current number of n-grams
    int capacity;        // Allocated capacity
    int order;           // N-gram order (2=bigram, 3=trigram, etc.)
    char **command_history;  // Circular buffer for command history
    int history_size;    // Current history size
    int history_capacity;// Max history capacity
    int history_index;   // Current position in circular buffer
} NGramModel;

// Function declarations for n-gram model
static void init_ngram_model(NGramModel *model, int order);
static void free_ngram_model(NGramModel *model);
static void add_to_history(NGramModel *model, const char *command);
static void train_model(NGramModel *model);
static void update_ngram(NGramModel *model, const char **context, const char *next_command);
static char **get_suggestions(NGramModel *model, const char *prev_command, int *count);
static double calculate_probability(NGramModel *model, const char **context, const char *next_command);
static void normalize_probabilities(NGramModel *model);

// Global model instance
static NGramModel ngram_model;

// Context information
typedef struct {
    char current_dir[PATH_MAX];
    time_t timestamp;
    int hour_of_day;
    int is_weekend;
} CommandContext;

// Structure to store command information
typedef struct {
    char *command;          // Base command (e.g., 'ls')
    char **args;            // Common arguments
    int arg_count;          // Number of arguments
    int total_uses;         // Total times used
    time_t last_used;       // When last used
    char **contexts;        // Common contexts (directories)
    int context_count;      // Number of contexts
} CommandInfo;

// Structure to store command sequences
typedef struct {
    char *prev_command;
    char *current_command;
    CommandContext context;  // Context when command was used
    int count;              // Frequency
    time_t last_used;       // Last time this sequence was used
    float weight;           // Weighted score for sorting
} CommandSequence;

static CommandSequence *sequences = NULL;
static int sequence_count = 0;
static int sequence_capacity = 0;

// Global command info database
static CommandInfo *command_db = NULL;
static int command_db_size = 0;
static int command_db_capacity = 0;

// Current context
static CommandContext current_context;

// Initialize the current context
static void update_context() {
    getcwd(current_context.current_dir, sizeof(current_context.current_dir));
    time_t now = time(NULL);
    current_context.timestamp = now;
    
    struct tm *timeinfo = localtime(&now);
    current_context.hour_of_day = timeinfo->tm_hour;
    current_context.is_weekend = (timeinfo->tm_wday == 0 || timeinfo->tm_wday == 6);
}

// Calculate similarity between two contexts (0.0 to 1.0)
static float context_similarity(const CommandContext *a, const CommandContext *b) {
    float score = 0.0f;
    
    // Directory similarity (1.0 if same directory, 0.0 if completely different)
    if (strcmp(a->current_dir, b->current_dir) == 0) {
        score += 0.5f;
    }
    
    // Time of day similarity (closer hours get higher scores)
    int hour_diff = abs(a->hour_of_day - b->hour_of_day);
    if (hour_diff > 12) hour_diff = 24 - hour_diff;
    score += (12.0f - hour_diff) / 24.0f * 0.3f;
    
    // Weekend/weekday similarity
    if (a->is_weekend == b->is_weekend) {
        score += 0.2f;
    }
    
    return score;
}

// Parse a command into its components
static void parse_command(const char *cmd, char *base_cmd, char **args, int *arg_count) {
    char cmd_copy[MAX_COMMAND_LENGTH];
    strncpy(cmd_copy, cmd, sizeof(cmd_copy) - 1);
    cmd_copy[sizeof(cmd_copy) - 1] = '\0';
    
    *arg_count = 0;
    char *token = strtok(cmd_copy, " \t\n");
    
    if (token) {
        strncpy(base_cmd, token, MAX_COMMAND_LENGTH - 1);
        base_cmd[MAX_COMMAND_LENGTH - 1] = '\0';
        
        while ((token = strtok(NULL, " \t\n")) != NULL && *arg_count < MAX_ARGS - 1) {
            args[(*arg_count)++] = strdup(token);
        }
    }
    
    args[*arg_count] = NULL; // NULL terminate the argument list
}

// Find or create a command info entry
static CommandInfo* get_command_info(const char *command) {
    // First, try to find existing command
    for (int i = 0; i < command_db_size; i++) {
        if (strcmp(command_db[i].command, command) == 0) {
            return &command_db[i];
        }
    }
    
    // Not found, create new entry
    if (command_db_size >= command_db_capacity) {
        command_db_capacity = command_db_capacity ? command_db_capacity * 2 : 16;
        command_db = realloc(command_db, command_db_capacity * sizeof(CommandInfo));
    }
    
    CommandInfo *info = &command_db[command_db_size++];
    memset(info, 0, sizeof(CommandInfo));
    info->command = strdup(command);
    info->args = calloc(MAX_ARGS, sizeof(char*));
    info->contexts = calloc(16, sizeof(char*)); // Up to 16 different contexts
    info->last_used = time(NULL);
    
    return info;
}

// Initialize the AI suggestion system
void init_ai_suggest() {
    // Initialize n-gram model with trigrams (order=3)
    init_ngram_model(&ngram_model, 3);
    
    // Load command history if available
    char *home = getenv("HOME");
    if (home) {
        char histfile[PATH_MAX];
        snprintf(histfile, sizeof(histfile), "%s/.myshell_history", home);
        
        // Read history file and train model
        FILE *f = fopen(histfile, "r");
        if (f) {
            char line[MAX_COMMAND_LENGTH];
            while (fgets(line, sizeof(line), f)) {
                // Remove newline
                line[strcspn(line, "\n")] = 0;
                if (strlen(line) > 0) {
                    add_to_history(&ngram_model, line);
                }
            }
            fclose(f);
            
            // Train model on loaded history
            train_model(&ngram_model);
        }
    }
}

// Add a command to the history and update the model
void add_command_sequence(const char *prev, const char *current) {
    if (!current || strlen(current) == 0 || isspace(current[0])) {
        return;
    }
    
    // Add to history
    add_to_history(&ngram_model, current);
    
    // If we have a previous command, update the model
    if (prev && strlen(prev) > 0) {
        train_model(&ngram_model);
    }
    
    // Update current context
    update_context();
    
    // Parse the current command
    char base_cmd[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGS];
    int arg_count = 0;
    parse_command(current, base_cmd, args, &arg_count);
    
    // Update command info
    CommandInfo *cmd_info = get_command_info(base_cmd);
    cmd_info->total_uses++;
    cmd_info->last_used = time(NULL);
    
    // Add arguments to command info
    for (int i = 0; i < arg_count; i++) {
        // Simple check if argument already exists
        int found = 0;
        for (int j = 0; j < cmd_info->arg_count; j++) {
            if (strcmp(cmd_info->args[j], args[i]) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found && cmd_info->arg_count < MAX_ARGS - 1) {
            cmd_info->args[cmd_info->arg_count++] = strdup(args[i]);
        }
    }
    
    // Add current directory to contexts if not already present
    int context_found = 0;
    for (int i = 0; i < cmd_info->context_count; i++) {
        if (strcmp(cmd_info->contexts[i], current_context.current_dir) == 0) {
            context_found = 1;
            break;
        }
    }
    
    if (!context_found && cmd_info->context_count < 15) {
        cmd_info->contexts[cmd_info->context_count++] = strdup(current_context.current_dir);
    }
    
    // Check if this sequence already exists
    int existing_idx = -1;
    for (int i = 0; i < sequence_count; i++) {
        if (strcmp(sequences[i].prev_command, prev) == 0 && 
            strcmp(sequences[i].current_command, current) == 0) {
            existing_idx = i;
            break;
        }
    }
    
    time_t now = time(NULL);
    
    if (existing_idx >= 0) {
        // Update existing sequence
        sequences[existing_idx].count++;
        sequences[existing_idx].last_used = now;
        
        // Update context (weighted average with existing context)
        // Learning rate for context updates (commented out as not currently used)
        // float alpha = 0.3f;
        sequences[existing_idx].context = current_context;
    } else {
        // Add new sequence
        if (sequence_count >= sequence_capacity) {
            sequence_capacity = sequence_capacity ? sequence_capacity * 2 : 1;
            sequences = realloc(sequences, sequence_capacity * sizeof(CommandSequence));
        }
        
        sequences[sequence_count].prev_command = strdup(prev);
        sequences[sequence_count].current_command = strdup(current);
        sequences[sequence_count].context = current_context;
        sequences[sequence_count].count = 1;
        sequences[sequence_count].last_used = now;
        sequences[sequence_count].weight = 1.0f;
        sequence_count++;
    }
}

// Calculate weight for a sequence
static float calculate_sequence_weight(const CommandSequence *seq) {
    time_t now = time(NULL);
    float recency = 1.0f / (1.0f + (now - seq->last_used) / (3600.0f * 24.0f)); // Decay over days
    float context_score = context_similarity(&seq->context, &current_context);
    
    // Combine factors with weights
    return (seq->count * 0.5f) + (recency * 0.3f) + (context_score * 0.2f);
}

// Compare function for sorting sequences by weight
int compare_sequences(const void *a, const void *b) {
    const CommandSequence *seq_a = (const CommandSequence*)a;
    const CommandSequence *seq_b = (const CommandSequence*)b;
    
    // First compare by weight
    if (seq_b->weight > seq_a->weight) return 1;
    if (seq_b->weight < seq_a->weight) return -1;
    
    // If weights are equal, use count as tiebreaker
    return seq_b->count - seq_a->count;
}

// Find similar commands using Levenshtein distance
static int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    int dp[len1 + 1][len2 + 1];
    
    for (int i = 0; i <= len1; i++) dp[i][0] = i;
    for (int j = 0; j <= len2; j++) dp[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            dp[i][j] = dp[i-1][j-1] + cost; // substitution
            if (dp[i-1][j] + 1 < dp[i][j]) dp[i][j] = dp[i-1][j] + 1; // deletion
            if (dp[i][j-1] + 1 < dp[i][j]) dp[i][j] = dp[i][j-1] + 1; // insertion
        }
    }
    
    return dp[len1][len2];
}

// Find similar commands in the database
static char** find_similar_commands(const char *partial, int *count) {
    char **suggestions = malloc(MAX_SUGGESTIONS * sizeof(char*));
    int scores[MAX_SUGGESTIONS] = {0};
    *count = 0;
    
    for (int i = 0; i < command_db_size; i++) {
        int distance = levenshtein_distance(partial, command_db[i].command);
        
        // If this command is a better match than the worst in our current suggestions
        if (*count < MAX_SUGGESTIONS || distance < scores[MAX_SUGGESTIONS-1]) {
            // Find position to insert
            int pos = *count;
            while (pos > 0 && distance < scores[pos-1]) {
                if (pos < MAX_SUGGESTIONS) {
                    scores[pos] = scores[pos-1];
                    if (suggestions[pos]) free(suggestions[pos]);
                    suggestions[pos] = suggestions[pos-1];
                }
                pos--;
            }
            
            if (pos < MAX_SUGGESTIONS) {
                if (suggestions[pos]) free(suggestions[pos]);
                suggestions[pos] = strdup(command_db[i].command);
                scores[pos] = distance;
                if (*count < MAX_SUGGESTIONS) (*count)++;
            }
        }
    }
    
    return suggestions;
}

// Get command suggestions based on previous command
char **get_command_suggestions(const char *prev_command, int *count) {
    if (!prev_command) {
        *count = 0;
        return NULL;
    }
    
    return get_suggestions(&ngram_model, prev_command, count);
    
    // First pass: calculate weights for all matching sequences
    for (int i = 0; i < sequence_count; i++) {
        if (strcmp(sequences[i].prev_command, prev_command) == 0) {
            sequences[i].weight = calculate_sequence_weight(&sequences[i]);
        }
    }
    
    // Count matching sequences
    int match_count = 0;
    for (int i = 0; i < sequence_count; i++) {
        if (strcmp(sequences[i].prev_command, prev_command) == 0) {
            match_count++;
        }
    }
    
    if (match_count == 0) {
        return find_similar_commands(prev_command, count);
    }
    
    // Allocate array for matching sequences
    CommandSequence *matches = malloc(match_count * sizeof(CommandSequence));
    int match_index = 0;
    
    // Copy matching sequences
    for (int i = 0; i < sequence_count; i++) {
        if (strcmp(sequences[i].prev_command, prev_command) == 0) {
            matches[match_index++] = sequences[i];
        }
    }
    
    // Sort by weight
    qsort(matches, match_count, sizeof(CommandSequence), compare_sequences);
    
    // Limit number of suggestions
    int suggestion_count = (match_count < MAX_SUGGESTIONS) ? match_count : MAX_SUGGESTIONS;
    char **suggestions = malloc((suggestion_count + 1) * sizeof(char*));
    
    // Copy suggestions and add context information
    for (int i = 0; i < suggestion_count; i++) {
        // Add context hints to suggestions
        char context_hint[256] = "";
        if (strcmp(matches[i].context.current_dir, current_context.current_dir) != 0) {
            // If the command is usually used in a different directory, show the directory
            snprintf(context_hint, sizeof(context_hint), " (in %s)", 
                    basename(matches[i].context.current_dir));
        }
        
        // Allocate space for the suggestion + context hint + null terminator
        suggestions[i] = malloc(strlen(matches[i].current_command) + strlen(context_hint) + 1);
        sprintf(suggestions[i], "%s%s", matches[i].current_command, context_hint);
    }
    suggestions[suggestion_count] = NULL;
    
    free(matches);
    *count = suggestion_count;
    return suggestions;
}

// Free resources used by the AI suggestion system
void free_ai_suggest() {
    free_ngram_model(&ngram_model);
}

// N-gram model implementation
static void init_ngram_model(NGramModel *model, int order) {
    memset(model, 0, sizeof(NGramModel));
    model->order = order > MAX_NGRAM_ORDER ? MAX_NGRAM_ORDER : order;
    model->capacity = 1024;
    model->ngrams = calloc(model->capacity, sizeof(NGram));
    
    // Initialize command history circular buffer
    model->history_capacity = MAX_HISTORY_SIZE;
    model->command_history = calloc(model->history_capacity, sizeof(char *));
    model->history_size = 0;
    model->history_index = 0;
}

static void free_ngram_model(NGramModel *model) {
    if (!model) return;
    
    // Free n-grams
    for (int i = 0; i < model->size; i++) {
        free(model->ngrams[i].context);
        free(model->ngrams[i].next_command);
    }
    free(model->ngrams);
    
    // Free command history
    for (int i = 0; i < model->history_size; i++) {
        free(model->command_history[i]);
    }
    free(model->command_history);
    
    memset(model, 0, sizeof(NGramModel));
}

static void add_to_history(NGramModel *model, const char *command) {
    if (!model || !command || strlen(command) == 0) return;
    
    // Free old command if we're overwriting
    if (model->command_history[model->history_index]) {
        free(model->command_history[model->history_index]);
    }
    
    // Add new command
    model->command_history[model->history_index] = strdup(command);
    
    // Update indices
    model->history_index = (model->history_index + 1) % model->history_capacity;
    if (model->history_size < model->history_capacity) {
        model->history_size++;
    }
}

static void train_model(NGramModel *model) {
    if (model->history_size < 2) return;  // Need at least 2 commands to learn
    
    // Reset model
    model->size = 0;
    
    // Generate n-grams from history
    for (int i = model->order - 1; i < model->history_size; i++) {
        // Create context (n-1 previous commands)
        const char *context[model->order - 1];
        for (int j = 0; j < model->order - 1; j++) {
            int idx = (model->history_index - model->history_size + i - j - 1 + model->history_capacity) % model->history_capacity;
            context[model->order - 2 - j] = model->command_history[idx];
        }
        
        // Get next command
        int next_idx = (model->history_index - model->history_size + i) % model->history_capacity;
        const char *next_cmd = model->command_history[next_idx];
        
        // Update n-gram counts
        update_ngram(model, context, next_cmd);
    }
    
    // Calculate probabilities
    normalize_probabilities(model);
}

static void update_ngram(NGramModel *model, const char **context, const char *next_command) {
    if (!model || !next_command) return;
    
    // Check if this n-gram already exists
    for (int i = 0; i < model->size; i++) {
        int match = 1;
        
        // Check context matches
        for (int j = 0; j < model->order - 1; j++) {
            if (!context[j] || !model->ngrams[i].context[j] || 
                strcmp(context[j], model->ngrams[i].context[j]) != 0) {
                match = 0;
                break;
            }
        }
        
        // Check next command matches
        if (match && model->ngrams[i].next_command && 
            strcmp(next_command, model->ngrams[i].next_command) == 0) {
            model->ngrams[i].count++;
            return;
        }
    }
    
    // Add new n-gram if not found
    if (model->size >= model->capacity) {
        model->capacity *= 2;
        model->ngrams = realloc(model->ngrams, model->capacity * sizeof(NGram));
    }
    
    // Initialize new n-gram
    NGram *ngram = &model->ngrams[model->size];
    ngram->context = calloc(model->order - 1, sizeof(char *));
    
    // Copy context
    for (int i = 0; i < model->order - 1; i++) {
        if (context[i]) {
            ngram->context[i] = strdup(context[i]);
        }
    }
    
    // Copy next command
    ngram->next_command = strdup(next_command);
    ngram->count = 1;
    ngram->probability = 0.0;
    
    model->size++;
}

static char **get_suggestions(NGramModel *model, const char *prev_command, int *count) {
    if (!model || !prev_command || model->size == 0) {
        *count = 0;
        return NULL;
    }
    
    // Allocate array for suggestions
    char **suggestions = calloc(MAX_SUGGESTIONS, sizeof(char *));
    double scores[MAX_SUGGESTIONS] = {0};
    *count = 0;
    
    // Find matching n-grams
    for (int i = 0; i < model->size; i++) {
        // For bigram model (order=2), just check the previous command
        if (model->order >= 2 && model->ngrams[i].context[0] && 
            strcmp(model->ngrams[i].context[0], prev_command) == 0) {
            
            // Check if this is one of the top suggestions
            double score = model->ngrams[i].probability;
            
            // Apply recency bonus if available
            if (model->history_size > 0) {
                int last_idx = (model->history_index - 1 + model->history_capacity) % model->history_capacity;
                if (strcmp(model->command_history[last_idx], model->ngrams[i].next_command) == 0) {
                    score *= 1.5;  // Boost recent commands
                }
            }
            
            // Insert in sorted order
            for (int j = 0; j < MAX_SUGGESTIONS; j++) {
                if (score > scores[j]) {
                    // Shift existing suggestions down
                    for (int k = MAX_SUGGESTIONS - 1; k > j; k--) {
                        if (suggestions[k-1]) {
                            free(suggestions[k]);
                            suggestions[k] = strdup(suggestions[k-1]);
                            scores[k] = scores[k-1];
                        }
                    }
                    
                    // Insert new suggestion
                    if (suggestions[j]) {
                        free(suggestions[j]);
                    }
                    suggestions[j] = strdup(model->ngrams[i].next_command);
                    scores[j] = score;
                    
                    if (*count < MAX_SUGGESTIONS) {
                        (*count)++;
                    }
                    break;
                }
            }
        }
    }
    
    return suggestions;
}

static void normalize_probabilities(NGramModel *model) {
    // First pass: calculate total counts for each context
    for (int i = 0; i < model->size; i++) {
        double total = 0;
        
        // Find all n-grams with the same context
        for (int j = 0; j < model->size; j++) {
            int match = 1;
            
            for (int k = 0; k < model->order - 1; k++) {
                if ((!model->ngrams[i].context[k] && model->ngrams[j].context[k]) ||
                    (model->ngrams[i].context[k] && !model->ngrams[j].context[k]) ||
                    (model->ngrams[i].context[k] && model->ngrams[j].context[k] &&
                     strcmp(model->ngrams[i].context[k], model->ngrams[j].context[k]) != 0)) {
                    match = 0;
                    break;
                }
            }
            
            if (match) {
                total += model->ngrams[j].count;
            }
        }
        
        // Calculate probability with add-k smoothing
        if (total > 0) {
            model->ngrams[i].probability = 
                (model->ngrams[i].count + SMOOTHING_FACTOR) / 
                (total + SMOOTHING_FACTOR * model->size);
        }
    }
}

// Analyze command history to learn patterns
void analyze_command_history() {
    // Get the current history state
    HISTORY_STATE *hs = history_get_history_state();
    if (!hs || hs->length < 2) {
        free(hs);
        return; // Need at least 2 commands to find patterns
    }
    
    // Limit the number of history entries to analyze
    int start = (hs->length > MAX_HISTORY_ANALYSIS) ? 
                (hs->length - MAX_HISTORY_ANALYSIS) : 0;
    
    // Add sequences from history
    for (int i = start + 1; i < hs->length; i++) {
        HIST_ENTRY *prev_he = history_get(i-1);
        HIST_ENTRY *curr_he = history_get(i);
        
        if (prev_he && curr_he && prev_he->line && curr_he->line) {
            add_command_sequence(prev_he->line, curr_he->line);
        }
    }
    
    free(hs);
}
