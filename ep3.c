#include "ep3.h"


FAT fat;
Bitmap bitmap;
FILE *sistema_arquivos;
int primeiro_bloco_raiz = 0;

void escreve_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, num_bloco * BLOCK_SIZE, SEEK_SET);
    fwrite(dados, BLOCK_SIZE, 1, sistema_arquivos);
}

void le_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, num_bloco * BLOCK_SIZE, SEEK_SET);
    fread(dados, BLOCK_SIZE, 1, sistema_arquivos);
}

void imprime_arvore(uint16_t bloco_atual, int nivel) {
    if (bloco_atual == primeiro_bloco_raiz && nivel == 0) {
        printf("/\n");  
        bloco_atual = fat.prox_bloco[bloco_atual]; 
    }
    
    while (bloco_atual != 0xFFFF) {
        Metadados buffer;
        le_bloco(bloco_atual, &buffer);

        for (int i = 0; i < nivel; i++) {
            printf("  ");
        }
        printf("%s\n", buffer.nome);

        if (buffer.eh_diretorio) {
            imprime_arvore(buffer.primeiro_bloco, nivel + 1);
        }

        bloco_atual = fat.prox_bloco[bloco_atual];
    }
}


int localiza_arquivo(char *nome_arquivo, uint32_t diretório_raiz) {
    Metadados buffer;
    int bloco_atual = diretório_raiz;
    while (bloco_atual != 0xFFFF) { 
        le_bloco(bloco_atual, &buffer);
        if (strcmp(buffer.nome, nome_arquivo) == 0) {
            return bloco_atual;  
        }
        bloco_atual = fat.prox_bloco[bloco_atual]; 
    }
    return -1;  
}


void monta(char *path) {
  

    printf("Primeiro bloco raiz: %d\n", primeiro_bloco_raiz);
    sistema_arquivos = fopen(path, "r+b"); 

     // Arquivo não existe, cria um novo
    if (sistema_arquivos == NULL) {
       
        sistema_arquivos = fopen(path, "w+b");
        if (sistema_arquivos == NULL) {
            fprintf(stderr, "Erro ao criar o arquivo do sistema de arquivos.\n");
            exit(1);
        }

        // Inicializa bitmap e FAT
        memset(&bitmap, 0, sizeof(Bitmap));
        memset(&fat, 0xFF, sizeof(FAT));       

    } else {
        
        imprime_arvore(primeiro_bloco_raiz, 0);
    }
}




void copia(char *origem, char *destino) {
    FILE *origem_fp = fopen(origem, "rb");
    if (origem_fp) {
        fseek(origem_fp, 0, SEEK_END);
        long tamanho = ftell(origem_fp);
        rewind(origem_fp);

        int primeiro_bloco = -1, ultimo_bloco = -1;
        while (tamanho > 0) {
            int bloco_livre = -1;
            for (int i = 0; i < MAX_BLOCKS; i++) {
                if (bitmap.blocos_livres[i] == 0) {
                    bloco_livre = i;
                    bitmap.blocos_livres[i] = 1; // Marca como usado
                    break;
                }
            }
            if (bloco_livre == -1) {
                printf("Erro: Espaço insuficiente para copiar o arquivo.\n");
                fclose(origem_fp);
                return;
            }

            if (primeiro_bloco == -1) primeiro_bloco = bloco_livre; // Define o primeiro bloco
            if (ultimo_bloco != -1) fat.prox_bloco[ultimo_bloco] = bloco_livre;
            ultimo_bloco = bloco_livre;
            fat.prox_bloco[ultimo_bloco] = 0xFFFF; // Final da cadeia

            char buffer[BLOCK_SIZE] = {0};
            size_t bytes_lidos = fread(buffer, 1, BLOCK_SIZE, origem_fp);
            escreve_bloco(bloco_livre, buffer);
            tamanho -= bytes_lidos;
        }

        fclose(origem_fp);
        escreve_bloco(0, &bitmap);
        escreve_bloco(1, &fat);

        // Salvando metadados do arquivo destino
        Metadados destino_meta;
        strcpy(destino_meta.nome, destino);
        destino_meta.tamanho = ftell(origem_fp);
        destino_meta.criado = destino_meta.modificado = destino_meta.acessado = time(NULL);
        destino_meta.primeiro_bloco = primeiro_bloco;
        destino_meta.eh_diretorio = 0;
        escreve_bloco(primeiro_bloco, &destino_meta); // Grava metadados no primeiro bloco do arquivo
    } else {
        printf("Erro: Arquivo de origem não encontrado.\n");
    }
}





