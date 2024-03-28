#include "ep1.h"

CircularQueue* fila_circular = NULL;
PriorityQueue* fila_prioridade = NULL;
Processo processos[100]; 
pthread_mutex_t mtx;
int escalonador;
int num_processos;
int mudancas_contexto = 0;
int ultimo_processo_foi_executado = -1;
time_t tempo_inicio_programa;


double calcula_tempo_atual() {   
    struct timeval agora;
    gettimeofday(&agora, NULL);
    double tempo_segundos = agora.tv_sec + agora.tv_usec / 1000000.0;
    return tempo_segundos - tempo_inicio_programa;

}

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


int ler_arquivo(const char *nome_arquivo) {

 

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

void atualiza_processos(Processo* processo) {
    for (int i = 0; i < num_processos; i++) {
        if (strcmp(processos[i].nome, processo->nome) == 0) {
            processos[i].tf = processo->tf;
            processos[i].tr = processo->tr;
            break;
        }
    }
}




 void atualiza_quantum(Processo *processo) {
  
    double tempo_atual = calcula_tempo_atual();
    double quantum_min = 1;
    double quantum_max = 10; 

    double urgencia = processo->deadline - tempo_atual;
    if (urgencia <= 0 ){
        urgencia = 0.1;
    }

    
    processo->quantum = quantum_min + 1.0/urgencia;


   if (processo->quantum > quantum_max) {
        processo->quantum = quantum_max;
    } else if (processo->quantum < quantum_min) {
        processo->quantum = quantum_min;
    }

}




void* execute_process(void* p) {
    Processo* processo = (Processo*) p;
    double tempo_inicio, tempo_atual;
    tempo_inicio = calcula_tempo_atual();

    cpu_set_t cpuset;
    pthread_t thread;
    thread = pthread_self(); 

    CPU_ZERO(&cpuset); 

    CPU_SET(0, &cpuset); 

    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
      
        fprintf(stderr, "Erro na afinidade da CPU para a thread");
        exit(EXIT_FAILURE);
    }

   
   
    tempo_atual = tempo_inicio;
    printf("\n");
    printf("-----------------------------------------------------------------------------------------------------\n \n");
    printf("Executando processo %s... \n", processo->nome);
   

    if (escalonador == 1) {
        while (tempo_atual - tempo_inicio < processo->dt) {
       
            int consumo_cpu = 0;
            for (int i = 0; i < 1000; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = calcula_tempo_atual();
        }
    } 
    else if (escalonador == 2) {
       
        while (tempo_atual - tempo_inicio < processo->quantum && tempo_atual - tempo_inicio < processo->tempo_restante) {
            int consumo_cpu = 0;
            for (int i = 0; i < 100 && processo->tempo_restante> 0; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = calcula_tempo_atual();

    
        }


        pthread_mutex_lock(&mtx);
        processo->tempo_restante = processo->tempo_restante - processo->quantum;
        pthread_mutex_unlock(&mtx);


        if (processo->tempo_restante > 0) {
            pthread_mutex_lock(&mtx);
            insere_cq(fila_circular, *processo);
            printf("Processo %s interrompido depois de ser executado por %.2f segundos e, então, reinserido na fila\n", processo->nome, tempo_atual - tempo_inicio);
            printf("Quantum: %f\n", processo->quantum);
            printf("resta: %f\n", processo->tempo_restante);
            pthread_mutex_unlock(&mtx);
            pthread_exit(NULL);
        }
    }
    else if (escalonador == 3) {
         atualiza_quantum(processo);
         while (tempo_atual - tempo_inicio < processo->quantum && tempo_atual - tempo_inicio < processo->tempo_restante) {
            int consumo_cpu = 0;
            for (int i = 0; i < 100 && processo->dt > 0; i++) {
                consumo_cpu += i * i;
            }
            tempo_atual = calcula_tempo_atual();
           
        }

       


        pthread_mutex_lock(&mtx);
        processo->tempo_restante = processo->tempo_restante - processo->quantum;
        pthread_mutex_unlock(&mtx);

        if (processo->tempo_restante > 0) {
            pthread_mutex_lock(&mtx);
            insere_cq(fila_circular, *processo);
            printf("Processo %s interrompido depois de ser executado por %.2f segundos e, então, reinserido na fila\n", processo->nome, (double)(tempo_atual - tempo_inicio));
            printf("Quantum: %f\n", processo->quantum);
            printf("resta: %f\n", processo->tempo_restante);
            pthread_mutex_unlock(&mtx);
            pthread_exit(NULL);
        }
    
    }
   


    ultimo_processo_foi_executado = 1;
    processo->tf = calcula_tempo_atual();
    processo->tr = processo->tf - processo->t0;
    atualiza_processos(processo);
    printf("Tempo total de execução do processo %s: %.2f segundos\n", processo->nome, processo->dt);
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
        mudancas_contexto++;
        pthread_mutex_unlock(&mtx);

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
        mudancas_contexto++;
        pthread_mutex_unlock(&mtx);
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
        mudancas_contexto++;
        pthread_mutex_unlock(&mtx);
       
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


void arquivo_saida(const char *nome_arquivo_saida, Processo *processos) {
    FILE *arquivo = fopen(nome_arquivo_saida, "w");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_processos; i++) {
        fprintf(arquivo, "%s %.2f %.2f\n", processos[i].nome, processos[i].tr, processos[i].tf);
    }

    fprintf(arquivo, "%d\n", mudancas_contexto);

    fclose(arquivo);
}



void* adiciona_fila(void *arg) {

    Processo *processos = (Processo *) arg;


    double tempo_atual = 0;

    cpu_set_t cpuset;
    pthread_t thread;
    thread = pthread_self(); 

    CPU_ZERO(&cpuset); 

    CPU_SET(0, &cpuset); 

    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
      
        fprintf(stderr, "Erro na afinidade da CPU para a thread");
        exit(EXIT_FAILURE);
    }

  
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
                processos[i].tempo_restante = processos[i].dt;
                insere_cq(fila_circular, processos[i]); 
               
                
            }
            else if (escalonador == 3) { 
                 processos[i].tempo_restante = processos[i].dt;
                insere_cq(fila_circular, processos[i]);
               
                
            }
            i++;
        }

        pthread_mutex_unlock(&mtx);

       
 
    }


    return NULL;
}

