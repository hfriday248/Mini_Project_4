#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Define constants

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define ERROR_MESSAGE "The error occurred\n"

// Globals

char *path[MAX_ARGS] = {"/bin", NULL};

// Function prototypes

void execute_command(char **args);
int is_builtin_command(char **args);
void run_builtin_command(char **args);
void redirect_output(char **args);
void parse_input(char *line, char **args);
int main(int argc, char *argv[]) {
    FILE *input = stdin;  // Default to interactive mode
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // Check command-line arguments
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
            printf("hannah> ");
    }
    
    // Read input
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
            execute_command(args);
        }
    }
    fclose(input);
    free(line);
    return 0;
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
void execute_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        redirect_output(args);
        for (int i = 0; path[i] != NULL; i++) {
            char command[MAX_INPUT];
            snprintf(command, sizeof(command), "%s/%s", path[i], args[0]);
            execv(command, args);
        }
        // If execv fails
        fprintf(stderr, ERROR_MESSAGE);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        wait(NULL);
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
        }
    } else if (strcmp(args[0], "path") == 0) {
        for (int i = 1; i < MAX_ARGS && args[i] != NULL; i++) {
            path[i - 1] = args[i];
        }
        path[MAX_ARGS - 1] = NULL;
    } else if (strcmp(args[0], "loop") == 0) {
        if (args[1] == NULL || args[2] == NULL || atoi(args[1]) <= 0) {
            fprintf(stderr, ERROR_MESSAGE);
        } else {
            int loop_count = atoi(args[1]);
            for (int i = 1; i <= loop_count; i++) {
                char *loop_args[MAX_ARGS];
                for (int j = 2; args[j] != NULL; j++) {
                    loop_args[j - 2] = args[j];
                }
                loop_args[MAX_ARGS - 1] = NULL;
                execute_command(loop_args);
            }
        }
    }
}

// Handle built-in commands
int is_builtin_command(char **args) {
    return strcmp(args[0], "exit") == 0 ||
           strcmp(args[0], "cd") == 0 ||
           strcmp(args[0], "path") == 0 ||
           strcmp(args[0], "loop") == 0;
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
