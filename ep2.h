#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



typedef struct {
    int id;
    int velocidade;
    int posicao;
    int voltas;
    int quebrado;
    pthread_t thread_id;
} Ciclista;

typedef struct {
    int* ids;
    int quantidade_por_posição;
} PosicaoPista;