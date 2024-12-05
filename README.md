# Mini_Project_4

# Unix Shell Project: Custom Shell Implementation

## Project Description
This project is a custom shell implementation for Unix-based systems. It supports basic shell commands, redirection, piping, background processes, and error handling, providing users with a simplified interface for interacting with the operating system. The shell allows users to execute commands, redirect input/output, pipe commands together, and manage background processes, all while handling errors gracefully.

## Features
- Execute basic Unix commands (e.g., `ls`, `pwd`, `echo`)
- Support for input/output redirection (`>` and `>>`)
- Implementation of piping (`|`) between commands
- Background process handling using `&`
- Error handling for invalid commands and incorrect syntax
- Interactive mode for direct user input
- Built-in support for changing directories (`cd`) and exiting the shell (`exit`)

## Installation
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

2. Once inside the shell, you can enter commands like:
- ls: List files in the current directory.
- pwd: Print the working directory.
- echo Hello, World!: Print text to the terminal.
- cd /path/to/directory: Change the current directory.
- exit: Exit the shell.

3. You can also use redirection, piping, and background processes:
- Redirection: echo Hello > output.txt
- Piping: echo Hello | grep Hello
- Background processes: sleep 5

## Project Structure
The project currently consists of the following file:
- hannah.c: Contains the main shell implementation with features for command execution, built-in commands, redirection, piping, and background process handling.
- README.md: Project description, setup, and usage instructions.

## Authors
- Hannah Friday
- Dallas Desimone
- Breanna Holloway
- Jacob Zukas
