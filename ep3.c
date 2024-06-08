#include "ep3.h"


FAT fat;
Bitmap bitmap;
FILE *sistema_arquivos;
No* primeiro_no = NULL;
uint16_t primeiro_bloco_raiz = 0;

// função que escreve o bitmap e a FAT do sistema de arquivos
void atualiza_fat_bitmap() {
    fseek(sistema_arquivos, 0, SEEK_SET);
    fwrite(&bitmap, sizeof(Bitmap), 1, sistema_arquivos);
    fwrite(&fat, sizeof(FAT), 1, sistema_arquivos);
    fflush(sistema_arquivos);
}

// função que le o bitmap e a FAT do sistema de arquivos
void carrega_fat_bitmap() {
    fseek(sistema_arquivos, 0, SEEK_SET);
    fread(&bitmap, sizeof(Bitmap), 1, sistema_arquivos);
    fread(&fat, sizeof(FAT), 1, sistema_arquivos);
}

// função que escreve um bloco no sistema de arquivos no byte correspondente
void escreve_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, FAT_BITMAP_SIZE + num_bloco * BLOCK_SIZE, SEEK_SET);
    fwrite(dados, BLOCK_SIZE, 1, sistema_arquivos);
}

// função que le um bloco do sistema de arquivos no byte correspondente
void le_bloco(int num_bloco, void *dados) {
    fseek(sistema_arquivos, FAT_BITMAP_SIZE + num_bloco * BLOCK_SIZE, SEEK_SET);
    fread(dados, BLOCK_SIZE, 1, sistema_arquivos);
}


// função que imprime a árvore de diretórios
void imprime_arvore(uint16_t bloco_atual, int nivel) {    

    if (bloco_atual == 0xFFFF || bloco_atual >= MAX_BLOCKS) {
        return; 
    }

    if (bloco_atual == primeiro_bloco_raiz && nivel == 0) {
        printf("/\n");
        nivel++;  
    }

    while (bloco_atual != 0xFFFF) { 
       
        Metadados metadados[MAX_METADATA_PER_BLOCK];
        
        le_bloco(bloco_atual, &metadados); 

        for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
            
            if (metadados[i].primeiro_bloco != 0 || metadados[i].tamanho != 0) {
                for (int j = 0; j < nivel; j++) {
                    printf("  "); 
                }
                printf("%s", metadados[i].nome);
                if (metadados[i].eh_diretorio) {

                    printf("/ \n");
                    imprime_arvore(metadados[i].primeiro_bloco, nivel + 1);
                }
                else {
                    printf("\n");
                }
            }
        }
        
        bloco_atual = fat.prox_bloco[bloco_atual]; 
       
    }
}

const char* extrair_nome_arquivo(const char* caminho) {
    if (caminho == NULL) {
        return ""; 
    }

    const char* ultimo_slash = strrchr(caminho, '/'); 
    if (ultimo_slash == NULL) {
        return caminho;
    } else if (*(ultimo_slash + 1) == '\0') {
        return ""; 
    } else {
        return ultimo_slash + 1; 
    }
}

// função que localiza o arquivo pelo caminho e retorna o bloco onde ele está
int acha_arquivo(const char* caminho, uint32_t bloco_atual) {

    if (strcmp(caminho, "/") == 0) {
        printf("Caminho é a raiz. Bloco raiz: %d\n", primeiro_bloco_raiz);
        return primeiro_bloco_raiz;         
    }

    if (caminho == NULL || *caminho == '\0') {
        printf("Caminho inválido ou vazio.\n");
        return -1; 
    }

    char caminho_copia[MAX_FILENAME];
    strncpy(caminho_copia, caminho, MAX_FILENAME);
    caminho_copia[MAX_FILENAME-1] = '\0';

    char *segmento = strtok(caminho_copia, "/");
    

    while (segmento != NULL) {
        Metadados buffer[MAX_METADATA_PER_BLOCK];
        le_bloco(bloco_atual, buffer);

        int encontrado = 0;
        for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
            
            if (strcmp(buffer[i].nome, segmento) == 0) {
               
                if (buffer[i].eh_diretorio == 0 && strtok(NULL, "/") != NULL) {
                    printf("Caminho inválido: %s não é um diretório.\n", segmento);
                    return -1; 
                }
                bloco_atual = buffer[i].primeiro_bloco; 
                encontrado = 1;
                break;
            }
        }

        if (!encontrado) {
            
            return -1; 
        }

        segmento = strtok(NULL, "/");       
        
    }
  
    return bloco_atual;
}


