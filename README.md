# Mini_Project_4

## Unix Shell Project: Custom Shell Implementation

### Project Description
This project implements a custom Unix shell designed to provide a simplified interface for interacting with the operating system. The shell supports executing basic commands, redirecting input/output, piping commands, and managing background processes. It includes essential built-in commands like `cd`, `exit`, and `path`, allowing users to navigate directories, terminate the shell session, and modify the executable search path. This project also demonstrates basic error handling for invalid commands and improper syntax.

### Features
- **Command Execution**: Supports basic Unix commands (e.g., `ls`, `pwd`, `echo`).
- **Built-in Commands**:
  - `cd`: Change the current working directory.
  - `exit`: Terminate the shell session.
  - `path`: Modify executable search paths.
- **Redirection**: Handle output redirection using `>` or `>>`.
- **Piping**: Enable data flow between commands using `|`.
- **Background Processes**: Run commands asynchronously using `&`.
- **Looping**: Supports infinite loops and conditional loops for repeated command execution.
- **Interactive Mode**: Accept commands directly from the user.
- **Batch Mode**: Read and execute commands from a script file.
- **Error Handling**: Displays error messages for invalid commands and improper syntax.

### Installation
1. Clone this repository:
   ```bash
   git clone https://github.com/hfriday248/Mini_Project_4.git
 
2. Navigate to the project directory
   cd Mini_Project_4

3. Compile the project
   gcc -std=c99 -D_POSIX_C_SOURCE=200809L -o hannah hannah.c

## Usage
1. Start the shell by running
   ./hannah

2. Using Commands
   Enter commands directly into the shell. Examples:
- ls: List files in the current directory.
- pwd: Print the current working directory.
- echo "Hello, World!": Print text to the terminal.
- cd /path/to/directory: Change the current directory.
- Rexit: Exit the shell.

3. Advanced Features
- Redirection: echo Hello > output.txt
- Piping: ls | grep filename
- Background processes: sleep 10 &
- Batch Mode: ./hannah commands.txt

## Project Structure
The project currently consists of the following files:
- hannah.c: Contains the main shell implementation with features for command execution, built-in commands, redirection, piping, and background process management.
- README.md: Project description, setup, and usage instructions.

## Authors
- Hannah Friday
