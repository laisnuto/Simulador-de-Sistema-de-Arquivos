#include "ep3.h"


FAT fat;
Bitmap bitmap;
FILE *sistema_arquivos;
int primeiro_bloco_raiz = 0;

void atualiza_fat_bitmap() {
    fseek(sistema_arquivos, 0, SEEK_SET);
    fwrite(&bitmap, sizeof(Bitmap), 1, sistema_arquivos);
    fwrite(&fat, sizeof(FAT), 1, sistema_arquivos);
}

void carrega_fat_bitmap() {
    fseek(sistema_arquivos, 0, SEEK_SET);
    fread(&bitmap, sizeof(Bitmap), 1, sistema_arquivos);
    fread(&fat, sizeof(FAT), 1, sistema_arquivos);
}


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


int acha_arquivo(char *nome_arquivo, uint32_t diretório_raiz) {

    Metadados buffer;
    uint32_t bloco_atual = diretório_raiz;
    while (bloco_atual != 0xFFFF) { 
        le_bloco(bloco_atual, &buffer);
        if (strcmp(buffer.nome, nome_arquivo) == 0) {
            return bloco_atual;  
        }
        bloco_atual = fat.prox_bloco[bloco_atual]; 
    }
    return -1;  
}

int encontrar_bloco_livre() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (bitmap.blocos_livres[i] == 0) {
            return i;
        }
    }
    return -1;
}


void escreve_metadados(uint16_t bloco_inicial, Metadados *meta) {
    uint16_t bloco_atual = bloco_inicial;
    uint16_t ultimo_bloco = 0xFFFF;

    while (bloco_atual != 0xFFFF) {
        ultimo_bloco = bloco_atual;
        bloco_atual = fat.prox_bloco[bloco_atual];
    }

        
    int bloco_livre = encontrar_bloco_livre();
    if (bloco_livre == -1) {
        fprintf(stderr, "Não há blocos livres disponíveis.\n");
        return;
    }


    if (ultimo_bloco != 0xFFFF) {
        fat.prox_bloco[ultimo_bloco] = bloco_livre;
    } else {
        bloco_inicial = bloco_livre;
    }


    bitmap.blocos_livres[bloco_livre] = 1; 


    escreve_bloco(ultimo_bloco, meta);
    atualiza_fat_bitmap(); 
}



int encontrar_bloco_diretorio_pai(const char *caminho) {
    int ultima_barra = -1;

    if (strcmp(caminho, "/") == 0) {
        return -1;
    }

    char caminho_copia[MAX_FILENAME];
    strncpy(caminho_copia, caminho, MAX_FILENAME);
    caminho_copia[MAX_FILENAME] = '\0';

    for (int i = strlen(caminho_copia) - 1; i >= 0; i--) {
        if (caminho_copia[i] == '/') {
            ultima_barra = i;
            break;
        }
    }

    if (ultima_barra == -1) {
        return -1;
    }

    if(ultima_barra == 0) {
        return primeiro_bloco_raiz;
    }

    
    caminho_copia[ultima_barra] = '\0';

    return acha_arquivo(caminho_copia, primeiro_bloco_raiz);
}



// função que monta o sistema de arquivos, se já existir, imprime a árvore de diretórios
void monta(char *path) {
    int tamanho_bitmap = sizeof(Bitmap);
    int tamanho_fat = sizeof(FAT);

    sistema_arquivos = fopen(path, "r+b"); 

    if (sistema_arquivos == NULL) {
       
        sistema_arquivos = fopen(path, "w+b");
        if (sistema_arquivos == NULL) {
            fprintf(stderr, "Erro ao criar o arquivo do sistema de arquivos.\n");
            exit(1);
        }

        
        memset(&bitmap, 0, sizeof(Bitmap));
        memset(&fat, 0xFF, sizeof(FAT)); 

        
        fwrite(&bitmap, tamanho_bitmap, 1, sistema_arquivos);
        fwrite(&fat, tamanho_fat, 1, sistema_arquivos);

        
        bitmap.blocos_livres[0] = 1; 

        
        fseek(sistema_arquivos, 0, SEEK_SET);
        fwrite(&bitmap, tamanho_bitmap, 1, sistema_arquivos);

        char bloco_vazio[BLOCK_SIZE] = {0}; 
        for (int i = 0; i < MAX_BLOCKS; i++) {
            fwrite(bloco_vazio, BLOCK_SIZE, 1, sistema_arquivos); 
        }


        printf("Sistema de arquivos criado com sucesso.\n");
       

    }
    
    else {
        
        carrega_fat_bitmap();
        imprime_arvore(primeiro_bloco_raiz, 0);
    }
}