// Função para localizar um bloco livre se exitir
int encontrar_bloco_livre() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
      
        if (bitmap.blocos_livres[i] == 0) {
            
            return i;

        }
    }
    return -1;
}



//Função de escrever metadados em um diretório dado o primeiro bloco do diretório
void escreve_metadados(uint16_t bloco_inicial, Metadados *meta) {
    Metadados buffer[16];
    int encontrou_espaco = 0;
    uint16_t bloco_atual = bloco_inicial;
    uint16_t ultimo_bloco = 0xFFFF;

    while (bloco_atual != 0xFFFF && !encontrou_espaco) {
        le_bloco(bloco_atual, &buffer);
        for (int i = 0; i < 16; i++) {
            if (buffer[i].primeiro_bloco == 0 && buffer[i].tamanho == 0) { 
                buffer[i] = *meta;  
                escreve_bloco(bloco_atual, &buffer);  
                encontrou_espaco = 1;
                break;
            }
        }
        ultimo_bloco = bloco_atual;
        bloco_atual = fat.prox_bloco[bloco_atual];
    }


    if (!encontrou_espaco) {
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
        memset(&buffer, 0, sizeof(buffer));  
        buffer[0] = *meta;  
        escreve_bloco(bloco_livre, &buffer);  
    }

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

    sistema_arquivos = fopen(path, "r+b"); 

    if (sistema_arquivos == NULL) {
       
        sistema_arquivos = fopen(path, "w+b");
        if (sistema_arquivos == NULL) {
            fprintf(stderr, "Erro ao criar o arquivo do sistema de arquivos.\n");
            exit(1);
        }

        
        memset(&bitmap, 0, sizeof(Bitmap));
        for (int i = 0; i < MAX_BLOCKS; i++) {
            fat.prox_bloco[i] = 0xFFFF;
        }
        
        bitmap.blocos_livres[0] = 1; 
        fat.prox_bloco[primeiro_bloco_raiz] = -1;
  
        atualiza_fat_bitmap();

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



// função que copia um arquivo normal para o sistema de arquivos simulado
void copia(char *origem, char *destino) {

    const char* nome_origem = extrair_nome_arquivo(origem);
    int arquivo_existente = acha_arquivo(destino, primeiro_bloco_raiz);
    if (arquivo_existente != -1) {
        fprintf(stderr, "O arquivo já existe \n");
        return;
    }

    Metadados metadados_destino;

    
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
    const char* nome_destino = extrair_nome_arquivo(destino);
    strcpy(metadados_destino.nome, nome_destino);
    metadados_destino.eh_diretorio = 0;
    metadados_destino.tamanho = 0;
    metadados_destino.criado = time(NULL);
    metadados_destino.modificado = time(NULL);
    metadados_destino.acessado = time(NULL);
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

    fat.prox_bloco[bloco_livre] = -1;
    fclose(arquivo_origem);
    atualiza_fat_bitmap();
   
      
    int bloco_pai = encontrar_bloco_diretorio_pai(destino);


  
    if (bloco_pai != -1) {
        escreve_metadados(bloco_pai, &metadados_destino);
    } else {
        fprintf(stderr, "Não foi encontrado o diretório pai do destino.\n");
    }

    printf("Arquivo %s copiado para %s\n", nome_origem, nome_destino);
}




void criadir(char *diretorio) {

    int arquivo_existente = acha_arquivo(diretorio, primeiro_bloco_raiz);
    if (arquivo_existente != -1) {
        fprintf(stderr, "Esse diretório já existe \n");
        return;
    }

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
    const char* nome_diretorio = extrair_nome_arquivo(diretorio);
    strcpy(metadados.nome, nome_diretorio);
    metadados.eh_diretorio = 1;
    metadados.tamanho = 0;
    metadados.criado = time(NULL);
    metadados.modificado = time(NULL);
    metadados.acessado = time(NULL);
    metadados.primeiro_bloco = bloco_livre;

    escreve_metadados(bloco_pai, &metadados);

    bitmap.blocos_livres[bloco_livre] = 1;
    

    atualiza_fat_bitmap();

}


void apaga_metadados_pai(uint16_t bloco_pai,const char* nome_arquivo_apagado) {
   
   Metadados metadados[MAX_METADATA_PER_BLOCK];
   le_bloco(bloco_pai, &metadados);
   int achou = 0;

    while (bloco_pai != 0xFFFF) {
        Metadados metadados[MAX_METADATA_PER_BLOCK];
        le_bloco(bloco_pai, &metadados);

       
        for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
             if (strcmp(metadados[i].nome, nome_arquivo_apagado) == 0) {
                memset(&metadados[i], 0, sizeof(Metadados));
                escreve_bloco(bloco_pai, metadados);
                achou = 1; 
                break;
            }
        }

        if (achou) {
            break;
        }
     
        uint16_t proximo = fat.prox_bloco[bloco_pai];

        bloco_pai = proximo;
    }
}



void limpa_arquivo(uint16_t bloco_arquivo) {
    uint16_t bloco_atual = bloco_arquivo;
    char bloco_vazio[BLOCK_SIZE] = {0}; 

    while (bloco_atual != 0xFFFF) {
        uint16_t proximo = fat.prox_bloco[bloco_atual];
        escreve_bloco(bloco_atual, bloco_vazio);  
        bitmap.blocos_livres[bloco_atual] = 0;
        fat.prox_bloco[bloco_atual] = 0xFFFF;
        bloco_atual = proximo;
    }
    atualiza_fat_bitmap(); 
}




void limpa_diretorio(uint16_t bloco_diretorio) {
    uint16_t bloco_atual = bloco_diretorio;

    while (bloco_atual != 0xFFFF) {
        Metadados metadados[MAX_METADATA_PER_BLOCK];
        le_bloco(bloco_atual, &metadados); 

       
        for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
            if (metadados[i].primeiro_bloco != 0 || metadados[i].tamanho != 0) { 
                if (metadados[i].eh_diretorio) {
                   
                    limpa_diretorio(metadados[i].primeiro_bloco);
                }
                
                limpa_arquivo(metadados[i].primeiro_bloco);
                
               
                printf("Apagando %s...\n", metadados[i].nome);
            }
        }

     
        uint16_t proximo = fat.prox_bloco[bloco_atual];
        bloco_atual = proximo;
    }

    atualiza_fat_bitmap();
}