void imprime( const char* nome_arquivo_dados, Processo *processos) {
    FILE *arquivo = fopen(nome_arquivo_dados, "w");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo de dados para análise");
        exit(EXIT_FAILURE);
    }

    // Cabeçalho para facilitar a importação e análise dos dados
    fprintf(arquivo, "NomeProcesso,TempoResposta,TempoFinalizacao,MudancasContexto\n");

    for (int i = 0; i < num_processos; i++) {
        fprintf(arquivo, "%s,%.2f,%.2f,%d\n",
                processos[i].nome,
                processos[i].tr, // Tempo de resposta
                processos[i].tf, // Tempo de finalização
                mudancas_contexto); // Supondo que `mudancas_contexto` é uma variável global ou passada de outra forma
    }

    fclose(arquivo);
}

double inicializa_tempo() {
    struct timeval agora;
    gettimeofday(&agora, NULL);
    return agora.tv_sec + agora.tv_usec / 1000000.0;
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <escalonador> <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    escalonador = atoi(argv[1]);
    const char *nome_arquivo_entrada = argv[2];
    const char *nome_arquivo_saida = argv[3];

    num_processos = ler_arquivo(nome_arquivo_entrada);

  
    fila_circular = cria_cq(num_processos);
    fila_prioridade = cria_pq(num_processos);

    qsort(processos, num_processos, sizeof(Processo), compara_t0);

    tempo_inicio_programa = inicializa_tempo();
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

      mudancas_contexto--;
    arquivo_saida(nome_arquivo_saida, processos);
    imprime("dados", processos);
    
    pthread_mutex_destroy(&mtx);
    destroi_cq(fila_circular);
    destroi_pq(fila_prioridade);



    return EXIT_SUCCESS;
}
   