void copia(char *origem, char *destino) {
    Metadados metadados_destino;

    printf("Copiando %s para  ...\n", origem);
    
    FILE *arquivo_origem = fopen(origem, "r");
    if (arquivo_origem == NULL) {
        perror("Erro ao abrir o arquivo de origem");
        return;
    }

    int bloco_livre = encontrar_bloco_livre();
    if (bloco_livre == -1) {
        fprintf(stderr, "Não há espaço livre no sistema de arquivos.\n");
        fclose(arquivo_origem);
        return;
    }

    
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    int bytes_lidos = 0;
    
    strcpy(metadados_destino.nome, destino);
    metadados_destino.eh_diretorio = 0;
    metadados_destino.tamanho = 0;
    metadados_destino.criado = metadados_destino.modificado = metadados_destino.acessado = time(NULL);
    metadados_destino.primeiro_bloco = bloco_livre;

    while ((bytes_lidos = fread(buffer, 1, BLOCK_SIZE, arquivo_origem)) > 0) {
       
        escreve_bloco(bloco_livre, buffer);
        bitmap.blocos_livres[bloco_livre] = 1;
        metadados_destino.tamanho += bytes_lidos;

        uint16_t ultimo_bloco = bloco_livre;
        bloco_livre = encontrar_bloco_livre();

        if (bloco_livre == -1) {
            fprintf(stderr, "Não há mais espaço livre no sistema de arquivos.\n");
            fclose(arquivo_origem);
            return;
        }

        fat.prox_bloco[ultimo_bloco] = bloco_livre;
        
    }

    fclose(arquivo_origem);

    atualiza_fat_bitmap();
      
    int bloco_pai = encontrar_bloco_diretorio_pai(destino);
    if (bloco_pai != -1) {
        escreve_metadados(bloco_pai, &metadados_destino);
    } else {
        fprintf(stderr, "Não foi encontrado o diretório pai do destino.\n");
    }

    
}




void criadir(char *diretorio) {
    int bloco_pai = encontrar_bloco_diretorio_pai(diretorio);
    if (bloco_pai == -1) {
        fprintf(stderr, "Diretório pai não encontrado.\n");
        return;
    }

    int bloco_livre = encontrar_bloco_livre();

    if (bloco_livre == -1) {
        fprintf(stderr, "Não há espaço livre no sistema de arquivos.\n");
        return;
    }

    Metadados metadados;
    strcpy(metadados.nome, diretorio);
    metadados.eh_diretorio = 1;
    metadados.tamanho = 0;
    metadados.criado = metadados.modificado = metadados.acessado = time(NULL);
    metadados.primeiro_bloco = bloco_livre;

    escreve_metadados(bloco_pai, &metadados);

    bitmap.blocos_livres[bloco_livre] = 1;

    atualiza_fat_bitmap();
}


void limpa_diretorio(uint16_t bloco_diretorio) {
    uint16_t bloco_atual = bloco_diretorio;
    while (bloco_atual != 0xFFFF) {
        Metadados buffer;
        le_bloco(bloco_atual, &buffer);
        if (buffer.eh_diretorio) {
            limpa_diretorio(buffer.primeiro_bloco);
        }
        printf("Apagando %s ...\n", buffer.nome);
        bitmap.blocos_livres[bloco_atual] = 0; 
        uint16_t proximo = fat.prox_bloco[bloco_atual];
        fat.prox_bloco[bloco_atual] = 0xFFFF; 
        bloco_atual = proximo;
    }
}

void apagadir(char *diretorio) {
    int bloco_diretorio = acha_arquivo(diretorio, primeiro_bloco_raiz);
    if (bloco_diretorio == -1) {
        fprintf(stderr, "Diretório não encontrado.\n");
        return;
    }

    limpa_diretorio(bloco_diretorio);

    atualiza_fat_bitmap();
}



