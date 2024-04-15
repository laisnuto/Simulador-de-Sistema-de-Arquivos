#include "ep2.h"


int d;
PosicaoPista* pista;
int d;
int k;
Ciclista* ciclistas;
pthread_mutex_t* pista_mutexes;


// Função para atualizar a velocidade do ciclista
void atualizar_velocidade(Ciclista *c) {
    if (c->velocidade == 30) {
       
       if (rand() % 100 < 70){
            c->velocidade = 60;
        } else {
            c->velocidade = 30;
        }

    } else if (c->velocidade == 60) {

        if (rand() % 100 < 50) {
            c->velocidade = 30;
        } else {
            c->velocidade = 60;
        }      
    }
}

// Função para verificar se é hora do ciclista quebrar
void quebra(Ciclista *c) {
    if (c->voltas % 6 == 0 && rand() % 100 < 15) {
        c->quebrado = 1; 
        printf("Ciclista %d quebrou na volta %d\n", c->id, c->voltas);
    }
}



// Função para criar a pista do tamanho certo
void cria_pista(int d) {
    pista = malloc(d * sizeof(PosicaoPista));
    pista_mutexes = malloc(d * sizeof(pthread_mutex_t));
    for (int i = 0; i < d; i++) {
        pista[i].ids = malloc(10 * sizeof(int));
        pista[i].quantidade_por_posição = 0;
        pthread_mutex_init(&pista_mutexes[i], NULL);
    }
}

// Função para adicionar um ciclista à posição da pista
void entrar_na_pista(int posicao, int ciclista_id) {
    int percorrido = 0;
    int achou_vaga = 0;

    while(percorrido < d && !achou_vaga){
        pthread_mutex_lock(&pista_mutexes[posicao]);
        if (pista[posicao].quantidade_por_posição < 5) {
            pista[posicao].ids[pista[posicao].quantidade_por_posição++] = ciclista_id;
            achou_vaga = 1;
        }
        pthread_mutex_unlock(&pista_mutexes[posicao]);
        posicao = (posicao + 1) % d;
        percorrido++;
    }
   
}

// Função para remover um ciclista da posição da pista
void sair_da_pista(int posicao, int ciclista_id) {
    pthread_mutex_lock(&pista_mutexes[posicao]);
    for (int i = 0; i < pista[posicao].quantidade_por_posição; i++) {
        if (pista[posicao].ids[i] == ciclista_id) {
            pista[posicao].ids[i] = pista[posicao].ids[--pista[posicao].quantidade_por_posição];
            break;
        }
    }
    pthread_mutex_unlock(&pista_mutexes[posicao]);
}

// Função que simula a corrida de cada ciclista
void *ciclista_func(void *arg) {
    Ciclista *c = (Ciclista *)arg;
    c->velocidade = 30;

    while (!c->quebrado && c->voltas < k) {
        usleep(c->velocidade == 30 ? 120000 : 60000);
        
        pthread_mutex_lock(&pista_mutexes[c->posicao]);
        sair_da_pista(c->posicao, c->id);
        
        int nova_posicao = (c->posicao + 1) % d;
        entrar_na_pista(nova_posicao, c->id);
        c->posicao = nova_posicao;
        
        if (nova_posicao == 0) {
            c->voltas++;
            checar_quebra(c);
            if (c->voltas > 1) atualizar_velocidade(c);
        }
        
        pthread_mutex_unlock(&pista_mutexes[c->posicao]);
    }
    pthread_exit(NULL);
}

void print_pista() {
    for (int i = 0; i < d; i++) {
        if (pista[i].quantidade_por_posição > 0) {
            printf(". ");
            for (int j = 0; j < pista[i].quantidade_por_posição; j++) {
                printf("%d ", pista[i].ids[j]);
            }
        } else {
            printf(". ");
        }
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    int debug = 0;


    if (argc != 3) {
        printf("Uso: %s <d> <k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    d = atoi(argv[1]); 
    k = atoi(argv[2]); 

    if (argc == 4 && strcmp(argv[3], "-debug") == 0) {
        debug = 1;
    }

    cria_pista(d);

    ciclistas = malloc(k * sizeof(Ciclista));

    srand(time(NULL));
    for (int i = 0; i < k; i++) {
        ciclistas[i].id = i;
        ciclistas[i].velocidade = 30;  
        ciclistas[i].posicao = rand() % d;
        ciclistas[i].voltas = 0;
        ciclistas[i].quebrado = 0;

        entrar_na_pista(ciclistas[i].posicao, i);

        if (pthread_create(&ciclistas[i].thread_id, NULL, ciclista_func, &ciclistas[i]) != 0) {
            perror("Falha ao criar thread do ciclista");
            free(ciclistas);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < k; i++) {
        pthread_join(ciclistas[i].thread_id, NULL);
    }

    free(ciclistas);
    return EXIT_SUCCESS;
}