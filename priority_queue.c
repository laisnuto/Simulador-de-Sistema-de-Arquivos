#include <ep1.h>

typedef struct PriorityQueue {
    Processo *fila;
    int tamanho;
    int capacidade;
} PriorityQueue;

PriorityQueue* cria(int capacidade) {
    PriorityQueue* pq = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    pq->fila = (Processo*) malloc(capacidade * sizeof(Processo));
    pq->tamanho = 0;
    pq->capacidade = capacidade;
    return pq;
}

void destroi(PriorityQueue* pq) {
    free(pq->fila);
    free(pq);
}

void troca(Processo *a, Processo *b) {
    Processo temp = *a;
    *a = *b;
    *b = temp;
}

void corrige_subindo(PriorityQueue *pq, int i) {
    while (i > 0 && pq->fila[i].prioridade < pq->fila[(i - 1) / 2].prioridade) {
        troca(&pq->fila[i], &pq->fila[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void insere_elemento(PriorityQueue* pq, Processo processo) {
    if (pq->tamanho == pq->capacidade) {
        printf("A fila tá cheia.\n");
        return;
    }
    int i = pq->tamanho;
    pq->tamanho++;
    pq->fila[i] = processo;
   corrige_subindo(pq, i); 
}

void corrige_descendo(PriorityQueue *pq, int i) {
    int tamanho = pq->tamanho;
    while (1) {
        int esq = 2 * i + 1; 
        int dir = 2 * i + 2; 
        int menor = i;

        if (esq < tamanho && pq->fila[esq].prioridade < pq->fila[menor].prioridade) {
            menor = esq;
        }
        if (dir < tamanho && pq->fila[dir].prioridade < pq->fila[menor].prioridade) {
            menor = dir;
        }
        
        if (menor != i) {
            troca(&pq->fila[i], &pq->fila[menor]);
            i = menor; 
        } else {
            
            break;
        }
    }
}


Processo deleta_elemento(PriorityQueue* pq) {
    if (pq->tamanho == 0) {
        printf("A fila tá vazia.\n");
        exit(EXIT_FAILURE);
    }
    Processo topo = pq->fila[0];
    pq->tamanho--;
    pq->fila[0] = pq->fila[pq->tamanho];
    
    corrige_descendo(pq, 0);
   
    return topo;
}

