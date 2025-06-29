# MiniShell

A lightweight shell implementation in C that mimics core functionalities of bash/zsh with AI-powered command suggestions. This project demonstrates system programming skills, process management, and AI/ML integration in system software.

✨ **Now with AI-Powered Command Suggestions!** ✨

## Features

### Core Functionality
- Command execution (ls, grep, etc.)
- Pipeline support (|)
- I/O redirection (<, >, >>)
- Built-in commands (cd, pwd, echo, pinfo, etc.)
- Signal handling (Ctrl+C)
- Persistent command history
- Tab completion for commands and filenames

### AI-Powered Features
- **Smart Command Suggestions**: Predicts next commands based on your usage patterns
- **Local N-gram Model**: Fast, private command prediction without external dependencies
- **Context-Aware**: Considers command sequences and working directory
- **Self-Learning**: Improves suggestions as you use the shell
- **Fuzzy Matching**: Handles typos and partial commands

### Technical Implementation
- Process management using fork-exec model
- Pipeline mechanism using pipe() and dup2()
- Recursive command preprocessing
- Memory management and resource cleanup
- Comprehensive error handling
- N-gram statistical model for command prediction
- Command history analysis and pattern recognition

## Installation

1. Clone the repository:
```bash
git clone https://github.com/aditig05/MiniShell.git
cd MiniShell
```

2. Compile the project:
```bash
make
```

3. Run the shell:
```bash
./myshell
```

## Usage Examples

```bash
# Basic commands
ls
pwd
echo "Hello, World!"

# Pipelining
ls | grep .txt
cat file.txt | grep "pattern" | wc -l

# Redirection
ls > output.txt
cat < input.txt
ls >> append.txt

# Built-in commands
cd /path/to/directory
pinfo
setenv PATH /usr/local/bin:/usr/bin
```

## Project Structure

```
.
├── main.c              # Shell loop and signal handling
├── shell.h             # Structures and declarations
├── shell.c             # Core shell functionality
├── parser.c            # Command parsing
├── commands.c          # Built-in commands
├── ai_suggest.c        # AI-powered command suggestions
├── natural_commands.c  # Natural language processing
├── Makefile            # Build configuration
└── README.md           # Project documentation
```

## Technical Details

### AI Suggestion System
- **N-gram Model**: Implements bigram and trigram models for command prediction
- **Command History Analysis**: Learns from user's command patterns
- **Context Awareness**: Considers:
  - Previous command in sequence
  - Current working directory
  - Time of day
- **Smoothing**: Uses add-k smoothing for better prediction of rare commands
- **Efficient Storage**: Circular buffer for command history

### Process Management
- Fork-exec model for command execution
- Pipe creation and management
- Process synchronization
- Resource cleanup

### Command Parsing
- Recursive preprocessing
- Tokenization
- Command structure handling
- Special character processing

### Memory Management
- Dynamic allocation/deallocation
- File descriptor management
- Resource cleanup
- Memory leak prevention

## Error Handling
- System call error checking
- Proper error messages
- Graceful failure handling

## Using AI Command Suggestions

The shell now provides intelligent command suggestions based on your usage patterns. Suggestions appear automatically after each command execution.

### How it Works
1. The system learns from your command history
2. It builds a statistical model of command sequences
3. Suggestions are based on:
   - Most frequently used commands after the current one
   - Your recent command history
   - Current working directory context

### Example Usage
```bash
# As you use commands, the system learns patterns
cd /tmp
ls -la        # After this, you might see suggestions like 'ls -l', 'pwd', etc.

# The more you use the shell, the better the suggestions become
cd ~/projects
git status    # After this, common git commands will be suggested
```

## Limitations
- No job control
- Limited command history persistence
- No command aliases
- Basic environment variable expansion only
- No command substitution

## Future Improvements

### Phase 2: Advanced AI Integration
1. **LLM Integration**
   - Optional OpenAI GPT integration for advanced suggestions
   - Natural language command understanding
   - Error correction and explanation

2. **Enhanced Local AI**
   - More sophisticated pattern recognition
   - User-specific command prediction models
   - Cross-session learning

3. **Shell Features**
   - Job control (background processes, job management)
   - Command aliases
   - Advanced environment variable expansion
   - Command completion with AI suggestions

## Contributing
Feel free to submit issues and enhancement requests!

## License

This project is open source and available under the MIT License.

## Author

Aditi Goyal
- GitHub: [@aditig05](https://github.com/aditig05) 