void apagadir(char *diretorio) {
    int bloco_diretorio = acha_arquivo(diretorio, primeiro_bloco_raiz);
    if (bloco_diretorio == -1) {
        fprintf(stderr, "Diretório não encontrado.\n");
        return;
    }

    limpa_diretorio(bloco_diretorio);
    limpa_arquivo(bloco_diretorio);

    int bloco_pai = encontrar_bloco_diretorio_pai(diretorio);
    const char* nome_diretorio = extrair_nome_arquivo(diretorio);
    
    
    apaga_metadados_pai(bloco_pai, nome_diretorio);

    printf("Diretório %s apagado.\n", nome_diretorio);

    atualiza_fat_bitmap();
}



void mostra(char *arquivo) {
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }



    char buffer[BLOCK_SIZE + 1];
    uint16_t bloco_atual = bloco_arquivo;
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

    const char* nome = extrair_nome_arquivo(arquivo);
    Metadados metadados;
    strcpy(metadados.nome, nome);
    metadados.eh_diretorio = 0;
    metadados.tamanho = 0;
    metadados.criado = metadados.modificado = metadados.acessado = time(NULL);
    metadados.primeiro_bloco = bloco_livre;

    bitmap.blocos_livres[bloco_livre] = 1;


    escreve_metadados(bloco_pai, &metadados);

    atualiza_fat_bitmap();
}


