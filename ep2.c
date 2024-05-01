#include "ep2.h"

pthread_barrier_t barrier;
PosicaoPista* pista;
int d;
int k;
Ciclista* ciclistas;
pthread_mutex_t pista_mutex; 
int  relogio_global = 0;
int debug = 0;
int proxima_volta_para_terminar = 2;
int colocacao = 1;
int voltas_globais = 0;


// Função para verificar e atualizar qual o número da volta do último ciclista (que é a volta global)
void verificar_atualizar_volta_global() {
    int min_voltas = 2*k + 2;
    pthread_mutex_lock(&pista_mutex);
    for (int i = 0; i < k; i++) {
        if (!ciclistas[i].quebrado && !ciclistas[i].terminou)
            min_voltas = fmin(min_voltas, ciclistas[i].voltas);
    }
    voltas_globais = min_voltas;
    pthread_mutex_unlock(&pista_mutex);
}

// Função para atualizar a velocidade do ciclista
void atualizar_velocidade(Ciclista *c) {

    if (c->velocidade == 30) {
        if (rand() % 100 < 70){
            c->velocidade = 60;
             c->rodada_de_movimento = 1;
        } else {
            c->velocidade = 30;
            c->rodada_de_movimento = 1;
        }

    } else if (c->velocidade == 60) {
        if (rand() % 100 < 50) {
            c->velocidade = 30;
             c->rodada_de_movimento = 1;
        } else {
            c->velocidade = 60;
             c->rodada_de_movimento = 1;
        }      
    }
    
}

// Função para atualizar posição de um ciclista 
void atualiza_posicao(Ciclista *c, int posicao, int linha) {
   c->posicao = posicao;
   c->linha = linha;
}


// Função para criar a pista do tamanho certo
void cria_pista(int d) {
    pista = malloc(d * sizeof(PosicaoPista));
   
    for (int i = 0; i < d; i++) {
        for (int j = 0; j < 10; j++) {
            pista[i].ids[j] = -1; 
        }
        pista[i].quantidade_por_posicao = 0;
      
    }

    pthread_mutex_init(&pista_mutex, NULL);
}

// Função para adicionar um ciclista à posição da pista
void entrar_na_pista(int posicao, int ciclista_id, int linha) {
    pista[posicao].ids[linha] = ciclista_id;
    pista[posicao].quantidade_por_posicao++;
}

// Função para remover um ciclista da posição da pista
void sair_da_pista(int posicao, int ciclista_id, int linha) {
    pista[posicao].ids[linha] = -1;
    pista[posicao].quantidade_por_posicao--;
}

// Função para atualizar a pista
void atualiza_pista(int id, int antiga_posicao, int nova_posicao, int antiga_linha, int nova_linha) {
  
    pista[antiga_posicao].ids[antiga_linha] = -1;
    pista[nova_posicao].ids[nova_linha] = id;

    printf("Ciclista %d  com id %d se moveu para a posição %d linha %d \n", id,  pista[nova_posicao].ids[nova_linha], nova_posicao, nova_linha);
  
}

// Função para verificar se o ciclista vai quebrar
void checa_quebra(Ciclista *c) {
    if (c->voltas > 1 && c->voltas % 6 == 0 && rand() % 100 < 15) {
        c->quebrado = 1; 
        c->terminou = 1;
        c->tempo_chegada = relogio_global * 60;
        printf("Ciclista %d quebrou na volta %d\n", c->id, c->voltas);
    }

    pthread_mutex_lock(&pista_mutex);
    sair_da_pista(c->posicao, c->id, c->linha);  
    pthread_mutex_unlock(&pista_mutex);
}



// Função pra mover um 
void mover_ciclista(int id) {
    Ciclista* c = &ciclistas[id-1];
    int nova_posicao = (c->posicao + 1) % d;
    int antiga_posicao = c->posicao;
    int pode_ultrapassar;
    int linha_livre = -1;
    int posicao_nova_livre = 0;


    if(pista[nova_posicao].ids[c->linha] == -1){
        posicao_nova_livre = 1;
    }

    if (c->velocidade == 30 && c->rodada_de_movimento == 0) {
        c->rodada_de_movimento = 1;
        
        return; 
    }

    if(posicao_nova_livre){
       
        if (c->velocidade == 30 ) {
            c->rodada_de_movimento = 0;
        }
        else{
            c->rodada_de_movimento = 1;
        }

        atualiza_pista(id, antiga_posicao, nova_posicao, c->linha, c->linha);
        atualiza_posicao(c, nova_posicao, c->linha);
    }
    else{

        pode_ultrapassar = 0;
        for (int i = 9; i >= 0; i--) {
            if (pista[nova_posicao].ids[i] == -1) {
                linha_livre = i;
                pode_ultrapassar = 1;
                break;
            }
        }

         if (pode_ultrapassar) {
            atualiza_pista(id, c->posicao, nova_posicao, c->linha, linha_livre);
            atualiza_posicao(c, nova_posicao, linha_livre);
        } else {
            c->velocidade = 30;
            c->rodada_de_movimento = 1;
        }

    }
   
    
     
}




