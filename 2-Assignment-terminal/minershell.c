#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // Task-A: Enabling File Redirection in your Shell, added for file operations // here 1

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// Function to print the unique error message // here 3
void print_error() { // here 3
    fprintf(stderr, "An error has occurred\n"); // here 3
} // here 3

char **tokenize(char *line) {
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++) {
        char readChar = line[i];
        if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0) {
                tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        } else {
            token[tokenIndex++] = readChar;
        }
    }
    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

int main(int argc, char *argv[]) {
    char line[MAX_INPUT_SIZE];
    char **tokens;
    int i;

    while (1) {
        bzero(line, sizeof(line));
        printf("minersh$ ");
        fgets(line, sizeof(line), stdin);
        if (strlen(line) == 1) continue;
        line[strlen(line)] = '\0';
        tokens = tokenize(line);

        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        if (tokens[0] != NULL && strcmp(tokens[0], "cd") == 0) {
            if (tokens[1] == NULL || chdir(tokens[1]) != 0) {
                print_error(); // here 3
            }
            continue;
        }

        int redirect_index = -1, pipe_index = -1;
        for (i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], ">") == 0) {
                redirect_index = i;  // Bug: Identificar correctamente el operador ">" en comandos con pipes
            } else if (strcmp(tokens[i], "|") == 0) {
                pipe_index = i;  // Bug: Identificar correctamente el operador "|"
            }
        }

        if (pipe_index != -1) {  
            tokens[pipe_index] = NULL;

            int fd[2];
            if (pipe(fd) == -1) {
                print_error(); // here 3
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {  
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                if (execvp(tokens[0], tokens) == -1) {
                    print_error(); // here 3
                    exit(1);
                }
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {  
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);

                if (redirect_index != -1) {  
                    int fd_out = open(tokens[redirect_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);  
                    if (fd_out == -1) {
                        print_error(); // here 3
                        exit(1);
                    }
                    dup2(fd_out, STDOUT_FILENO);  
                    close(fd_out);
                    tokens[redirect_index] = NULL;  // Bug: Evitar que ">" sea interpretado como argumento
                }

                if (execvp(tokens[pipe_index + 1], &tokens[pipe_index + 1]) == -1) {
                    print_error(); // here 3
                    exit(1);
                }
            }

            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            print_error(); // here 3
        }

        if (pid == 0) {
            if (redirect_index != -1) {
                int fd = open(tokens[redirect_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    print_error(); // here 3
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                tokens[redirect_index] = NULL;
            }

            if (execvp(tokens[0], tokens) == -1) {
                print_error(); // here 3
            }
            exit(1);
        } else {
            waitpid(pid, NULL, 0);
        }

        for (i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }
    return 0;
}