void toca(char *arquivo) {
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        cria_arquivo_vazio(arquivo);
        printf("O arquivo vazio %s acabou de ser criado.\n", extrair_nome_arquivo(arquivo));
    }  else {
      
        int bloco_pai = encontrar_bloco_diretorio_pai(arquivo);
        if (bloco_pai == -1) {
            printf("Diretório pai não encontrado.\n");
            return;
        }

        int achou = 0;
        const char* nome = extrair_nome_arquivo(arquivo); 

        while (bloco_pai != 0xFFFF) {
            Metadados metadados[MAX_METADATA_PER_BLOCK];
            le_bloco(bloco_pai, &metadados);

        
            for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
                    if (strcmp(metadados[i].nome, nome) == 0) {
                    metadados[i].acessado = time(NULL); 
                    escreve_bloco(bloco_pai, &metadados);
                    achou = 1; 
                    break;
                }
            }
            

            if (achou) {
                break;
            }
        
            uint16_t proximo = fat.prox_bloco[bloco_pai];

            bloco_pai = proximo;
        }
        
       
        printf("O arquivo %s foi acessado.\n", nome);
    }
}

void apaga(char *arquivo) {
   const char* nome_arquivo = extrair_nome_arquivo(arquivo);
    int bloco_arquivo = acha_arquivo(arquivo, primeiro_bloco_raiz);
    if (bloco_arquivo == -1) {
        printf("Arquivo não encontrado.\n");
        return;
    }

    
    
    int bloco_pai = encontrar_bloco_diretorio_pai(arquivo);
    if (bloco_pai == -1) {
        printf("Diretório pai não encontrado.\n");
        return;
    }

    limpa_arquivo(bloco_arquivo);

    apaga_metadados_pai(bloco_pai, nome_arquivo);


    
    atualiza_fat_bitmap();
}



void imprime_tempo(time_t tempo) {
    char buffer[20];
    struct tm *tm_info = localtime(&tempo);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s    ", buffer);
}

void lista(char *diretorio) {
    int bloco_diretorio = acha_arquivo(diretorio, 0);
    if (bloco_diretorio == -1) {
        printf("Diretório não encontrado.\n");
        return;
    }

   
    printf("%-24s %-10s %-24s %-24s %-24s\n", "Nome", "Tamanho", "Data de Criação", "Última Modificação", "Último Acesso");

    while (bloco_diretorio != 0xFFFF) { 
        Metadados metadados[MAX_METADATA_PER_BLOCK];
        le_bloco(bloco_diretorio, &metadados); 

        for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
            if (metadados[i].primeiro_bloco != 0 || metadados[i].tamanho != 0) { 

                if(metadados[i].eh_diretorio) {
                    printf("/%-24s", metadados[i].nome);
                } else {
                    printf("%-24s", metadados[i].nome);
                }
              
                printf("%-10d ", metadados[i].tamanho);
                imprime_tempo(metadados[i].criado);
                printf(" ");
                imprime_tempo(metadados[i].modificado);
                printf(" ");
                imprime_tempo(metadados[i].acessado);
                printf("\n");
            }
        }
        bloco_diretorio = fat.prox_bloco[bloco_diretorio]; 
    }
}


void limpa_db() {
    while (primeiro_no) {
        No *atual = primeiro_no;
        primeiro_no = primeiro_no->prox;
        free(atual);
    }
}


void adiciona_ao_banco_de_dados(const char* caminho, const char* nome) {
    No *novo = malloc(sizeof(No));
    strcpy(novo->caminho, caminho);
    strcpy(novo->nome, nome);
    novo->prox = NULL;

    if (primeiro_no == NULL) {
        primeiro_no = novo;
    } else {
        No *atual = primeiro_no;
        while (atual->prox != NULL) {
            atual = atual->prox;
        }
        atual->prox = novo;
    }
}


void atualiza_recursivo(uint16_t bloco_atual, const char* caminho_atual) {
    if (bloco_atual == 0xFFFF) return;

    Metadados metadados[MAX_METADATA_PER_BLOCK];
    le_bloco(bloco_atual, &metadados);

    for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
        if (metadados[i].primeiro_bloco != 0 || metadados[i].tamanho != 0) {
            char novo_caminho[256];
            snprintf(novo_caminho, sizeof(novo_caminho), "%s/%s", caminho_atual, metadados[i].nome);
            adiciona_ao_banco_de_dados(novo_caminho, metadados[i].nome);

            if (metadados[i].eh_diretorio) {
                atualiza_recursivo(metadados[i].primeiro_bloco, novo_caminho);
            }
        }
    }
}


