#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <bits/pthreadtypes.h>

typedef struct {
    int id;
    int velocidade;
    int posicao;
    int linha;
    int voltas;
    int quebrado;
    int rodada_de_movimento;
    int tempo_chegada;
    int pode_sair;
    int colocacao;
    int terminou;
    int comecou;
    pthread_t thread_id;
} Ciclista;

typedef struct {
    int ids[10];  
    int quantidade_por_posicao;
} PosicaoPista;