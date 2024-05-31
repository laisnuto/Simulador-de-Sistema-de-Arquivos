#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <bits/pthreadtypes.h>
#include <time.h>

#define BLOCK_SIZE 4096 
#define MAX_BLOCKS 25581
#define MAX_FILES 25600
#define MAX_FILENAME

typedef struct {
    char nome[255];      
    uint32_t tamanho; 
    time_t criado;       
    time_t modificado;   
    time_t acessado;      
    uint32_t primeiro_bloco;
    uint8_t eh_diretorio;     
} Metadados;


typedef struct {
    uint16_t prox_bloco[MAX_BLOCKS]; 
} FAT;


typedef struct {
    uint8_t blocos_livres[MAX_BLOCKS]; 
} Bitmap;


// 104857600 = 1 * qtd_blocos + 2 * qtd_blocos + 4096 * qtd_blocos


void monta(char *arquivo);
void desmonta();
void copia(char *origem, char *destino);
void criadir(char *diretorio);
void apagadir(char *diretorio);
void mostra(char *arquivo);
void toca(char *arquivo);
void apaga(char *arquivo);
void lista(char *diretorio);
void atualizadb();
void busca(char *string);
void status();

