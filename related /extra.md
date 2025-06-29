# Custom Shell Implementation

A lightweight shell implementation in C that mimics core functionalities of bash/zsh, built from scratch. This project demonstrates system programming skills, process management, and low-level C development.

## Project Overview

This shell implementation provides a command-line interface with features like:
- Basic command execution
- Pipeline operations
- I/O redirection
- Built-in commands
- Signal handling

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

## Project Structure
```
.
├── main.c          # Shell loop and signal handling
├── shell.h         # Structures and declarations
├── shell.c         # Core shell functionality
├── parser.c        # Command parsing
├── commands.c      # Built-in commands
└── Makefile        # Build configuration
```

## Building and Running

1. Compile the project:
```bash
make
```

2. Run the shell:
```bash
./myshell
```

3. Clean build files:
```bash
make clean
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

## Interview Questions and Answers

### Basic Concepts

1. **Q: What is a shell and why did you build one?**
   ```
   A: A shell is a command-line interface that allows users to interact with the operating system. I built one to:
   - Understand system programming concepts
   - Learn about process management
   - Practice low-level C programming
   - Implement complex features like pipelines
   ```

2. **Q: How does your shell handle command execution?**
   ```
   A: The shell uses the fork-exec model:
   1. fork() creates a copy of the current process
   2. exec() replaces the child process with the new command
   3. Parent process waits for child completion
   This allows running external commands while keeping the shell running.
   ```

### Advanced Features

3. **Q: Explain how pipeline implementation works in your shell.**
   ```
   A: Pipeline implementation involves:
   1. Creating pipes between commands using pipe()
   2. Connecting processes using dup2()
   3. Managing file descriptors
   4. Handling process synchronization
   Example: ls | grep .txt
   - ls output goes to pipe
   - grep reads from pipe
   - Both processes run concurrently
   ```

4. **Q: How do you handle I/O redirection?**
   ```
   A: I/O redirection is implemented by:
   1. Opening files with appropriate modes
   2. Using dup2() to redirect file descriptors
   3. Managing file permissions
   4. Handling errors gracefully
   Example: ls > output.txt
   - Opens output.txt for writing
   - Redirects stdout to the file
   - Executes ls command
   ```

### Technical Challenges

5. **Q: What was the most challenging part to implement?**
   ```
   A: The most challenging aspects were:
   1. Pipeline implementation
   2. Process synchronization
   3. Memory management
   4. Error handling
   Each required careful consideration of system resources and edge cases.
   ```

6. **Q: How do you handle errors in your shell?**
   ```
   A: Error handling includes:
   1. System call error checking
   2. Process creation errors
   3. File operation errors
   4. Memory allocation errors
   Each error is properly reported and resources are cleaned up.
   ```

### System Programming

7. **Q: Explain the fork-exec model in detail.**
   ```
   A: The fork-exec model works in two steps:
   1. fork():
      - Creates exact copy of parent process
      - Returns different values to parent/child
      - Shares resources initially
   2. exec():
      - Replaces child process with new program
      - Maintains same PID
      - Inherits file descriptors
   ```

8. **Q: How do you manage file descriptors?**
   ```
   A: File descriptor management involves:
   1. Proper opening/closing of files
   2. Using dup2() for redirection
   3. Closing unused descriptors
   4. Handling pipe descriptors
   This prevents resource leaks and ensures proper I/O.
   ```

### Memory Management

9. **Q: How do you prevent memory leaks?**
   ```
   A: Memory leak prevention includes:
   1. Proper allocation/deallocation
   2. Resource cleanup in error cases
   3. File descriptor management
   4. Process cleanup
   Each allocated resource is tracked and freed.
   ```

10. **Q: What data structures did you use and why?**
    ```
    A: Main data structures:
    1. Command structure:
       - Stores command name
       - Arguments array
       - I/O redirection info
    2. Pipeline structure:
       - Array of commands
       - Pipe connections
    These structures allow efficient command processing.
    ```

### Signal Handling

11. **Q: How do you handle Ctrl+C in your shell?**
    ```
    A: Ctrl+C handling involves:
    1. Catching SIGINT signal
    2. Printing new prompt
    3. Not terminating shell
    4. Cleaning up child processes
    This makes the shell more user-friendly.
    ```

12. **Q: What other signals does your shell handle?**
    ```
    A: The shell handles:
    1. SIGINT (Ctrl+C)
    2. SIGCHLD (child process termination)
    3. SIGTERM (termination request)
    Each signal is handled appropriately.
    ```

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

3. Additional Built-in Commands
   - More shell utilities
   - Custom commands
   - Enhanced process management

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is open source and available under the MIT License.