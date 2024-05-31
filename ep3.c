#include "ep3.h"


FAT fat;
Bitmap bitmap;
FILE *sistema_arquivos;

void escreve_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, num_bloco * BLOCK_SIZE, SEEK_SET);
    fwrite(dados, BLOCK_SIZE, 1, sistema_arquivos);
}

void le_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, num_bloco * BLOCK_SIZE, SEEK_SET);
    fread(dados, BLOCK_SIZE, 1, sistema_arquivos);
}

int localiza_arquivo(char *nome_arquivo, uint32_t diretório_raiz) {
    Metadados buffer;
    int bloco_atual = diretório_raiz;
    while (bloco_atual != 0xFFFF) { // Continua até o final da cadeia de blocos
        le_bloco(bloco_atual, &buffer);
        if (strcmp(buffer.nome, nome_arquivo) == 0) {
            return bloco_atual;  // Retorna o bloco onde o arquivo foi encontrado
        }
        bloco_atual = fat.prox_bloco[bloco_atual]; // Move para o próximo bloco do diretório
    }
    return -1;  // Retorna -1 se o arquivo não foi encontrado
}



void monta(char *arquivo) {
    sistema_arquivos = fopen(arquivo, "r+b");  
    
    // Se o arquivo não existir, cria um novo
    // inicializa o bitmap liberando os blocos e reserva espaço para o fat

    if (!sistema_arquivos) {
        sistema_arquivos = fopen(arquivo, "w+b");  
        memset(bitmap.blocos_livres, 0, sizeof(bitmap.blocos_livres));  

        escreve_bloco(0, &bitmap);  

        
        memset(&fat, 0xFF, sizeof(fat)); 
        escreve_bloco(1, &fat);  
    } 
     // Se o arquivo já existir, carrega o bitmap e o FAT do disco.
    else {
       
        le_bloco(0, &bitmap);
        le_bloco(1, &fat);
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
    // Implementação
}

void atualizadb() {
    // Implementação
}

void busca(char *string) {
    // Implementação
}

void status() {
    // Implementação
}

void desmonta() {
    
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
        else if (strcmp(acao, "copia") == 0) {
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