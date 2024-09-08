#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
#define MAX_FAVS 100

char favoritos_path[MAX_LINE] = "";
char recordatorio[MAX_LINE] = "";

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
            exit(EXIT_FAILURE);
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
                    exit(EXIT_FAILURE);
                }
            }
            if (i != num_pipes) {
                // Si no es el último comando: (Último comando no necesita redirigir
                // la salida estándar.)
                if (dup2(pipefd[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Cierra todos los fd de hijo.
            for (j = 0; j < 2 * num_pipes; j++) {
                close(pipefd[j]);
            }

            if (execvp(args[inicio_cmd], &args[inicio_cmd]) == -1) {
                perror("Proyecto Shell >");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            // Error con fork.
            perror("fork");
            exit(EXIT_FAILURE);
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

// SECCIÓN DE FAVORITOS

typedef struct {
    int id;
    char comando[MAX_LINE];
} Favorito;

Favorito favoritos[MAX_FAVS];
int num_favoritos = 0;


void favs_agregar(char *comando) {
    if (strlen(comando) == 0) {
        printf("No se puede agregar un comando vacío a favoritos.\n");
        return;
    }
    for (int i = 0; i < num_favoritos; i++) {
        if (strcmp(favoritos[i].comando, comando) == 0) {
            printf("El comando ya está en la lista de favoritos.\n");
            return;
        }
    }
    if (num_favoritos == MAX_FAVS) {
        printf("No se pueden agregar más favoritos.\n");
        return;
    }
    favoritos[num_favoritos].id = num_favoritos + 1;
    strcpy(favoritos[num_favoritos].comando, comando);
    num_favoritos++;
    printf("Comando agregado a favoritos: %s\n", comando);
}

void favs_crear(char *path) {
    strcpy(favoritos_path, path);
    FILE *file = fopen(favoritos_path, "w");
    if (file == NULL) {
        perror("Error al crear archivo fav");
        return;
    } fclose(file);
}

void favs_mostrar() {
    if (num_favoritos == 0) {
        printf("No hay favoritos guardados.\n");
        return;
    }
    printf("Lista de favoritos:\n");
    for (int i = 0; i < num_favoritos; i++) {
        printf("%d: %s\n", favoritos[i].id, favoritos[i].comando);
    }
}

void favs_eliminar(char *numeros) {
    char *numero = strtok(numeros, ",");
    while (numero != NULL) {
        int id = atoi(numero);
        if (id <= 0 || id > num_favoritos) {
            printf("ID %d no es válido.\n", id);
            numero = strtok(NULL, ",");
            continue;
        }
        for (int i = 0; i < num_favoritos; i++) {
            if (favoritos[i].id == id) {
                printf("Eliminando favorito: %s\n", favoritos[i].comando);
                for (int j = i; j < num_favoritos - 1; j++) {
                    favoritos[j] = favoritos[j + 1];
                }
                num_favoritos--;
                break;
            }
        }
        numero = strtok(NULL, ",");
    }
    for (int i = 0; i < num_favoritos; i++) {
        favoritos[i].id = i + 1;
    }
}

void favs_buscar(char *comando) {
    for (int i = 0; i < num_favoritos; i++) { // Pasa por todos los comandos de favoritos, y muestra los que tienen el comando ingresado.
        if (strstr(favoritos[i].comando, comando) != NULL) {
            printf("%d: %s\n", favoritos[i].id, favoritos[i].comando);
        }
    }
}

void favs_borrar(){
    num_favoritos = 0;
}

void favs_ejecutar(int id) {
    for (int i = 0; i < num_favoritos; i++) {
        if (favoritos[i].id == id) {
            pid_t pid = fork();
            if (pid == 0) { // Proceso hijo
                char *args[MAX_ARGS];
                separador(favoritos[i].comando, args);
                ejecutar_comando(args); // Ejecuta el comando en un hijo
                exit(0);
            } else if (pid > 0) {
                wait(NULL); // Padre espera a que el hijo termine
            } else {
                perror("fork");
            }
            return;
        }
    }
    printf("Comando no encontrado.\n");
}

void favs_cargar(char *path) {

    strcpy(favoritos_path, path);
    FILE *file = fopen(favoritos_path, "r");
    if (file == NULL) {
        perror("Error al abrir archivo fav");
        return;
    }
    num_favoritos = 0;
    while (fgets(favoritos[num_favoritos].comando, MAX_LINE, file) != NULL) { // Lee los comandos del archivo y los guarda en la lista de favoritos.
        favoritos[num_favoritos].comando[strcspn(favoritos[num_favoritos].comando, "\n")] = '\0';
        favoritos[num_favoritos].id = num_favoritos + 1;
        num_favoritos++;
    } fclose(file);
    favs_mostrar();
}

void favs_guardar() {
    if (strlen(favoritos_path) == 0) {
        printf("Todavía no se ha creado un archivo de favoritos.\n");
        return;
    }
    FILE *file = fopen(favoritos_path, "w"); // Modo 'w' para sobrescribir el archivo
    if (file == NULL) {
        perror("Error al abrir archivo fav");
        return;
    }
    for (int i = 0; i < num_favoritos; i++) {
        fprintf(file, "%s\n", favoritos[i].comando); // Escribe los comandos en el archivo
    }
    fclose(file); // Asegura que el archivo se cierra siempre
    printf("Favoritos se guardo correctamente.\n");
}

// FIN DE SECCIÓN DE FAVORITOS

// SECCIÓN DE RECORDATORIO

void sig_handler_recordatorio(int sig) {
    if(sig == SIGALRM) {
        printf("\nExpiró el tiempo: %s\n", recordatorio);
        fflush(stdout);
    }
}

void ejecutar_set(char **args) {
    if(strcmp(args[0], "set") == 0 && strcmp(args[1], "recordatorio") == 0) {
        if(args[2] != NULL) {
            int tiempo_esp = atoi(args[2]);

            if(tiempo_esp <= 0) {
                fprintf(stderr, "\nEl tiempo de espera debe ser un número positivo \n Proyecto Shell > $ ");
                exit(0);
                return;
            }
            if(args[3] != NULL) {
                strcpy(recordatorio, args[3]);
                int i = 4;
                while(args[i] != NULL) {
                    strcat(recordatorio, " ");
                    strcat(recordatorio, args[i]);
                    i++;
                }

                signal(SIGALRM, sig_handler_recordatorio);
                alarm(tiempo_esp);
                pause();
                exit(0);
                return;
            } else {
                fprintf(stderr, "\nFalta el mensaje de recordatorio \nProyecto Shell > $ ");
                exit(0);
                return;
            }
        } else {
            fprintf(stderr, "\nFalta el tiempo de espera \nProyecto Shell > $ ");
            exit(0);
            return;
        }
    }
    exit(0);
    return;
}

// FIN SECCIÓN RECORDATORIO

int main() {
    char buffer[MAX_LINE];
    char copybuffer[MAX_LINE];
    char *args[MAX_ARGS];
    while (1) {
        prompt();
        leer_comando(buffer);

        if (strlen(buffer) == 0) {
            continue; // Caso de que no se ingrese nada.
        }
        strcpy(copybuffer, buffer);
        separador(buffer, args);

        if (strcmp(args[0], "exit") == 0) {
            break; // Caso en que quiera salir del programa.
        }

        if(strcmp(args[0], "set") == 0) {
            if (args[1] == NULL) {
                printf("El comando es <<set recordatorio tiempo \"mensaje\">> \n");
                continue;
            }
            pid_t rec_pid = fork();
            if (rec_pid == 0){
                ejecutar_set(args);
            }
            favs_agregar(copybuffer);
            continue;
        }

        if (strcmp(args[0], "favs") == 0) {
            if (args[1] == NULL) {
                printf("Uso: favs [crear|mostrar|guardar|cargar]\n");
                continue;
            }
            if (strcmp(args[1], "crear") == 0) {
                if (args[2] == NULL) {
                    printf("Uso: favs crear ruta/ejemplo.txt\n");
                    continue;
                }
                favs_crear(args[2]);
            } else if (strcmp(args[1], "mostrar") == 0) {  // favs mostrar: despliega lista de favoritos.
                favs_mostrar();
            } else if (strcmp(args[1], "eliminar") == 0) { // favs eliminar num1,num2,num3,...: elimina los favoritos con los números ingresados.
                if (args[2] == NULL) {
                    printf("Uso: favs eliminar num1,num2,...\n");
                    continue;
                }
                favs_eliminar(args[2]);
            } else if (strcmp(args[1], "buscar") == 0) { // favs buscar comando: busca los favoritos que contengan el comando ingresado.
                if (args[2] == NULL) {
                    printf("Uso: favs buscar comando\n");
                    continue;
                }
                favs_buscar(args[2]);
            } else if (strcmp(args[1], "borrar") == 0) { // favs borrar: borra todos los favoritos.
                favs_borrar();
            } else if (args[2] != NULL && strcmp(args[2], "ejecutar") == 0) { // favs num ejecutar: ejecuta el favorito con el número ingresado.
                favs_ejecutar(atoi(args[1]));
            } else if (strcmp(args[1], "cargar") == 0) { // favs cargar: carga los favoritos guardados en el archivo.
                if (args[2] == NULL){
                    printf("Uso: favs cargar ruta/ejemplo.txt\n");
                    continue;
                }
                favs_cargar(args[2]);
            } else if (strcmp(args[1], "guardar") == 0) { // favs guardar: guarda los favoritos en el archivo.
                favs_guardar();
            } else if (strcmp(args[1], "ejecutar") == 0) {
                if (args[2] == NULL) {
                    printf("Uso: favs ejecutar num (o favs num ejecutar)\n");
                    continue;
                }
                favs_ejecutar(atoi(args[1]));
            } else {
                printf("Uso: favs [crear|mostrar|guardar|cargar]\n");
            }
        } else {
            ejecutar_comando(args);
            favs_agregar(copybuffer); // Agrega el comando a la lista de favoritos.
        }
    }

    return 0;
}
