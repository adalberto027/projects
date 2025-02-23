#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h> // here: Para manejar la redirección de archivos, here2: Necesario para redirigir pipes

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// here3: Definir mensaje de error estándar
#define ERROR_MESSAGE "An error has occurred\n"

// here3: Función para imprimir errores
void print_error() {
    write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE)); // here3: Mensaje de error a STDERR
}

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
	int pipe_found = 0; // here2: bandera para detectar pipes
	char **cmd1 = NULL, **cmd2 = NULL; // here2: Arrays para los comandos antes y después del pipe

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		// read input
		printf("minersh$ ");

		fgets(line, sizeof(line), stdin);

		// here: si el usuario solo presiona enter, se vuelve a pedir entrada, here2: Verifica entrada vacía
		if (strlen(line) == 1) continue;

		/* END: TAKING INPUT */

		line[strlen(line)] = '\0'; //terminate with new line

		tokens = tokenize(line); // tokenize the input "me"

		// here3: Manejo de errores si la tokenización falla
		if (tokens == NULL) {
			print_error(); // here3: Usar la función de error
			continue;
		}

		// here2: verificar si hay un pipe '|'
		for (i = 0; tokens[i] != NULL; i++) { 
			if (strcmp(tokens[i], "|") == 0) { // here2: detectar el símbolo '|'
				pipe_found = 1; // here2: activar la bandera de pipe
				tokens[i] = NULL; // here2: separar ambos comandos
				cmd1 = tokens; // here2: primer comando antes del pipe
				cmd2 = &tokens[i + 1]; // here2: segundo comando después del pipe
				break;
			}
		}

		// here: revisar si hay redirección de salida '>', here2: Ahora lo hace solo si no hay pipe
		if (!pipe_found) { // here2: Solo procesar redirección si no hay pipe
			for (i = 0; tokens[i] != NULL; i++) { 
				if (strcmp(tokens[i], ">") == 0) { // here: detectar el símbolo '>', here2: Procesar redirección
					if (tokens[i + 1] == NULL) { // here3: Verificar si hay un archivo después de '>'
						print_error(); // here3: Mensaje de error si falta el archivo
						break;
					}
					redirect = 1; // here: activar la bandera de redirección
					output_file = tokens[i + 1]; // here: el siguiente token es el nombre del archivo
					tokens[i] = NULL; // here: eliminar '>' y el archivo de salida de los tokens
					break;
				}
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
				print_error(); // here3: Mensaje de error si falta un argumento
			} else {
				if (chdir(tokens[1]) != 0) {
					print_error(); // here3: Mensaje de error si `chdir` falla
				}
			}
			continue; // here: en el caso de cd no se necesita fork
		}

		if (pipe_found) { // here2: Manejar el caso de pipes
			int pipefd[2]; // here2: Crear un pipe
			if (pipe(pipefd) == -1) { // here3: Verificar error en `pipe()`
				print_error();
				continue;
			}

			pid_t pid1 = fork(); // here2: Crear primer hijo

			if (pid1 < 0) { // here3: Verificar error en `fork()`
				print_error();
				continue;
			}

			if (pid1 == 0) { // here2: Proceso hijo para cmd1
				close(pipefd[0]); // here2: Cerrar lectura del pipe
				dup2(pipefd[1], STDOUT_FILENO); // here2: Redirigir STDOUT al pipe
				close(pipefd[1]); // here2: Cerrar escritura del pipe
				if (execvp(cmd1[0], cmd1) == -1) { // here2: Ejecutar cmd1
					print_error(); // here3: Error en ejecución
					exit(1);
				}
			}

			pid_t pid2 = fork(); // here2: Crear segundo hijo

			if (pid2 < 0) { // here3: Verificar error en `fork()`
				print_error();
				continue;
			}

			if (pid2 == 0) { // here2: Proceso hijo para cmd2
				close(pipefd[1]); // here2: Cerrar escritura del pipe
				dup2(pipefd[0], STDIN_FILENO); // here2: Redirigir STDIN al pipe
				close(pipefd[0]); // here2: Cerrar lectura del pipe
				if (execvp(cmd2[0], cmd2) == -1) { // here2: Ejecutar cmd2
					print_error(); // here3: Error en ejecución
					exit(1);
				}
			}

			close(pipefd[0]); // here2: Cerrar pipe en proceso padre
			close(pipefd[1]); // here2: Cerrar pipe en proceso padre
			waitpid(pid1, NULL, 0); // here2: Esperar primer hijo
			waitpid(pid2, NULL, 0); // here2: Esperar segundo hijo
		} 
		
		else { // here2: Manejar comandos sin pipes
			pid_t pid = fork(); // here: crear un proceso hijo, here2: Solo si no hay pipes

			if (pid < 0) { // here3: Verificar error en `fork()`
				print_error();
				continue;
			}

			if (pid == 0) { // here: proceso hijo
				if (execvp(tokens[0], tokens) == -1) { // here: ejecutar comando
					print_error(); // here3: Error en ejecución
				}
				exit(1);
			}

			else { // here: proceso padre, espera a que termine el hijo
				waitpid(pid, NULL, 0);
			}
		}

		// here: liberar memoria para evitar fugas
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
