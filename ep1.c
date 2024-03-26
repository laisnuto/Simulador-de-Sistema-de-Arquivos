#include "ep1.h"

CircularQueue* fila_circular = NULL;
PriorityQueue* fila_prioridade = NULL;
pthread_mutex_t mtx;
int escalonador;
int num_processos;
int ultimo_processo_foi_executado = -1;



PriorityQueue* cria_pq(int capacidade) {
    PriorityQueue* pq = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    pq->processos = (Processo*) malloc(capacidade * sizeof(Processo));
    pq->tamanho = 0;
    pq->capacidade = capacidade;
    return pq;
}

void destroi_pq(PriorityQueue* pq) {
    free(pq->processos);
    free(pq);
}

void troca(Processo *a, Processo *b) {
    Processo temp = *a;
    *a = *b;
    *b = temp;
}


void corrige_subindo(PriorityQueue *pq, int i) {
    while (i > 0 && pq->processos[i].prioridade < pq->processos[(i - 1) / 2].prioridade) {
        troca(&pq->processos[i], &pq->processos[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void insere_pq(PriorityQueue* pq, Processo processo) {
    if (pq->tamanho == pq->capacidade) {
        printf("A fila tá cheia.\n");
        return;
    }
    int i = pq->tamanho;
    pq->tamanho++;
    pq->processos[i] = processo;
   corrige_subindo(pq, i); 
}

void corrige_descendo(PriorityQueue *pq, int i) {
    int tamanho = pq->tamanho;
    while (1) {
        int esq = 2 * i + 1; 
        int dir = 2 * i + 2; 
        int menor = i;

        if (esq < tamanho && pq->processos[esq].prioridade < pq->processos[menor].prioridade) {
            menor = esq;
        }
        if (dir < tamanho && pq->processos[dir].prioridade < pq->processos[menor].prioridade) {
            menor = dir;
        }
        
        if (menor != i) {
            troca(&pq->processos[i], &pq->processos[menor]);
            i = menor; 
        } else {
            
            break;
        }
    }
}

Processo remove_pq(PriorityQueue* pq) {
    if (pq->tamanho == 0) {
        printf("A fila tá vazia.\n");
        exit(EXIT_FAILURE);
    }
    Processo topo = pq->processos[0];
    pq->tamanho--;
    pq->processos[0] = pq->processos[pq->tamanho];
    
    corrige_descendo(pq, 0);
   
    return topo;
}


CircularQueue* cria_cq(int capacidade) {
    CircularQueue *queue = (CircularQueue *)malloc(sizeof(CircularQueue));
    if (!queue) return NULL;

    queue->capacidade = capacidade;
    queue->processos = (Processo *)malloc(sizeof(Processo) * capacidade);
    if (!queue->processos) {
        free(queue);
        return NULL;
    }
    queue->tamanho = 0;
    queue->frente = 0;
    queue->fim = capacidade - 1;

    return queue;
}

void destroi_cq (CircularQueue *queue) {
    if (queue) {
        free(queue->processos);
        free(queue);
    }
}

int insere_cq(CircularQueue *queue, Processo item) {
    if (queue->tamanho == queue->capacidade) return -1;
    queue->fim = (queue->fim + 1) % queue->capacidade;
    queue->processos[queue->fim] = item;
    queue->tamanho++;
    return 0;
}

Processo remove_cq(CircularQueue *queue) {
    Processo item; 
    item.deadline = -1; 
    item.t0 = -1;
    item.dt = -1;
    item.prioridade = -1; 
    if (queue->tamanho == 0) return item;

    item = queue->processos[queue->frente];
    queue->frente = (queue->frente + 1) % queue->capacidade;
    queue->tamanho--;
    return item;
}

Processo get_frente(CircularQueue *queue) {
    Processo item; 
    item.deadline = -1; 
    item.t0 = -1;
    item.dt = -1;
    item.prioridade = -1; 
    if (queue->tamanho == 0) return item;
    return queue->processos[queue->frente];
}

void rotaciona_cq(CircularQueue *queue) {
    if (queue->tamanho > 0) {
        queue->fim = (queue->fim + 1) % queue->capacidade;
        queue->processos[queue->fim] =  queue->processos[queue->frente];
        queue->frente = (queue->frente + 1) % queue->capacidade;
    }
}


int ler_arquivo(const char *nome_arquivo, Processo *processos) {

 

    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    int n = 0;
    while (fscanf(arquivo, "%16s %lf %lf %lf", processos[n].nome, &processos[n].deadline, &processos[n].t0, &processos[n].dt) == 4) {
        n++;
    }

    fclose(arquivo);

    return n;
}



// void atualiza_quantum(Processo *processo, int tempo_total_restante, int tempo_atual, int num_processos_ativos) {
//     if (num_processos_ativos <= 0) return;

    
//     int urgencia = processo->deadline - processo->tf; 
//     if (urgencia < 0) urgencia = 0; 

   
   
//     double proporcao_trabalho = (double)processo->dt / (double)tempo_total_restante;
 
//     int quantum_base = 5; // Quantum base, ajuste conforme necessário
//     double fator_urgencia = 1.0 + (double)urgencia / (double)processo->deadline; // Fator de urgência (>1 se urgente, ~1 se não)
//     double quantum_ajustado = quantum_base * fator_urgencia * proporcao_trabalho;
    
//     // Certifica-se de que o quantum ajustado é pelo menos 1 e não mais do que o dt restante do processo
//     if (quantum_ajustado < 1) quantum_ajustado = 1;
//     if (quantum_ajustado > processo->dt) quantum_ajustado = processo->dt;

//     processo->quantum = (int)round(quantum_ajustado); // Atualiza o quantum do processo
// }



void* execute_process(void* p) {
    Processo* processo = (Processo*) p;
    double tempo_inicio, tempo_fim, tempo_atual;
    tempo_inicio = time(NULL);

   
   
    tempo_atual = tempo_inicio;
    printf("Executando processo %s... \n", processo->nome);
    // atualiza_quantum(processo, tempo_total_restante, tempo_atual, num_processos_ativos);

    if (escalonador == 1) {
        while (tempo_atual - tempo_inicio < processo->dt) {
       
            int consumo_cpu = 0;
            for (int i = 0; i < 1000; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = time(NULL);
        }
    } 
    else if (escalonador == 2) {
       
        while (tempo_atual - tempo_inicio < processo->quantum && tempo_atual - tempo_inicio < processo->dt) {
            int consumo_cpu = 0;
            for (int i = 0; i < 100 && processo->dt > 0; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = time(NULL);

    
        }

        printf("Tempo atual: %f\n", tempo_atual);
        printf("tempo inicio: %f\n", tempo_inicio);


        processo->dt = processo->dt - processo->quantum;

        if (processo->dt > 0) {
            pthread_mutex_lock(&mtx);
            insere_cq(fila_circular, *processo);
            printf("Processo %s interrompido depois de ser executado por %.2f segundos e, então, reinserido na fila\n", processo->nome, tempo_atual - tempo_inicio);
            printf("Quantum: %f\n", processo->quantum);
            printf("Dt: %f\n", processo->dt);
            pthread_mutex_unlock(&mtx);
           
            
            pthread_exit(NULL);
        }
    }
    else if (escalonador == 3) {
         while (tempo_atual - tempo_inicio < processo->quantum || tempo_atual - tempo_inicio < processo->dt) {
            int consumo_cpu = 0;
            for (int i = 0; i < 100 && processo->dt > 0; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = time(NULL);
        }

        pthread_mutex_lock(&mtx);
        processo->dt = processo->dt - processo->quantum;
        pthread_mutex_unlock(&mtx);

        if (processo->dt > 0) {
            pthread_mutex_lock(&mtx);
            // atualiza_quantum();
            insere_cq(fila_circular, *processo);
            printf("Processo %s interrompido depois de ser executado por %.2f segundos e, então, reinserido na fila\n", processo->nome, tempo_atual - tempo_inicio);
            printf("Quantum: %f\n", processo->quantum);
            printf("Dt: %f\n", processo->dt);
            pthread_mutex_unlock(&mtx);
           
            printf("Processo %s interrompido e reinserido na fila\n", processo->nome);
            pthread_exit(NULL);
        }
    
    }
   


    ultimo_processo_foi_executado = 1;
    tempo_fim = (double)time(NULL); 
    processo->tf = tempo_fim; 
    processo->tr = processo->tf - processo->t0;
    unsigned long tempo_execucao = tempo_fim - tempo_inicio;
    
    printf("Tempo total de execução do processo %s: %lu segundos\n", processo->nome, tempo_execucao);

    return NULL;
}


void shortest_job_first() {
    pthread_t thread;
    int total_processos_executados = 0;
    

    while (1) {
        pthread_mutex_lock(&mtx);
        if (fila_prioridade->tamanho == 0) {
            pthread_mutex_unlock(&mtx);
            if (total_processos_executados == num_processos) {
                break; 
            }
            continue;
        }

        Processo processo_atual = remove_pq(fila_prioridade);
        pthread_mutex_unlock(&mtx);
        printf("Processo %s removido da fila de prioridade\n", processo_atual.nome);

        if (pthread_create(&thread, NULL, execute_process, &processo_atual)) {
            fprintf(stderr, "Erro ao criar thread\n");
            exit(EXIT_FAILURE);
        }

        pthread_join(thread, NULL); 
        total_processos_executados++;
    }

  
   
   
}


// Função para o escalonador Round-Robin
void round_robin() {
    pthread_t thread;
    int total_processos_executados = 0;

    while (1) {
        
        pthread_mutex_lock(&mtx);
        if (fila_circular->tamanho == 0) {
            pthread_mutex_unlock(&mtx);
            if (total_processos_executados == num_processos) {
                break; 
            }
            continue;
        }

        Processo processo_atual = remove_cq(fila_circular);
        pthread_mutex_unlock(&mtx);
        printf("Processo %s removido da fila \n", processo_atual.nome);
        ultimo_processo_foi_executado = 0;

        if (pthread_create(&thread, NULL, execute_process, &processo_atual)) {
            fprintf(stderr, "Erro ao criar thread\n");
            exit(EXIT_FAILURE);
        }

        pthread_join(thread, NULL); 

        if(ultimo_processo_foi_executado == 1) {
            total_processos_executados++;
        }
       
    }


}

// Função para o escalonador com prioridade
void escalonamento_com_prioridade() {
  
    pthread_t thread;
    int total_processos_executados = 0;

    while (1) {
        
        pthread_mutex_lock(&mtx);
        if (fila_circular->tamanho == 0) {
            pthread_mutex_unlock(&mtx);
            if (total_processos_executados == num_processos) {
                break; 
            }
            continue;
        }

        Processo processo_atual = remove_cq(fila_circular);
        pthread_mutex_unlock(&mtx);
        printf("Processo %s removido da fila \n", processo_atual.nome);
       
        ultimo_processo_foi_executado = 0;

        if (pthread_create(&thread, NULL, execute_process, &processo_atual)) {
            fprintf(stderr, "Erro ao criar thread\n");
            exit(EXIT_FAILURE);
        }

        pthread_join(thread, NULL); 

        if(ultimo_processo_foi_executado == 1) {
            total_processos_executados++;
        }
       
    }
}



int compara_t0(const void *a, const void *b) {
    const Processo *pa = (const Processo *)a;
    const Processo *pb = (const Processo *)b;
    return pa->t0 - pb->t0;
}


void arquivo_saida(const char *nome_arquivo_saida, Processo *processos,  int mudancas_contexto) {
    FILE *arquivo = fopen(nome_arquivo_saida, "w");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_processos; i++) {
        fprintf(arquivo, "%s %f %f\n", processos[i].nome, processos[i].tr, processos[i].tf);
    }

    fprintf(arquivo, "%d\n", mudancas_contexto);

    fclose(arquivo);
}



void imprime_cq(CircularQueue *queue) {
    if (queue == NULL || queue->tamanho == 0) {
        printf("A fila circular está vazia.\n");
        return;
    }

    printf("Fila Circular: \n");
    for (int i = 0, pos = queue->frente; i < queue->tamanho; i++, pos = (pos + 1) % queue->capacidade) {
        Processo p = queue->processos[pos];
        printf("Nome: %s, Deadline: %f, t0: %f, dt: %f\n", p.nome, p.deadline, p.t0, p.dt);
    }
    printf("\n");
}

void imprime_pq(PriorityQueue *pq) {
    if (pq == NULL || pq->tamanho == 0) {
        printf("A fila de prioridade está vazia.\n");
        return;
    }

    printf("Fila de Prioridade: \n");
    for (int i = 0; i < pq->tamanho; i++) {
        Processo p = pq->processos[i];
        printf("Nome: %s, Prioridade: %f, t0: %f, dt: %f\n", p.nome, p.prioridade, p.t0, p.dt);
    }
    printf("\n");
}

void insere_e_imprime_pq(PriorityQueue *pq, Processo *processos, int num_processos) {
    for (int i = 0; i < num_processos; i++) {
        processos[i].prioridade = processos[i].dt;
        insere_pq(pq, processos[i]);
        imprime_pq(pq);
    }

    printf("Removendo todos da PriorityQueue:\n");
    while (pq->tamanho > 0) {
        remove_pq(pq);
        imprime_pq(pq);
    }
}

void insere_imprime_e_rotaciona_cq(CircularQueue *cq, Processo *processos, int num_processos) {
    printf("Inserindo na CircularQueue:\n");
    for (int i = 0; i < num_processos; i++) {
        insere_cq(cq, processos[i]);
        imprime_cq(cq);
    }

    printf("Rotacionando a CircularQueue:\n");
    for (int i = 0; i < num_processos; i++) {
        rotaciona_cq(cq);
        imprime_cq(cq);
    }
}



void* adiciona_fila(void *arg) {
    Processo *processos = (Processo *) arg;


    double tempo_atual = 0;
    int i = 0;
    
    while(i < num_processos) {

       
     
        if (processos[i].t0 > tempo_atual) {
            sleep(processos[i].t0 - tempo_atual);
            tempo_atual = processos[i].t0;
        }

     

        pthread_mutex_lock(&mtx);
        while(i < num_processos && processos[i].t0 <= tempo_atual) {

            if (escalonador == 1) { 
                processos[i].prioridade = processos[i].dt;
               
                insere_pq(fila_prioridade, processos[i]);
               
               
            } else if (escalonador == 2) { 
                processos[i].quantum = 1;
                insere_cq(fila_circular, processos[i]); 
               
                
            }
            else if (escalonador == 3) { 
             
                insere_cq(fila_circular, processos[i]);
               
                
            }
            i++;
        }

        pthread_mutex_unlock(&mtx);

       
 
    }


    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <escalonador> <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    escalonador = atoi(argv[1]);
    const char *nome_arquivo_entrada = argv[2];
    const char *nome_arquivo_saida = argv[3];

    Processo processos[100]; 

    num_processos = ler_arquivo(nome_arquivo_entrada, processos);

  
    fila_circular = cria_cq(num_processos);
    fila_prioridade = cria_pq(num_processos);

    qsort(processos, num_processos, sizeof(Processo), compara_t0);

    pthread_t thread_adicionar_fila;
    pthread_mutex_init(&mtx, NULL);

   
    if(pthread_create(&thread_adicionar_fila, NULL, adiciona_fila, (void*)processos)) {
        fprintf(stderr, "Erro ao criar thread de monitoramento\n");
        return EXIT_FAILURE;
    }

    if (escalonador == 1) { 
        shortest_job_first();
    } else if (escalonador == 2) { 
        round_robin();
    } else if (escalonador == 3) { 
        escalonamento_com_prioridade();
    }

    pthread_join(thread_adicionar_fila, NULL);

    for (int i = 0; i < num_processos; i++) {
        printf("Nome: %s, Deadline: %f, t0: %f, dt: %f, tr: %f, tf: %f\n", processos[i].nome, processos[i].deadline, processos[i].t0, processos[i].dt, processos[i].tr, processos[i].tf);
    }

    arquivo_saida(nome_arquivo_saida, processos, 0);
    
    pthread_mutex_destroy(&mtx);
    destroi_cq(fila_circular);
    destroi_pq(fila_prioridade);



    return EXIT_SUCCESS;
}
   
