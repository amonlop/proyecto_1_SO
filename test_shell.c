#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

void prompt() {
    printf("Proyecto Shell > $ ");
}

void leer_comando(char *buffer) {
    fgets(buffer, MAX_LINE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Le quita el \n al texto ingresado.
}

void separador(char *buffer, char **args) {
    int i = 0;
    args[i] = strtok(buffer, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
}

void ejecutar_comando(char **args) {
    int num_pipes = 0;
    int pipe_positions[MAX_ARGS];
    int i = 0;

    // Identifica las posiciones de los pipes.
    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            pipe_positions[num_pipes++] = i;
            args[i] = NULL; // Separa el comando cuando encuentre "|".
        }
        i++;
    }

    int pipefd[2 * num_pipes]; // Lectura/escritura para cada "|"
    for (i = 0; i < num_pipes; i++) { // Crea las pipes según cantidad de "|"
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    int j = 0;
    int inicio_cmd = 0;
    pid_t pid;
    for (i = 0; i <= num_pipes; i++) {
        pid = fork();
        if (pid == 0) {
            // Proceso hijo
            if (i != 0) {
                // Si no es el primer comando: (Primer comando no necesita redirigir
                // la entrada estándar)
                if (dup2(pipefd[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }
            if (i != num_pipes) {
                // Si no es el último comando: (Último comando no necesita redirigir
                // la salida estándar.)
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Cierra todos los fd de hijo.
            for (j = 0; j < 2 * num_pipes; j++) {
                close(pipefd[j]);
            }

            if (execvp(args[inicio_cmd], &args[inicio_cmd]) == -1) {
                perror("Proyecto Shell >");
                exit(1);
            }
        } else if (pid < 0) {
            // Error con fork.
            perror("fork");
            exit(1);
        }

        inicio_cmd = pipe_positions[i] + 1;
    }

    // Cierra todos los fd del padre.
    for (i = 0; i < 2 * num_pipes; i++) {
        close(pipefd[i]);
    }

    // Espera a hijos.
    for (i = 0; i <= num_pipes; i++) {
        wait(NULL);
    }
}

int main() {
    char buffer[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        prompt();
        leer_comando(buffer);

        if (strlen(buffer) == 0) {
            continue; // Caso de que no se ingrese nada.
        }

        separador(buffer, args);

        if (strcmp(args[0], "exit") == 0) {
            break; // Caso en que quiera salir del programa.
        }

        ejecutar_comando(args);
    }

    return 0;
}