void mostra(char *arquivo) {
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }

    Metadados metadata;
    le_bloco(bloco_arquivo, &metadata);

    char buffer[BLOCK_SIZE + 1];
    uint16_t bloco_atual = metadata.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        memset(buffer, 0, BLOCK_SIZE + 1);
        le_bloco(bloco_atual, buffer);
        printf("%s", buffer);
        bloco_atual = fat.prox_bloco[bloco_atual];
    }
    printf("\n");
}



void cria_arquivo_vazio(char *arquivo) {
    
    int bloco_pai = encontrar_bloco_diretorio_pai(arquivo);
    if (bloco_pai == -1) {
        fprintf(stderr, "Diretório pai não encontrado.\n");
        return;
    }

    int bloco_livre = encontrar_bloco_livre();
    if (bloco_livre == -1) {
        fprintf(stderr, "Não há espaço livre no sistema de arquivos.\n");
        return;
    }

    Metadados metadados;
    strcpy(metadados.nome, arquivo);
    metadados.eh_diretorio = 0;
    metadados.tamanho = 0;
    metadados.criado = metadados.modificado = metadados.acessado = time(NULL);
    metadados.primeiro_bloco = bloco_livre;

    bitmap.blocos_livres[bloco_livre] = 1;


    escreve_metadados(primeiro_bloco_raiz, &metadados);

    atualiza_fat_bitmap();
}

void toca(char *arquivo) {
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        cria_arquivo_vazio(arquivo);
    } else {
        int bloco_pai = encontrar_bloco_diretorio_pai(arquivo);
        Metadados metadata;
        le_bloco(bloco_pai, &metadata);
        metadata.acessado = time(NULL);
        escreve_bloco(bloco_pai, &metadata);
    }
}


void limpa_arquivo(uint16_t bloco_arquivo) {
    Metadados metadata;
    le_bloco(bloco_arquivo, &metadata);

    uint16_t bloco_atual = metadata.primeiro_bloco;
    while (bloco_atual != 0xFFFF) {
        uint16_t proximo = fat.prox_bloco[bloco_atual];
        fat.prox_bloco[bloco_atual] = 0xFFFF;
        bitmap.blocos_livres[bloco_atual] = 0;
        bloco_atual = proximo;
    }
}

void remove_do_diretorio_pai(char *arquivo, uint16_t bloco_arquivo) {
    int bloco_pai = encontrar_bloco_diretorio_pai(arquivo);
    if (bloco_pai == -1) {
        fprintf(stderr, "Diretório pai não encontrado.\n");
        return;
    }

    Metadados pai;
    le_bloco(bloco_pai, &pai);

    uint16_t bloco_atual = pai.primeiro_bloco;
    uint16_t bloco_anterior = 0xFFFF;

    while (bloco_atual != 0xFFFF) {
        Metadados buffer;
        le_bloco(bloco_atual, &buffer);

        if (bloco_atual == bloco_arquivo) {
            if (bloco_anterior == 0xFFFF) {
                // Primeiro bloco do diretório
                pai.primeiro_bloco = fat.prox_bloco[bloco_atual];
            } else {
                // Não é o primeiro bloco
                fat.prox_bloco[bloco_anterior] = fat.prox_bloco[bloco_atual];
            }
            fat.prox_bloco[bloco_atual] = 0xFFFF; // Isola o bloco do arquivo deletado
            escreve_bloco(bloco_pai, &pai); // Atualiza o diretório pai
            atualiza_fat_bitmap();
            return;
        }

        bloco_anterior = bloco_atual;
        bloco_atual = fat.prox_bloco[bloco_atual];
    }
}

void apaga(char *arquivo) {
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }

    // Remove o arquivo do diretório pai antes de limpá-lo
    remove_do_diretorio_pai(arquivo, bloco_arquivo);

    // Agora limpa o arquivo e seus blocos
    limpa_arquivo(bloco_arquivo);
    atualiza_fat_bitmap();
}





void lista(char *diretorio) {
    int bloco_diretorio = acha_arquivo(diretorio, 0);
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