#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_COMANDOS 100

void ejecutar_comando(char *comando) {
    char *args[MAX_COMANDOS];
    int i = 0;
    args[i] = strtok(comando, " \n");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \n");
    }

    if (args[0] == NULL) return;

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    int pid = fork();
    if (pid < 0) {
        perror("fork falló");
        exit(1);
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("execvp falló");
        exit(1);
    } else {
        wait(NULL);
    }
}

int main() {
    char linea[1024];
    
    while (1) {
        printf("mishell:$ ");
        fflush(stdout);
        if (fgets(linea, sizeof(linea), stdin) == NULL) {
            perror("fgets falló");
            exit(1);
        }

        if (linea[0] == '\n') continue; // Si la línea está vacía, reinicia el prompt

        // Procesar los comandos separados por pipes
        char *comandos = strtok(linea, "|");
        while (comandos != NULL) {
            ejecutar_comando(comandos);
            comandos = strtok(NULL, "|");
        }
    }

    return 0;
}
