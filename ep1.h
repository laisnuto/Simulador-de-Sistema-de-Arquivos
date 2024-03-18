#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char nome[17];
    int deadline;
    int t0;
    int dt;
    int prioridade;
} Processo;


int ler_arquivo(char *nome_arquivo, Processo *processos);
void shortest_job_first(Processo *processos, int num_processos);
void round_robin(Processo *processos, int num_processos);
void escalonamento_com_prioridade(Processo *processos, int num_processos);
void gerar_arquivo_saida(const char *nome_arquivo_saida, Processo *processos, int num_processos, int mudancas_contexto);
void simular_processos(Processo *processos, int num_processos, int escalonador);