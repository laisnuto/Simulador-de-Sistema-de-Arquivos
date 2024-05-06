#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
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


int compara_ciclistas_volta(const void *a, const void *b);
void imprimir_relatorio_por_volta(int volta);
void verificar_atualizar_volta_global();
void atualizar_velocidade(Ciclista *c);
void atualiza_posicao(Ciclista *c, int posicao, int linha);
void cria_pista(int d);
void entrar_na_pista(int posicao, int ciclista_id, int linha);
void sair_da_pista(int posicao, int ciclista_id, int linha);
void atualiza_pista(int id, int antiga_posicao, int nova_posicao, int antiga_linha, int nova_linha);
void checa_quebra(Ciclista *c);
void mover_ciclista(int id);
void *ciclista_caminho(void *arg);
void print_pista();
void imprimir_relatorio_final();
void verifica_e_remove_ciclistas();
int quantidade_de_ciclistas_na_corrida();
int compara_ciclistas_por_id(const void *a, const void *b); 
void acaba_corrida();