#include "ep1.h"

CircularQueue* fila_circular = NULL;
PriorityQueue* fila_prioridade = NULL;
pthread_mutex_t lock;


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
    while (fscanf(arquivo, "%16s %d %d %d", processos[n].nome, &processos[n].deadline, &processos[n].t0, &processos[n].dt) == 4) {
        n++;
    }

    fclose(arquivo);

    return n;
}

void* execute_process(void* p) {
    Processo* processo = (Processo*) p; 
    unsigned long tempo_inicio, tempo_atual;
    tempo_inicio = time(NULL); 

   
   while(tempo_atual - tempo_inicio < processo->dt){
        tempo_atual = time(NULL);
        tempo_atual = tempo_atual*1;
   }

    printf("Tempo total de execução do processo %s: %d segundos\n", processo->nome, processo->dt);


    return NULL;
}


void shortest_job_first(PriorityQueue* fila) {
    pthread_t threads[fila->tamanho + 1]; 
    Processo processoAtual;

    int i = 0;

    while (fila->tamanho > 0 ) {
        
        remove_pq(fila);
        
        if (pthread_create(&threads[i], NULL, execute_process, &processoAtual)) {
            fprintf(stderr, "Erro ao cria_pqr thread\n");
            exit(EXIT_FAILURE);
        }

        i++;

       
        pthread_join(threads[i], NULL);
        
    }

   
   
}


// Função para o escalonador Round-Robin
void round_robin(CircularQueue* fila) {
    pthread_t threads[fila->capacidade];

    int count = 0;
    int QUANTUM = 1;
    while (fila->tamanho > 0) {
        Processo processo = remove_cq(fila);
        if (processo.dt > QUANTUM) {
            processo.dt -= QUANTUM;
            insere_cq(fila, processo);
        } else {
            processo.dt = QUANTUM; 
        }

        if (pthread_create(&threads[count++], NULL, execute_process, (void*)&processo)) {
            fprintf(stderr, "Erro ao criar thread\n");
            exit(EXIT_FAILURE);
        }

        pthread_join(threads[count-1], NULL);
    }
}

// Função para o escalonador com prioridade
void escalonamento_com_prioridade(CircularQueue* fila) {
    pthread_t threads[fila->capacidade];

    int count = 0;
    int QUANTUM = 1;
    while (fila->tamanho > 0) {
        Processo processo = remove_cq(fila);
        processo.dt = QUANTUM; 

        if (pthread_create(&threads[count++], NULL, execute_process, (void*)&processo)) {
            fprintf(stderr, "Erro ao criar thread\n");
            exit(EXIT_FAILURE);
        }

      
        pthread_join(threads[count-1], NULL);
    }
}



int compara_t0(const void *a, const void *b) {
    const Processo *pa = (const Processo *)a;
    const Processo *pb = (const Processo *)b;
    return pa->t0 - pb->t0;
}


void gerar_arquivo_saida(const char *nome_arquivo_saida, Processo *processos, int num_processos, int mudancas_contexto) {
    FILE *arquivo = fopen(nome_arquivo_saida, "w");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_processos; i++) {
        fprintf(arquivo, "%s %d %d\n", processos[i].nome, processos[i].t0, processos[i].dt);
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
        printf("Nome: %s, Deadline: %d, t0: %d, dt: %d\n", p.nome, p.deadline, p.t0, p.dt);
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
        printf("Nome: %s, Prioridade: %d, t0: %d, dt: %d\n", p.nome, p.prioridade, p.t0, p.dt);
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




int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <escalonador> <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int escalonador = atoi(argv[1]);
    const char *nome_arquivo_entrada = argv[2];
    const char *nome_arquivo_saida = argv[3];

    Processo processos[100]; 
    int num_processos;

    num_processos = ler_arquivo(nome_arquivo_entrada, processos);

  
    fila_circular = cria_cq(num_processos);
    fila_prioridade = cria_pq(num_processos);
    

    insere_e_imprime_pq(fila_prioridade, processos, num_processos);


    insere_imprime_e_rotaciona_cq(fila_circular, processos, num_processos);

    qsort(processos, num_processos, sizeof(Processo), compara_t0);

    unsigned long tempo_atual = 0;
    int i = 0;
    while(i < num_processos) {
     
        if (processos[i].t0 > tempo_atual) {
            sleep(processos[i].t0 - tempo_atual);
            tempo_atual = processos[i].t0;
        }

        while(i < num_processos && processos[i].t0 <= tempo_atual) {
            if (escalonador == 1) { 
                processos[i].prioridade = processos[i].dt;
                insere_pq(fila_prioridade, processos[i]);
            } else if (escalonador == 2) { 
                insere_cq(fila_circular, processos[i]); 
            }
            else if (escalonador == 3) { 
                processos[i].prioridade = processos[i].deadline;
                insere_cq(fila_circular, processos[i]);
            }
            i++;
        }

        if (escalonador == 1) { 
            shortest_job_first(fila_prioridade);
        } else if (escalonador == 2) { 
            round_robin(fila_circular);
        } else if (escalonador == 3) { 
            escalonamento_com_prioridade(fila_circular);
        }
       
        gerar_arquivo_saida(nome_arquivo_saida, processos, num_processos, 0);
    }



    return EXIT_SUCCESS;
}
   
