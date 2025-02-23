#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // here: Para manejar la redirección de archivos

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
	char  line[MAX_INPUT_SIZE]; // here: se almacena la entrada
	char **tokens;                 
	int i;
	int redirect = 0; // here: bandera para detectar redirección
	char *output_file = NULL; // here: almacena el archivo de salida

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		// read input
		printf("minersh$ ");

		fgets(line, sizeof(line), stdin);

		// here: si el usuario solo presiona enter, se vuelve a pedir entrada
		if (strlen(line) == 1) continue;

		/* END: TAKING INPUT */

		line[strlen(line)] = '\0'; //terminate with new line

		tokens = tokenize(line); // tokenize the input "me"

		// here: revisar si hay redirección de salida '>'
		for (i = 0; tokens[i] != NULL; i++) { 
			if (strcmp(tokens[i], ">") == 0) { // here: detectar el símbolo '>'
				redirect = 1; // here: activar la bandera de redirección
				output_file = tokens[i + 1]; // here: el siguiente token es el nombre del archivo
				tokens[i] = NULL; // here: eliminar '>' y el archivo de salida de los tokens
				break;
			}
		}

		// here: manejar el comando "exit"
        if (tokens[0] != NULL && strcmp(tokens[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

		// here: manejar el comando "cd" 
		if (tokens[0] != NULL && strcmp(tokens[0], "cd") == 0) {

			if (tokens[1] == NULL) {
				fprintf(stderr, "Shell: Incorrect command\n");
			} else {
				if (chdir(tokens[1]) != 0) {
					printf("Shell: Incorrect command\n");
				}
			}
			continue; // here: en el caso de cd no se necesita fork
		}

		// here: crear un proceso hijo
		pid_t pid = fork();

		// here: manejar error en fork
		if (pid < 0) {
 		   perror("fork failed");
		}

		if (pid == 0) { // here: proceso hijo
			// here: manejar redirección de salida si está activada
			if (redirect) {
				int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644); // here: abrir el archivo de salida
				if (fd < 0) {
					perror("Error opening file"); // here: mensaje de error si no se puede abrir
					exit(1);
				}
				dup2(fd, STDOUT_FILENO); // here: redirigir STDOUT al archivo
				dup2(fd, STDERR_FILENO); // here: redirigir STDERR al archivo
				close(fd); // here: cerrar el archivo después de redirigir
			}

			if (execvp(tokens[0], tokens) == -1) { // here: ejecutar comando
				printf("Error: The command could not be executed.\n");
			}
			exit(1);
		}

		else { // here: proceso padre, espera a que termine el hijo
			waitpid(pid, NULL, 0);
		}

		// here: liberar memoria para evitar fugas
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
