#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

typedef struct {
    char nome[17];
    int deadline;
    int t0;
    int dt;
    int prioridade;
    int quantum;
    int tr;
    int tf;
} Processo;


typedef struct PriorityQueue {
    Processo *processos;
    int tamanho;
    int capacidade;
} PriorityQueue;

typedef struct {
    Processo *processos;  
    int capacidade;     
    int tamanho;        
    int frente;        
    int fim;         
} CircularQueue;


PriorityQueue* cria_pq(int capacidade);
void destroi_pq(PriorityQueue* pq);
void troca(Processo *a, Processo *b);
void corrige_subindo(PriorityQueue *pq, int i);
void insere_pq(PriorityQueue* pq, Processo processo);
void corrige_descendo(PriorityQueue *pq, int i);
Processo remove_pq(PriorityQueue* pq);


CircularQueue* cria_cq(int capacidade);
void destroi_cq(CircularQueue *queue);
int insere_cq(CircularQueue *queue, Processo item);
Processo remove_cq(CircularQueue *queue);
Processo get_frente(CircularQueue *queue);
void rotaciona_cq(CircularQueue *queue);



int ler_arquivo(const char *nome_arquivo, Processo *processos);
void* execute_process(void *p); 
void shortest_job_first();
void round_robin() ;
void escalonamento_com_prioridade();
void arquivo_saida(const char *nome_arquivo_saida, Processo *processos, int mudancas_contexto);
