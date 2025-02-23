#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // For file redirection

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define ERROR_MESSAGE "An error has occurred\n"

// Function to print errors consistently
void print_error() {
    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
}

// Tokenize the input line
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
    int redirect = 0; 
    char *output_file = NULL; 

    while (1) { 
        printf("minersh$ ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break; // Handle Ctrl+D exit
        }

        if (strlen(line) == 1) continue;
        line[strlen(line) - 1] = '\0'; // Remove newline character

        tokens = tokenize(line);

        // Handle exit command
        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        // Handle cd command
        if (tokens[0] != NULL && strcmp(tokens[0], "cd") == 0) {
            if (tokens[1] == NULL || chdir(tokens[1]) != 0) {
                print_error();
            }
            continue;
        }

        // Check for output redirection `>`
        for (i = 0; tokens[i] != NULL; i++) {
            if (strcmp(tokens[i], ">") == 0) {
                if (tokens[i + 1] == NULL) {
                    print_error(); // Missing filename after `>`
                    break;
                }
                redirect = 1;
                output_file = tokens[i + 1];
                tokens[i] = NULL; // Remove `>` from tokens
                break;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            print_error();
            continue;
        }

        if (pid == 0) { // Child process
            if (redirect) {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    print_error();
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO); // Redirect stdout to file
                close(fd);
            }

            if (execvp(tokens[0], tokens) == -1) {
                print_error();
                exit(1);
            }
        } else {
            waitpid(pid, NULL, 0);
        }

        // Free allocated memory
        for (i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(tokens);
    }

    return 0;
}