// Função que simula a corrida de cada ciclista
void *ciclista_caminho(void *arg) {
    Ciclista *c = (Ciclista *)arg;
    c->velocidade = 30;

    
    
    pthread_barrier_wait(&barrier);

    pthread_barrier_wait(&barrier);


    while (!c->quebrado && !c->terminou && c->voltas < 2*k) {        
        
        pthread_mutex_lock(&pista_mutex);
        mover_ciclista(c->id);
        pthread_mutex_unlock(&pista_mutex);

        if (c->posicao == 0) {
            if (c->comecou == 0) {
                if(c->rodada_de_movimento == 1){
                    c->comecou = 1;
                }
                
            }
            else{

                if(c->rodada_de_movimento == 1){
                    c->voltas++;
                    verificar_atualizar_volta_global();
                }


                checa_quebra(c);
                if (c->voltas > 1){
                    atualizar_velocidade(c);
                } 
                 
            }

           
        }
        
        pthread_barrier_wait(&barrier);
        pthread_barrier_wait(&barrier);
    }

    while(!c->pode_sair){
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}

void print_pista() {
   
     pthread_mutex_lock(&pista_mutex);
    for (int linha = 0; linha < 10; linha++) {
        for (int posicao = 0; posicao < d; posicao++) {
            if (pista[posicao].ids[linha] != -1) {
                printf("%2d ", pista[posicao].ids[linha]);  
            } else {
                printf("%2s ", ".");  
            }
        }
        printf("\n");
    }

    printf("\n");
    pthread_mutex_unlock(&pista_mutex);
}


int compare_ciclistas_final(const void *a, const void *b) {
    const Ciclista *ciclistaA = (const Ciclista *)a;
    const Ciclista *ciclistaB = (const Ciclista *)b;

    if (ciclistaA->quebrado != ciclistaB->quebrado)
        return ciclistaA->quebrado - ciclistaB->quebrado;
    if (!ciclistaA->quebrado && !ciclistaB->quebrado) {
        return ciclistaA->colocacao - ciclistaB->colocacao; 
    }
    return ciclistaA->tempo_chegada - ciclistaB->tempo_chegada; // Ordena pelo tempo de chegada se ambos quebraram
}


void imprimir_relatorio_final() {
    printf("--------------------------------------------------------------------------------\n");
    printf("                                RELATÓRIO FINAL                                 \n");
    printf("--------------------------------------------------------------------------------\n");
    printf(" Colocação | ID | Voltas Completadas | Tempo de Chegada (ms) | Status            \n");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < k; i++) {
        if (ciclistas[i].quebrado) {
            printf("     --     | %2d |        %3d        |             Quebrou           | Quebrou na volta %d\n",
                   ciclistas[i].id, ciclistas[i].voltas, ciclistas[i].voltas);
        } else if (ciclistas[i].terminou) {
            printf("     %2d     | %2d |        %3d        |         %6d                 | Completo          \n",
                   ciclistas[i].colocacao, ciclistas[i].id, ciclistas[i].voltas, ciclistas[i].tempo_chegada);
        }
    }

    printf("\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("                               CICLISTAS QUEBRADOS                              \n");
    printf("--------------------------------------------------------------------------------\n");
    printf("    ID    | Voltas Completadas | Tempo de Quebra (ms)    | Detalhes            \n");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < k; i++) {
        if (ciclistas[i].quebrado) {
            printf("    %2d    |        %3d         |          %6d          | Quebrou na volta %d \n",
                   ciclistas[i].id, ciclistas[i].voltas, ciclistas[i].tempo_chegada, ciclistas[i].voltas);
        }
    }

    printf("--------------------------------------------------------------------------------\n \n");
}



void verifica_e_remove_ciclistas() {
    int ciclistas_na_frente[k];  
    int contagem = 0; 

    for (int i = 0; i < k; i++) {
        if (!ciclistas[i].quebrado && !ciclistas[i].terminou && ciclistas[i].voltas >= proxima_volta_para_terminar && ciclistas[i].posicao == 0) {
            ciclistas_na_frente[contagem++] = ciclistas[i].id;
        }
    }

    if (contagem > 0) {
        srand(time(NULL));  
        int escolhido_index = rand() % contagem; 
        int escolhido_id = ciclistas_na_frente[escolhido_index];

        ciclistas[escolhido_id -1].terminou = 1;
        ciclistas[escolhido_id-1].colocacao = colocacao;
        colocacao++;
        ciclistas[escolhido_id-1].tempo_chegada = relogio_global * 60;

        pthread_mutex_lock(&pista_mutex);
        sair_da_pista(ciclistas[escolhido_id-1].posicao, ciclistas[escolhido_id-1].id, ciclistas[escolhido_id-1].linha);  
        pthread_mutex_unlock(&pista_mutex);

        proxima_volta_para_terminar += 2;  
    }
}




