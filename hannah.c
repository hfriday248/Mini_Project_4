#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define ERROR_MESSAGE "The error occurred\n"

char *path[MAX_ARGS] = {"/bin", NULL};

void execute_command(char **args, int background);
int is_builtin_command(char **args);
void run_builtin_command(char **args);
void redirect_output(char **args);
void parse_input(char *line, char **args);
void update_prompt();
void handle_signals(int sig);

char current_directory[MAX_INPUT] = ""; // To hold the current directory path

int main(int argc, char *argv[]) {
    FILE *input = stdin;  // Default to interactive mode
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // Set up signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, handle_signals);

    if (argc > 2) {
        fprintf(stderr, ERROR_MESSAGE);
        exit(1);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {
            fprintf(stderr, ERROR_MESSAGE);
            exit(1);
        }
    }

    // Shell loop
    while (1) {
        if (input == stdin) {
            update_prompt(); // Update prompt with current directory
            printf("%s> ", current_directory); // Display the current directory as part of the prompt
        }
        
        nread = getline(&line, &len, input);
        if (nread == -1) {  // EOF
            free(line);
            exit(0);
        }

        // Parse input into arguments
        char *args[MAX_ARGS] = {NULL};
        parse_input(line, args);

        // Skip empty lines
        if (args[0] == NULL) {
            continue;
        }

        // Check for built-in commands
        if (is_builtin_command(args)) {
            run_builtin_command(args);
        } else {
            int background = 0;
            // Check if the command should run in the background
            if (args[0] != NULL && strcmp(args[MAX_ARGS-2], "&") == 0) {
                args[MAX_ARGS-2] = NULL; // Remove "&" from args
                background = 1;
            }
            execute_command(args, background);
        }
    }
    fclose(input);
    free(line);
    return 0;
}

// Update the current directory and the prompt
void update_prompt() {
    if (getcwd(current_directory, sizeof(current_directory)) == NULL) {
        fprintf(stderr, ERROR_MESSAGE);
        exit(1);
    }
}

// Parse input line into arguments
void parse_input(char *line, char **args) {
    char *token = strtok(line, " \t\n");
    int index = 0;
    while (token != NULL) {
        args[index++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[index] = NULL;
}

// Execute non-built-in commands
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        redirect_output(args);

        // Handle piping
        int pipefd[2];
        pid_t pid2;
        char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
        int pipe_index = -1;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "|") == 0) {
                pipe_index = i;
                break;
            }
        }

        if (pipe_index != -1) {
            args[pipe_index] = NULL; // Split the command at "|"
            for (int i = 0; i < pipe_index; i++) cmd1[i] = args[i];
            for (int i = pipe_index + 1; args[i] != NULL; i++) cmd2[i - pipe_index - 1] = args[i];

            pipe(pipefd);  // Create pipe

            pid2 = fork();
            if (pid2 == 0) {
                // Second child for the second command
                close(pipefd[0]); // Close read end
                dup2(pipefd[1], STDOUT_FILENO); // Redirect output to pipe
                execvp(cmd2[0], cmd2);
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            } else if (pid2 > 0) {
                // First child for the first command
                close(pipefd[1]); // Close write end
                dup2(pipefd[0], STDIN_FILENO); // Redirect input from pipe
                execvp(cmd1[0], cmd1);
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            } else {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }
        } else {
            for (int i = 0; path[i] != NULL; i++) {
                char command[MAX_INPUT];
                snprintf(command, sizeof(command), "%s/%s", path[i], args[0]);
                execv(command, args);
            }
            fprintf(stderr, ERROR_MESSAGE);
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        if (!background) {
            wait(NULL);
        }
    } else {
        // Fork failed
        fprintf(stderr, ERROR_MESSAGE);
    }
}

// Check if command is a built-in command
void run_builtin_command(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        if (args[1] != NULL) {
            fprintf(stderr, ERROR_MESSAGE);
        } else {
            exit(0);
        }
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL || chdir(args[1]) != 0) {
            fprintf(stderr, ERROR_MESSAGE);
        } else {
            update_prompt(); // Update the prompt after changing the directory
        }
    } else if (strcmp(args[0], "path") == 0) {
        for (int i = 1; i < MAX_ARGS && args[i] != NULL; i++) {
            path[i - 1] = args[i];
        }
        path[MAX_ARGS - 1] = NULL;
    }
}

// Handle background processes
void handle_signals(int sig) {
    if (sig == SIGINT) {
        // Do nothing on SIGINT to avoid killing the shell (Ctrl+C)
    }
}

// Handle output redirection
void redirect_output(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }
}

// Check if command is a built-in command
int is_builtin_command(char **args) {
    return strcmp(args[0], "exit") == 0 ||
           strcmp(args[0], "cd") == 0 ||
           strcmp(args[0], "path") == 0;
}


// Handle output redirection
void redirect_output(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }
}