void atualizadb() {
    limpa_db();
    char caminho_raiz[] = "";
    adiciona_ao_banco_de_dados(caminho_raiz, "");
    atualiza_recursivo(primeiro_bloco_raiz, caminho_raiz);
}



void busca(const char* string) {
    No* atual = primeiro_no;
    while (atual) {
        if (strstr(atual->nome, string)) {
            printf("%s\n", atual->caminho);
        }
        atual = atual->prox;
    }
}




void conta_recursivo(uint16_t bloco_atual, int* total_arquivos, int* total_diretorios) {
    if (bloco_atual == 0xFFFF) return;

    Metadados metadados[MAX_METADATA_PER_BLOCK];
    le_bloco(bloco_atual, &metadados);

    for (int i = 0; i < MAX_METADATA_PER_BLOCK; i++) {
        if (metadados[i].primeiro_bloco != 0 || metadados[i].tamanho != 0) {
            if (metadados[i].eh_diretorio) {
                (*total_diretorios)++;
                conta_recursivo(metadados[i].primeiro_bloco, total_arquivos, total_diretorios);
            } else {
                (*total_arquivos)++;
            }
        }
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

    conta_recursivo(primeiro_bloco_raiz, &total_arquivos, &total_diretorios);

    printf("Total de arquivos: %d\n", total_arquivos);
    printf("Total de diretórios: %d\n", total_diretorios);
    printf("Espaço usado: %d bytes\n", espaco_usado);
    printf("Espaço livre: %d bytes\n", espaco_livre);

    int espaco_desperdicado = 0;
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (bitmap.blocos_livres[i]) {
            // Verifica se há espaço desperdiçado no bloco atual
            if (fat.prox_bloco[i] != 0xFFFF) {
                espaco_desperdicado += BLOCK_SIZE - sizeof(Metadados);
            }
        }
    }
    printf("Espaço desperdiçado: %d bytes\n", espaco_desperdicado);
 }

void desmonta() {
    fclose(sistema_arquivos);
    sistema_arquivos = NULL;
    limpa_db();
    printf("O seu sistema de arquivos foi desmontado");
}



int main() {
    char comando[256];
    char param1[256], param2[256];


    printf("{ep3}: ");

    while (1) {  

        scanf("%s", comando);

        if (strcmp(comando, "monta") == 0) {
            scanf("%s", param1);
            monta(param1);
        }
        else if (strcmp(comando, "desmonta") == 0) {
            desmonta();
        }
        else if (strcmp(comando, "copia") == 0) {
            scanf("%s %s", param1, param2);
            copia(param1, param2);
        }
        else if (strcmp(comando, "criadir") == 0) {
            scanf("%s", param1);
            criadir(param1);
        }
        else if (strcmp(comando, "apagadir") == 0) {
            scanf("%s", param1);
            apagadir(param1);
        }
        else if (strcmp(comando, "mostra") == 0) {
            scanf("%s", param1);
            mostra(param1);
        }
        else if (strcmp(comando, "toca") == 0) {
            scanf("%s", param1);
            toca(param1);
        }
        else if (strcmp(comando, "apaga") == 0) {
            scanf("%s", param1);
            apaga(param1);
        }
        else if (strcmp(comando, "lista") == 0) {
            scanf("%s", param1);
            lista(param1);
        }
        else if (strcmp(comando, "atualizadb") == 0) {
            atualizadb();
        }
        else if (strcmp(comando, "busca") == 0) {
            scanf("%s", param1);
            busca(param1);
        }
        else if (strcmp(comando, "status") == 0) {
            status();
        }
        else if (strcmp(comando, "sai") == 0) {
            desmonta();
            break;
        }
        else {
            printf("Comando desconhecido\n");
        }
        printf("{ep3}: ");
    }


    return 0;
}