void criadir(char *diretorio) {
    
    int bloco_livre = -1;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (bitmap.blocos_livres[i] == 0) { 
            bloco_livre = i;
            break;
        }
    }

    if (bloco_livre == -1) {
        printf("Erro: Sem espaço de armazenamento disponível.\n");
        return;
    }

   
    bitmap.blocos_livres[bloco_livre] = 1;

    Metadados dir_meta;
    strcpy(dir_meta.nome, diretorio);
    dir_meta.tamanho = 0;
    dir_meta.criado = dir_meta.modificado = dir_meta.acessado = time(NULL);
    dir_meta.primeiro_bloco = bloco_livre;
    dir_meta.eh_diretorio = 1;


    escreve_bloco(bloco_livre, &dir_meta);
    escreve_bloco(0, &bitmap);  
}



void apagadir(char *diretorio) {
    int bloco_diretorio = localiza_arquivo(diretorio, 0);
    if (bloco_diretorio == -1) {
        printf("Diretório não encontrado.\n");
        return;
    }

    Metadados dir_meta;
    le_bloco(bloco_diretorio, &dir_meta);

    if (!dir_meta.eh_diretorio) {
        printf("Erro: %s não é um diretório.\n", diretorio);
        return;
    }

    uint16_t bloco_atual = dir_meta.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        Metadados buffer;
        le_bloco(bloco_atual, &buffer);

        if (buffer.eh_diretorio) {
            apagadir(buffer.nome); // Recursivamente apaga subdiretórios
        } else {
            apaga(buffer.nome); // Apaga arquivos
        }

        bitmap.blocos_livres[bloco_atual] = 0;
        uint16_t proximo_bloco = fat.prox_bloco[bloco_atual];
        fat.prox_bloco[bloco_atual] = 0xFFFF;
        bloco_atual = proximo_bloco;
    }

    escreve_bloco(0, &bitmap);
    escreve_bloco(1, &fat);
}


void mostra(char *arquivo) {
    int bloco_arquivo = localiza_arquivo(arquivo, 0); // Supõe que o diretório raiz está no bloco 0
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }

    Metadados meta;
    le_bloco(bloco_arquivo, &meta);

    uint16_t bloco_atual = meta.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        le_bloco(bloco_atual, buffer);
        printf("%s", buffer);
        bloco_atual = fat.prox_bloco[bloco_atual];
    }
    printf("\n");
}




void toca(char *arquivo) {
    int bloco_arquivo = localiza_arquivo(arquivo, 0);
    if (bloco_arquivo == -1) {
        // Cria um arquivo vazio se não existir
        copia("/dev/null", arquivo); // "/dev/null" cria um arquivo vazio
        return;
    }

    Metadados meta;
    le_bloco(bloco_arquivo, &meta);
    meta.acessado = time(NULL);
    escreve_bloco(bloco_arquivo, &meta);
}


void apaga(char *arquivo) {
    int bloco_arquivo = localiza_arquivo(arquivo, 0);
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }

    Metadados meta;
    le_bloco(bloco_arquivo, &meta);
    uint16_t bloco_atual = meta.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        bitmap.blocos_livres[bloco_atual] = 0;
        uint16_t proximo_bloco = fat.prox_bloco[bloco_atual];
        fat.prox_bloco[bloco_atual] = 0xFFFF;
        bloco_atual = proximo_bloco;
    }

    escreve_bloco(0, &bitmap);
    escreve_bloco(1, &fat);
}

