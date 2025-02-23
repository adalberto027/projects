#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // Task-A: Enabling File Redirection in your Shell, added for file operations // here 1

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
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
	char  line[MAX_INPUT_SIZE]; // here we save the input  
	char **tokens;                 
	int i;

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		// read input
		printf("minersh$ ");

		fgets(line, sizeof(line), stdin);

		// here if the user press enter it ask for a comand again "me"
		if (strlen(line) == 1) continue;

		/* END: TAKING INPUT */

		line[strlen(line)] = '\0'; //terminate with new line

		tokens = tokenize(line); // tokenize the input "me"

		//here we handel the exit command to close the shell "me"
        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

		//the next part handel the cd case, we have to handel it different because it is not an executable "me"
		if (tokens[0] != NULL && strcmp(tokens[0], "cd") == 0) {
			if (tokens[1] == NULL) {
				fprintf(stderr, "Shell: Incorrect command\n");
			} else {
				if (chdir(tokens[1]) != 0) {
					printf("Shell: Incorrect command\n");
				}
			}
			continue; // in the case of cd we dont hace to execute fork "me"
		}

		// Task-A: Enabling File Redirection in your Shell, checking for ">" in tokens // here 1
		int redirect_index = -1; // here 1
		for (i = 0; tokens[i] != NULL; i++) { // here 1
			if (strcmp(tokens[i], ">") == 0) { // here 1
				redirect_index = i; // here 1
				break; // here 1
			} // here 1
		} // here 1

		// Task-B: Enabling Inter-Process Communication via Pipes // here 2
		int pipe_index = -1; // here 2
		for (i = 0; tokens[i] != NULL; i++) { // here 2
			if (strcmp(tokens[i], "|") == 0) { // here 2
				pipe_index = i; // here 2
				break; // here 2
			} // here 2
		} // here 2

		if (pipe_index != -1) { // here 2
			tokens[pipe_index] = NULL; // here 2

			int fd[2]; // here 2
			if (pipe(fd) == -1) { // here 2
				perror("pipe failed"); // here 2
				continue; // here 2
			} // here 2

			pid_t pid1 = fork(); // here 2
			if (pid1 == 0) { // here 2
				close(fd[0]); // here 2
				dup2(fd[1], STDOUT_FILENO); // here 2
				close(fd[1]); // here 2
				if (execvp(tokens[0], tokens) == -1) { // here 2
					perror("execvp failed"); // here 2
					exit(1); // here 2
				} // here 2
			} // here 2

			pid_t pid2 = fork(); // here 2
			if (pid2 == 0) { // here 2
				close(fd[1]); // here 2
				dup2(fd[0], STDIN_FILENO); // here 2
				close(fd[0]); // here 2
				if (execvp(tokens[pipe_index + 1], &tokens[pipe_index + 1]) == -1) { // here 2
					perror("execvp failed"); // here 2
					exit(1); // here 2
				} // here 2
			} // here 2

			close(fd[0]); // here 2
			close(fd[1]); // here 2
			waitpid(pid1, NULL, 0); // here 2
			waitpid(pid2, NULL, 0); // here 2
			continue; // here 2
		} // here 2

		// this command create a child process "me"
		pid_t pid = fork();

		// if fork fails we print an error and we avoid shell to crash "me"
		if (pid < 0) {
 		   perror("fork failed");
		}

		if (pid == 0) { // child process "me"
			if (redirect_index != -1) { // here 1
				int fd = open(tokens[redirect_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644); // here 1
				dup2(fd, STDOUT_FILENO); // here 1
				dup2(fd, STDERR_FILENO); // here 1
				close(fd); // here 1
				tokens[redirect_index] = NULL; // here 1
			} // here 1

			if (execvp(tokens[0], tokens) == -1) {
				printf("Error: The command could not be executed.\n");
			}
			exit(1);
		}

		else {
			waitpid(pid, NULL, 0);
		}

		// Freeing the allocated memory to avoid memory leaks "me"
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
