#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // Task 1: Required for handling file redirection

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens */
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE]; // Here we store the input
	char **tokens;                 
	int i;

	while(1) {            
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		// Read input
		printf("minersh$ ");
		fgets(line, sizeof(line), stdin);

		// If the user presses enter, prompt again
		if (strlen(line) == 1) continue;

		/* END: TAKING INPUT */
		line[strlen(line)] = '\0'; // Terminate with a new line
		tokens = tokenize(line); // Tokenize the input

		// Handle the exit command to close the shell
		if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
			printf("Exiting shell...\n");
			break;
		}

		// Task 1: Output redirection to a file
		int redirect_output = 0;
		char *output_file = NULL;
		for (i = 0; tokens[i] != NULL; i++) {
			if (strcmp(tokens[i], ">") == 0) {
				redirect_output = 1;
				output_file = tokens[i + 1];
				tokens[i] = NULL; // Remove redirection operator
				break;
			}
		}

		// Task 2: Handling pipes
		int pipefd[2];
		int use_pipe = 0;
		char **command2 = NULL;
		for (i = 0; tokens[i] != NULL; i++) {
			if (strcmp(tokens[i], "|") == 0) {
				use_pipe = 1;
				pipe(tokens);
				tokens[i] = NULL;
				command2 = &tokens[i + 1];
				break;
			}
		}

		// Create a child process
		pid_t pid = fork();

		// PID > 0 -> child process, PID == 0 -> Parent process, PID < 0 -> Error in fork()
		// If fork fails, print an error and prevent shell crash
		if (pid < 0) {
			perror("fork failed");
		}

		if (pid == 0) { // Child process
			if (redirect_output) { // Task 1: Output redirection
				int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (fd < 0) {
					perror("open");
					exit(1);
				}
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				close(fd);
			}
			if (use_pipe) { // Task 2: Pipe handling
				dup2(pipefd[1], STDOUT_FILENO);
				close(pipefd[0]);
				close(pipefd[1]);
			}
			if (execvp(tokens[0], tokens) == -1) {
				printf("Error: The command could not be executed.\n");
			}
			exit(1);
		}
		else { // Parent process
			if (use_pipe) { // Task 2: Pipe handling in parent process
				pid_t pid2 = fork();
				if (pid2 == 0) {
					dup2(pipefd[0], STDIN_FILENO);
					close(pipefd[1]);
					close(pipefd[0]);
					if (execvp(command2[0], command2) == -1) {
						printf("Error: The second command could not be executed.\n");
					}
					exit(1);
				}
				close(pipefd[0]);
				close(pipefd[1]);
				waitpid(pid2, NULL, 0);
			}
			waitpid(pid, NULL, 0);
		}

		// Free allocated memory to avoid memory leaks    
		for(i=0; tokens[i] != NULL; i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}