#include <stdlib.h>
#include <stdio.h>
#include "ep1.h" 


typedef struct PriorityQueue {
    Processo *fila;
    int tamanho;
    int capacidade;
} PriorityQueue;


PriorityQueue* cria(int capacidade);
void destroi(PriorityQueue* pq);
void troca(Processo *a, Processo *b);
void corrige_subindo(PriorityQueue *pq, int i);
void insere_elemento(PriorityQueue* pq, Processo processo);
void corrige_descendo(PriorityQueue *pq, int i);
Processo deleta_elemento(PriorityQueue* pq);

