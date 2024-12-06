#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100
#define ERROR_MESSAGE "The error occurred\n"

char *path[MAX_ARGS] = {"/bin", NULL};

void parse_input(char *line, char **args);
void execute_command(char **args, int background);
int is_builtin_command(char **args);
void run_builtin_command(char **args);
void redirect_output(char **args);
void handle_pipes(char **args, int background);
int is_background_command(char **args);

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    char *line = NULL;
    size_t len = 0;

    if (argc > 2) {
        fprintf(stderr, ERROR_MESSAGE);
        exit(1);
    } else if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, ERROR_MESSAGE);
            exit(1);
        }
    }

    while (1) {
        if (input == stdin) {
            printf("hannah> ");
        }

        ssize_t nread = getline(&line, &len, input);
        if (nread == -1) {
            free(line);
            fclose(input);
            exit(0);
        }

        char *args[MAX_ARGS] = {NULL};
        parse_input(line, args);

        if (args[0] == NULL) {
            continue;
        }

        int background = is_background_command(args);

        if (is_builtin_command(args)) {
            run_builtin_command(args);
        } else {
            execute_command(args, background);
        }
    }

    free(line);
    fclose(input);
    return 0;
}

void parse_input(char *line, char **args) {
    char *token = strtok(line, " \t\n");
    int index = 0;
    while (token != NULL) {
        args[index++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[index] = NULL;
}

void execute_command(char **args, int background) {
    pid_t pid = fork();

    if (pid == 0) {
        redirect_output(args);
        handle_pipes(args, background);
        execvp(args[0], args);
        perror(ERROR_MESSAGE);
        exit(1);
    } else if (pid > 0) {
        if (!background) {
            wait(NULL);
        }
    } else {
        perror(ERROR_MESSAGE);
    }
}

void handle_pipes(char **args, int background) {
    int pipefds[2 * MAX_ARGS];  // To hold multiple pipe file descriptors
    int i = 0;
    int j = 0;

    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            pipe(pipefds + j);
            pid_t pid = fork();
            if (pid == 0) {
                dup2(pipefds[j + 1], STDOUT_FILENO);  // Write to the pipe
                close(pipefds[j]);
                close(pipefds[j + 1]);
                args[i] = NULL;  // End the current command at the pipe
                execvp(args[0], args);
                perror(ERROR_MESSAGE);
                exit(1);
            } else {
                wait(NULL);
                dup2(pipefds[j], STDIN_FILENO);  // Read from the previous pipe
                close(pipefds[j]);
                close(pipefds[j + 1]);
                i++;
                j += 2;  // Move to the next pipe pair
            }
        } else {
            i++;
        }
    }
}

int is_background_command(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL; // Remove "&" from args
            return 1;
        }
    }
    return 0;
}

int is_builtin_command(char **args) {
    return strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0;
}

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
        int i = 1;
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            path[i - 1] = args[i];
            i++;
        }
        path[i - 1] = NULL;
    }
}

void redirect_output(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL || args[i + 2] != NULL) {
                fprintf(stderr, ERROR_MESSAGE);
                exit(1);
            }

            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(ERROR_MESSAGE);
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }
}
