#include <ep1.h>


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

// Função para o escalonador Shortest Job First
void shortest_job_first(Processo *processos, int num_processos) {
    // Implementação do escalonador Shortest Job First
}

// Função para o escalonador Round-Robin
void round_robin(Processo *processos, int num_processos) {
    // Implementação do escalonador Round-Robin
}

// Função para o escalonador com prioridade
void escalonamento_com_prioridade(Processo *processos, int num_processos) {
    // Implementação do escalonador com prioridade
}


void simular_processos(Processo *processos, int num_processos, int escalonador) {
    switch (escalonador) {
        case 1:
            escalonador_sjf(processos, num_processos);
            break;
        case 2:
            escalonador_rr(processos, num_processos);
            break;
        case 3:
            escalonador_prioridade(processos, num_processos);
            break;
        default:
            printf("Escalonador inválido.\n");
            exit(EXIT_FAILURE);
    }
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

    num_processos = (nome_arquivo_entrada, processos);
    simular_processos(processos, num_processos, escalonador);
    gerar_arquivo_saida(nome_arquivo_saida, processos, num_processos, 0);

    return EXIT_SUCCESS;
}