void print_ciclistas() {
    printf("Dados dos Ciclistas:\n");

    for (int i = 0; i < k; i++) {
        Ciclista c = ciclistas[i];
        printf("Ciclista %2d:    com indice i = %d \n", c.id, i+1);
        printf("Posição: %2d   ", c.posicao);
        printf("Velocidade: %2d km/h   ", c.velocidade);
        printf("Move: %2d  ", c.rodada_de_movimento);
        printf("Linha: %2d  ", c.linha);
        printf("Voltas: %2d   " , c.voltas);
        printf("Status: %2s   ", c.quebrado ? "Quebrado" : "Ativo");
        printf("Terminou: %2d   ", c.terminou);
        printf("Começou: %2d   ", c.comecou);
        printf("\n \n");
    }
}


int quantidade_de_ciclistas_na_corrida() {
    int ativos = 0;
    for (int i = 0; i < k; i++) {
        if (!ciclistas[i].terminou){
            ativos++;
        } 
    }
    return ativos;
}

int compare_ciclistas(const void *a, const void *b) {
    Ciclista *ciclistaA = (Ciclista *)a;
    Ciclista *ciclistaB = (Ciclista *)b;
    return ciclistaA->id - ciclistaB->id;
}


void acaba_corrida() {

    for (int i = 0; i < k; i++) {
        ciclistas[i].pode_sair = 1;      
    }

    imprimir_relatorio_final();
     
}

void corrida() {

      
    pthread_barrier_wait(&barrier);
    printf("Iniciando corrida\n");
    print_ciclistas();
    print_pista(); 
    qsort(ciclistas, k, sizeof(Ciclista), compare_ciclistas);
    pthread_barrier_wait(&barrier);
    
    while (quantidade_de_ciclistas_na_corrida() > 1){
       
        pthread_barrier_wait(&barrier);
        verifica_e_remove_ciclistas();
        if (debug) {
           
            printf("Relógio global: %dms\n", relogio_global * 60);
            print_pista();
        }
        relogio_global = relogio_global + 1;
        pthread_barrier_wait(&barrier);
    }
    
    acaba_corrida();
}




int main(int argc, char *argv[]) {


    if (argc != 3 && argc != 4) {
        printf("Uso: %s <d> <k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc != 3 && argc != 4) {
        printf("Uso: %s <d> <k> [opcao]\n", argv[0]);
        return EXIT_FAILURE;
    }

    d = atoi(argv[1]); 
    k = atoi(argv[2]); 

    if (argc == 4 && strcmp(argv[3], "-debug") == 0) {
        debug = 1;
    }

    printf("d = %d, k = %d, debug = %d\n", d, k, debug);


    cria_pista(d);

   
    pthread_barrier_init(&barrier, NULL, k + 1);


    ciclistas = malloc(k * sizeof(Ciclista));

    for (int i = 0; i < k; i++) {
        ciclistas[i].id = i+1;     
    }

    for (int i = 0; i < k; i++) {
        int j = rand() % (i + 1);
        int temp = ciclistas[i].id;
        ciclistas[i].id = ciclistas[j].id;
        ciclistas[j].id = temp;
    }

    int posicao_inicial = d - 1;
    int linha_atual = 9;

    for (int i = 0; i < k; i++) {
        ciclistas[i].velocidade = 30;  
        ciclistas[i].posicao = posicao_inicial;
        ciclistas[i].voltas = 0;
        ciclistas[i].quebrado = 0;
        ciclistas[i].linha = linha_atual;
        ciclistas[i].rodada_de_movimento = 0;
        ciclistas[i].tempo_chegada = -1;
        ciclistas[i].terminou = 0;
        ciclistas[i].comecou = 0;
        ciclistas[i].colocacao = -1;
        ciclistas[i].pode_sair = 0;

        pthread_mutex_lock(&pista_mutex);
        entrar_na_pista(posicao_inicial, ciclistas[i].id, linha_atual);
        pthread_mutex_unlock(&pista_mutex);

        linha_atual--;
        if (linha_atual == 4) { 
            linha_atual = 9;
            posicao_inicial--;
        }

           
        if (pthread_create(&ciclistas[i].thread_id, NULL, ciclista_caminho, &ciclistas[i]) != 0) {
             perror("Falha ao criar thread do ciclista");
             free(ciclistas);
             return EXIT_FAILURE;
        }

         
    }

    corrida();

    for (int i = 0; i < k; i++) {
        pthread_join(ciclistas[i].thread_id, NULL);
    }

    free(ciclistas);
    return EXIT_SUCCESS;
}