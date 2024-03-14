#include "newsh.h"

#define MAX_COMMAND_LENGTH 1024
#define MAX_PARAMETERS 10

void print_parameters(char *parametros[]) {
    printf("Parâmetros:\n");
    for (int i = 0; parametros[i] != NULL; i++) {
        printf("parametro[%d]: '%s'\n", i, parametros[i]);
    }
}


void change_directory(char *path) {
    char absolute_path[1000];
    if (realpath(path, absolute_path) == NULL) {
        perror("Erro resolvendo o caminho");
        return;
    }

    if (chdir(absolute_path) != 0) {
        perror("Erro mudando de diretório");
    }
}


void remove_file(char *filename){
       if (unlink(filename) != 0) {
        perror("Erro removendo arquivo");
    } 
}

void uname_a() {
    struct utsname info;

    uname(&info);

    printf("%s %s %s %s %s\n", info.sysname, info.nodename, info.release, info.version, info.machine);
}


// Função que exibe o prompt
void exibe_prompt() {
    struct passwd *pw;
    uid_t uid;
    time_t t = time(NULL);
    struct tm tempo = *localtime(&t);

    uid = getuid();
    pw = getpwuid(uid);

    if (pw) {
        printf("%s [%02d:%02d:%02d]: ", pw->pw_name, tempo.tm_hour, tempo.tm_min, tempo.tm_sec);
    } else {
        printf("unknown-user [%02d:%02d:%02d]: ", tempo.tm_hour, tempo.tm_min, tempo.tm_sec);
    }
    fflush(stdout);
}


// Função que lê o comando e seus parâmetros
void le_comando(char *comando, char *parametros[]) {
    char *linha;
    char *token;
    int i = 0;

    linha = readline("");
    if (linha && *linha) {
        add_history(linha);
    }

    // Ignora espaços em branco no início da linha
    while (*linha == ' ') {
        linha++;
    }

    token = strtok(linha, " ");
    if (token) {
        strncpy(comando, token, MAX_COMMAND_LENGTH - 1); // Deixa espaço para o caractere nulo
        comando[MAX_COMMAND_LENGTH - 1] = '\0'; // Garante que a string esteja terminada com um caractere nulo
    }

    while ((token = strtok(NULL, " ")) != NULL && i < MAX_PARAMETERS) {
        parametros[i] = token;
        i++;
    }
    parametros[i] = NULL;
}


int main() {
    char comando[1000];
    char *parametros[10];
    char *variaveis_ambiente[] = { NULL };
     int status;

    while (1) {
        exibe_prompt();
        le_comando(comando, parametros);

        printf("Comando: '%s'\n", comando);
        print_parameters(parametros);
    
        if (strcmp(comando, "cd") == 0) {
            change_directory(parametros[0]);
        }
        
        else if (strcmp(comando, "rm") == 0) {
            remove_file(parametros[0]);
        }
        
        else if (strcmp(comando, "uname") == 0 && strcmp(parametros[0], "-a") == 0) {
            uname_a();
        } 
        
        else { 
            if (fork() != 0) {
               
               waitpid(-1, &status, 0);

            } else {
                
                execve(comando, parametros, variaveis_ambiente);
                perror("execve() error");
                exit(EXIT_FAILURE);
               
            }
        }
    }

    return 0;
}
