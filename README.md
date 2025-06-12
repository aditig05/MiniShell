# MiniShell

A lightweight shell implementation in C that mimics core functionalities of bash/zsh. This project demonstrates system programming skills, process management, and low-level C development.

## Features

### Core Functionality
- Command execution (ls, grep, etc.)
- Pipeline support (|)
- I/O redirection (<, >, >>)
- Built-in commands (cd, pwd, echo, pinfo, etc.)
- Signal handling (Ctrl+C)

### Technical Implementation
- Process management using fork-exec model
- Pipeline mechanism using pipe() and dup2()
- Recursive command preprocessing
- Memory management and resource cleanup
- Comprehensive error handling

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
├── main.c          # Shell loop and signal handling
├── shell.h         # Structures and declarations
├── shell.c         # Core shell functionality
├── parser.c        # Command parsing
├── commands.c      # Built-in commands
├── Makefile        # Build configuration
└── README.md       # Project documentation
```

## Technical Details

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

## Limitations
- No job control
- No command history
- No command aliases
- No environment variable expansion
- No command substitution

## Future Improvements

1. Job Control
   - Background process support
   - Process suspension
   - Job management

2. Enhanced Features
   - Command history
   - Command aliases
   - Environment variable expansion
   - Command completion

## Contributing
Feel free to submit issues and enhancement requests!

## License

This project is open source and available under the MIT License.

## Author

Aditi Goyal
- GitHub: [@aditig05](https://github.com/aditig05) 