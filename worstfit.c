#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define MAX_ID 10

typedef struct BlocoMemoria {
    int inicio;
    int tamanho;
    int livre;
    char id[MAX_ID];
    struct BlocoMemoria* prox;
} BlocoMemoria;

BlocoMemoria* memoria = NULL;
int TAM_MEMORIA = 0;

void inicializar_memoria(int tamanho) {
    TAM_MEMORIA = tamanho;
    memoria = (BlocoMemoria*)malloc(sizeof(BlocoMemoria));
    memoria->inicio = 0;
    memoria->tamanho = tamanho;
    memoria->livre = 1;
    strcpy(memoria->id, "");
    memoria->prox = NULL;
}

void juntar_blocos() {
    BlocoMemoria* atual = memoria;
    while (atual && atual->prox) {
        if (atual->livre && atual->prox->livre) {
            atual->tamanho += atual->prox->tamanho;
            BlocoMemoria* temp = atual->prox;
            atual->prox = temp->prox;
            free(temp);
        } else {
            atual = atual->prox;
        }
    }
}

void alocar(char* id, int tamanho) {
    BlocoMemoria *atual = memoria, *melhor = NULL;

    while (atual) {
        if (atual->livre && atual->tamanho >= tamanho) {
            if (!melhor || atual->tamanho > melhor->tamanho)
                melhor = atual;
        }
        atual = atual->prox;
    }

    if (!melhor) {
        printf("ESPAÇO INSUFICIENTE DE MEMÓRIA para %s(%d)\n", id, tamanho);
        return;
    }

    if (melhor->tamanho > tamanho) {
        BlocoMemoria* novo = (BlocoMemoria*)malloc(sizeof(BlocoMemoria));
        novo->inicio = melhor->inicio + tamanho;
        novo->tamanho = melhor->tamanho - tamanho;
        novo->livre = 1;
        strcpy(novo->id, "");
        novo->prox = melhor->prox;

        melhor->prox = novo;
        melhor->tamanho = tamanho;
    }

    melhor->livre = 0;
    strcpy(melhor->id, id);
}

void liberar(char* id) {
    BlocoMemoria* atual = memoria;
    while (atual) {
        if (!atual->livre && strcmp(atual->id, id) == 0) {
            atual->livre = 1;
            strcpy(atual->id, "");
            juntar_blocos();
            return;
        }
        atual = atual->prox;
    }
}

void mostrar_memoria() {
    BlocoMemoria* atual = memoria;
    int livres = 0;

    printf("MEMÓRIA: ");
    while (atual) {
        if (atual->livre) {
            printf("[L:%d] ", atual->tamanho);
            livres++;
        } else {
            printf("[%s:%d] ", atual->id, atual->tamanho);
        }
        atual = atual->prox;
    }
    printf("\nBlocos livres: %d\n\n", livres);
}

void processar_interativo() {
    char comando[64], id[MAX_ID];
    int tamanho;

    printf("Digite comandos (IN(id, tamanho) ou OUT(id)), ou FIM para terminar:\n");
    while (1) {
        printf("> ");
        fgets(comando, sizeof(comando), stdin);

        if (strncmp(comando, "IN(", 3) == 0 && sscanf(comando, "IN(%[^,], %d)", id, &tamanho) == 2) {
            alocar(id, tamanho);
        } else if (strncmp(comando, "OUT(", 4) == 0 && sscanf(comando, "OUT(%[^)])", id) == 1) {
            liberar(id);
        } else if (strncmp(comando, "FIM", 3) == 0) {
            break;
        } else {
            printf("Comando inválido. Use IN(id, tam), OUT(id) ou FIM.\n");
        }

        mostrar_memoria();
    }
}


int main() {

    int tamanho;
    
    setlocale(LC_ALL, "pt_BR.UTF-8");
    printf("Insira o tamanho da memória, em potência de 2: ");
    scanf("%d", &tamanho);
    while (getchar() != '\n'); 

    inicializar_memoria(tamanho);
    processar_interativo();

    return 0;
}