void lista(char *diretorio) {
    int bloco_diretorio = localiza_arquivo(diretorio, 0);
    if (bloco_diretorio == -1) {
        printf("Diretório não encontrado.\n");
        return;
    }

    Metadados dir_meta;
    le_bloco(bloco_diretorio, &dir_meta);

    if (!dir_meta.eh_diretorio) {
        printf("Erro: %s não é um diretório.\n", diretorio);
        return;
    }

    uint16_t bloco_atual = dir_meta.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        Metadados buffer;
        le_bloco(bloco_atual, &buffer);

       //  printf("%s %s %u bytes\n", buffer.eh_diretorio ? "D" : "F", buffer.nome, buffer.tamanho);

        bloco_atual = fat.prox_bloco[bloco_atual];
    }
}





void atualizadb() {

//     FILE *db = fopen("database.txt", "w");
//     if (!db) {
//         printf("Erro ao criar banco de dados.\n");
//         return;
//     }

//     atualiza_recursivo(0, db);
//     fclose(db);
 }

void busca(char *string) {
    FILE *db = fopen("database.txt", "r");
    if (!db) {
        printf("Banco de dados não encontrado. Execute 'atualizadb' primeiro.\n");
        return;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), db)) {
        if (strstr(linha, string)) {
            printf("%s", linha);
        }
    }

    fclose(db);
}

    void conta_recursivo(uint16_t bloco_atual, int* total_arquivos, int* total_diretorios) {
        while (bloco_atual != 0xFFFF) {
            Metadados buffer;
            le_bloco(bloco_atual, &buffer);

            if (buffer.eh_diretorio) {
                total_diretorios++;
                conta_recursivo(buffer.primeiro_bloco, total_arquivos, total_diretorios);
            } else {
                total_arquivos++;
            }

            bloco_atual = fat.prox_bloco[bloco_atual];
        }
    }

void status() {
    int total_arquivos = 0, total_diretorios = 0, espaco_usado = 0, espaco_livre = 0;

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (bitmap.blocos_livres[i]) {
            espaco_usado += BLOCK_SIZE;
        } else {
            espaco_livre += BLOCK_SIZE;
        }
    }



    conta_recursivo(0, &total_arquivos, &total_diretorios);

    printf("Total de arquivos: %d\n", total_arquivos);
    printf("Total de diretórios: %d\n", total_diretorios);
    printf("Espaço usado: %d bytes\n", espaco_usado);
    printf("Espaço livre: %d bytes\n", espaco_livre);
}

void desmonta() {
    escreve_bloco(0, &bitmap);
    escreve_bloco(1, &fat);
    fclose(sistema_arquivos);
    sistema_arquivos = NULL;
}

int main() {

    char comando[256];
    printf("{ep3}: ");

    while (fgets(comando, sizeof(comando), stdin)) {

        comando[strcspn(comando, "\n")] = '\0'; 
        char *acao = strtok(comando, " ");

        if (strcmp(acao, "monta") == 0) {
            monta(strtok(NULL, " "));
        }
        else if (strcmp(acao, "desmonta") == 0) {
            desmonta();
        }
        else 

        if (strcmp(acao, "copia") == 0) {
            copia(strtok(NULL, " "), strtok(NULL, " "));
        }
        else if (strcmp(acao, "criadir") == 0) {
            criadir(strtok(NULL, " "));
        }
        else if (strcmp(acao, "apagadir") == 0) {
            apagadir(strtok(NULL, " "));
        }
        else if (strcmp(acao, "mostra") == 0) {
            mostra(strtok(NULL, " "));
        }
        else if (strcmp(acao, "toca") == 0) {
            toca(strtok(NULL, " "));
        }
        else if (strcmp(acao, "apaga") == 0) {
            apaga(strtok(NULL, " "));
        }
        else if (strcmp(acao, "lista") == 0) {
            lista(strtok(NULL, " "));
        }
        else if (strcmp(acao, "atualizadb") == 0) {
            atualizadb();
        }
        else if (strcmp(acao, "busca") == 0) {
            busca(strtok(NULL, " "));
        }
        else if (strcmp(acao, "status") == 0) {
            status();
        }
        else if (strcmp(acao, "sai") == 0) {
            break;
        }
        
        else {
            printf("Comando desconhecido\n");
        }
        printf("{ep3}: ");
    }

    desmonta();
    
    return 0;
}