# Custom Shell Implementation

A simple yet powerful shell implementation in C that supports basic command execution, pipelining, and I/O redirection.

## Features

### Basic Command Execution
- Execute system commands (ls, grep, etc.)
- Support for command arguments
- Error handling and reporting

### Pipeline Support
- Multiple commands connected by pipes (|)
- Example: `ls | grep .txt | wc -l`

### I/O Redirection
- Input redirection (`<`)
- Output redirection (`>`)
- Append mode (`>>`)
- Example: `ls > output.txt`, `cat < input.txt`, `ls >> append.txt`

### Built-in Commands
- `cd`: Change directory
- `pwd`: Print working directory
- `echo`: Print arguments
- `pinfo`: Process information
- `setenv`: Set environment variable
- `unsetenv`: Unset environment variable

### Signal Handling
- Graceful handling of Ctrl+C (SIGINT)
- Clean process management

## Project Structure

```
.
├── main.c          # Main entry point and shell loop
├── shell.h         # Header file with structures and declarations
├── shell.c         # Core shell functionality
├── parser.c        # Command parsing and preprocessing
├── commands.c      # Built-in command implementations
└── Makefile        # Build configuration
```

## Building the Project

1. Clone the repository
2. Run make:
```bash
make
```

3. Clean build files:
```bash
make clean
```

## Usage

1. Start the shell:
```bash
./myshell
```

2. Example commands:
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

## Implementation Details

### Command Parsing
- Recursive preprocessing of commands
- Support for complex command structures
- Proper handling of spaces and special characters

### Process Management
- Fork-exec model for command execution
- Proper pipe creation and management
- Signal handling and process cleanup

### Memory Management
- Proper allocation and deallocation of resources
- Clean handling of file descriptors
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
- Add job control support
- Implement command history
- Add command aliases
- Support environment variable expansion
- Add command substitution
- Implement more built-in commands
- Add support for command completion

## Contributing
Feel free to submit issues and enhancement requests!

## License
This project is open source and available under the MIT License. 