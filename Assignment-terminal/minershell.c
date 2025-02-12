#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
	// Tokenizar input
	char **tokens = tokenize(line);          
	int i;


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		// read input
		printf("$ ");

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
				fprintf(stderr, "cd: missing argument\n");
			} else {
				if (chdir(tokens[1]) != 0) {
					printf("Error: cd failed\n");
				}
			}
			continue; // in the case of cd we dont hace to execute fork "me"
		}

		// this command create a child process "me"
		pid_t pid = fork();

		// PID > 0 -> child process, PID == 0 -> Parent process, PID < 0 -> Error at fork() "me"

		// if fork fails we print an error and we avoid shell to crash "me"
		if (pid < 0) {
 		   perror("fork failed");
		}

		if (pid == 0) { // child process "me"
			if (execvp(tokens[0], tokens) == -1) {
				printf("Error: The command could not be executed.\n");
			}
			exit(1);
		}

		else { // whit this we ensures the parent process waits for the child to finish, preventing zombie processes. "me"
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
