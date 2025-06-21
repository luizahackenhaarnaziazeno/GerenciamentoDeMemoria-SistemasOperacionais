#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct Bloco {
    char id;            // ID do processo (ou ' ' para livre)
    int tam;            // Tamanho em KB
    int end_ini;        // Endereço inicial
    bool alocado;       // True se alocado, False se livre
    struct Bloco *prox; // Próximo bloco na lista
    struct Bloco *ant;  // Bloco anterior na lista
} Bloco;

Bloco *primeiro = NULL;      // Ponteiro para o primeiro bloco da lista
Bloco *ult_alocado = NULL;   // Ponteiro para o último bloco onde uma alocação ocorreu (Circular-Fit)

// Cria um novo bloco
Bloco* criar_bloco(char id, int tam, int end_ini, bool alocado) {
    Bloco *novo = (Bloco *)malloc(sizeof(Bloco));
    if (novo == NULL) {
        perror("Erro ao alocar memoria para o bloco");
        exit(EXIT_FAILURE);
    }
    novo->id = id;
    novo->tam = tam;
    novo->end_ini = end_ini;
    novo->alocado = alocado;
    novo->prox = NULL;
    novo->ant = NULL;
    return novo;
}

// Inicializa a memória com um único bloco grande e livre
void iniciar_memoria(int tam_total) {
    if (tam_total <= 0 || (tam_total & (tam_total - 1)) != 0) {
        printf("Erro: O tamanho da memoria deve ser uma potencia de dois e positivo.\n");
        exit(EXIT_FAILURE);
    }
    primeiro = criar_bloco(' ', tam_total, 0, false);
    primeiro->prox = primeiro; // Lista circular
    primeiro->ant = primeiro;  // Lista duplamente encadeada e circular
    ult_alocado = primeiro;    // Começa a busca do início
    printf("Mempria inicializada com %d KB.\n", tam_total);
}

// Exibe o status atual da memória
void mostrar_memoria() {
    printf("\n--- Estado da Memoria ---\n");
    if (primeiro == NULL) {
        printf("Memoria vazia.\n");
        return;
    }

    Bloco *atual = primeiro;
    int blocos_livres_contiguos = 0;
    do {
        printf("| %d KB ", atual->tam);
        if (atual->alocado) {
            printf("(Proc %c) ", atual->id);
        } else {
            printf("(Livre) ");
            blocos_livres_contiguos++;
        }
        printf("End: %d ", atual->end_ini);
        atual = atual->prox;
    } while (atual != primeiro); // Percorre toda a lista circular

    printf("|\n");
    printf("Total de Blocos Contiguos Livres: %d\n", blocos_livres_contiguos);
    printf("-------------------------\n");
}

// Aloca memória usando a política Circular-Fit
void alocar_memoria_circular(char id_proc, int tam_req) {
    printf("\nRequisicao: IN(%c, %d)\n", id_proc, tam_req);

    if (primeiro == NULL) {
        printf("Erro: Memoria nao inicializada.\n");
        return;
    }

    Bloco *busca_atual = ult_alocado;
    Bloco *bloco_encontrado = NULL;
    int nos_verificados = 0;
    int total_nos = 0;

    // Contar o número total de nós para o loop de segurança
    Bloco *temp = primeiro;
    if (temp != NULL) {
        do {
            total_nos++;
            temp = temp->prox;
        } while (temp != primeiro);
    }

    // Loop para procurar a partir do último bloco alocado, circulando
    while (nos_verificados < total_nos) {
        if (!busca_atual->alocado && busca_atual->tam >= tam_req) {
            bloco_encontrado = busca_atual;
            break; // Encontrou o primeiro bloco adequado
        }
        busca_atual = busca_atual->prox;
        nos_verificados++;
    }

    if (bloco_encontrado != NULL) {
        if (bloco_encontrado->tam == tam_req) {
            // Encaixe perfeito
            bloco_encontrado->id = id_proc;
            bloco_encontrado->alocado = true;
            printf("Alocado %d KB para o processo %c (encaixe perfeito).\n", tam_req, id_proc);
        } else {
            // Divide o bloco existente
            Bloco *novo_bloco_livre = criar_bloco(
                ' ',
                bloco_encontrado->tam - tam_req,
                bloco_encontrado->end_ini + tam_req,
                false
            );

            // Insere o novo bloco livre APÓS o bloco alocado
            novo_bloco_livre->prox = bloco_encontrado->prox;
            novo_bloco_livre->ant = bloco_encontrado;
            if (bloco_encontrado->prox != NULL) {
                bloco_encontrado->prox->ant = novo_bloco_livre;
            }
            bloco_encontrado->prox = novo_bloco_livre;

            // Atualiza o bloco original (agora alocado)
            bloco_encontrado->id = id_proc;
            bloco_encontrado->tam = tam_req;
            bloco_encontrado->alocado = true;

            printf("Alocado %d KB para o processo %c (bloco dividido).\n", tam_req, id_proc);
        }
        ult_alocado = bloco_encontrado; // Atualiza o ponteiro para o último bloco alocado
    } else {
        printf("ESPACO INSUFICIENTE DE MEMORIA para o processo %c.\n", id_proc);
    }
    mostrar_memoria(); // Exibe o status da memória após a alocação
}

// Libera memória e tenta juntar (coalescer) blocos livres adjacentes
void liberar_memoria(char id_proc) {
    printf("\nRequisicao: OUT(%c)\n", id_proc);

    if (primeiro == NULL) {
        printf("Erro: Memoria nao inicializada.\n");
        return;
    }

    Bloco *atual = primeiro;
    Bloco *bloco_liberado = NULL;

    do {
        if (atual->id == id_proc && atual->alocado) {
            atual->alocado = false; // Marca como livre
            atual->id = ' ';        // Limpa o ID do processo
            bloco_liberado = atual;
            printf("Memoria liberada para o processo %c.\n", id_proc);
            break;
        }
        atual = atual->prox;
    } while (atual != primeiro); // Percorre toda a lista circular

    if (bloco_liberado != NULL) {
        // Tenta juntar com o bloco anterior se for livre
        Bloco *bloco_anterior = bloco_liberado->ant;
        if (bloco_anterior != bloco_liberado && !bloco_anterior->alocado) {
            bloco_anterior->tam += bloco_liberado->tam; // Aumenta o tamanho do bloco anterior
            bloco_anterior->prox = bloco_liberado->prox; // Remove bloco_liberado da lista
            bloco_liberado->prox->ant = bloco_anterior;

            if (ult_alocado == bloco_liberado) {
                ult_alocado = bloco_anterior;
            }
            if (primeiro == bloco_liberado) {
                primeiro = bloco_anterior;
            }
            free(bloco_liberado); // Libera a memória do nó mesclado
            bloco_liberado = bloco_anterior; // Atualiza para o nó coalescido para a próxima verificação
        }

        // Tenta juntar com o próximo bloco se for livre
        Bloco *proximo_bloco = bloco_liberado->prox;
        if (proximo_bloco != bloco_liberado && !proximo_bloco->alocado) {
            bloco_liberado->tam += proximo_bloco->tam; // Aumenta o tamanho do bloco atual
            bloco_liberado->prox = proximo_bloco->prox; // Remove proximo_bloco da lista
            if (proximo_bloco->prox != NULL) {
                proximo_bloco->prox->ant = bloco_liberado;
            }

            if (primeiro == proximo_bloco) {
                 primeiro = bloco_liberado;
            }
            if (ult_alocado == proximo_bloco) {
                ult_alocado = bloco_liberado;
            }

            free(proximo_bloco); // Libera a memória do nó mesclado
        }
    } else {
        printf("Erro: Processo %c nao encontrado ou ja esta livre.\n", id_proc);
    }
    mostrar_memoria(); // Exibe o status da memória após a liberação
}

// Processa requisições de um arquivo
void processar_requisicoes_arquivo(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de requisicoes");
        return;
    }

    char linha[100];
    char comando[5];
    char id_processo;
    int tamanho;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        linha[strcspn(linha, "\n")] = 0; // Remove o '\n'

        // Requisição IN(ID,TAM)
        if (sscanf(linha, "IN(%c,%d)", &id_processo, &tamanho) == 2) {
            alocar_memoria_circular(id_processo, tamanho);
        }
        // Requisição OUT(ID)
        else if (sscanf(linha, "OUT(%c)", &id_processo) == 1) {
            liberar_memoria(id_processo);
        } else {
            printf("Linha invalida no arquivo: %s\n", linha);
        }
    }

    fclose(arquivo);
}

// Libera toda a memória alocada dinamicamente
void limpar_memoria() {
    if (primeiro == NULL) return;

    Bloco *atual = primeiro->prox;
    Bloco *temp;

    // Quebra a lista circular para liberar corretamente
    primeiro->ant->prox = NULL; // O último nó não aponta mais para o primeiro

    while (atual != NULL && atual != primeiro) {
        temp = atual;
        atual = atual->prox;
        free(temp);
    }
    free(primeiro); // Libera o próprio primeiro bloco
    primeiro = NULL;
    ult_alocado = NULL;
    printf("\nMemoria limpa.\n");
}

int main() {
    int tam_total_memoria;
    printf("Digite o tamanho total da memoria em KB (deve ser uma potencia de dois): ");
    scanf("%d", &tam_total_memoria);

    iniciar_memoria(tam_total_memoria);

    char nome_arq_requisicoes[100];
    printf("Digite o nome do arquivo com as requisicoes (ex: requests.txt): ");
    scanf("%s", nome_arq_requisicoes);

    printf("\n--- Simulando Alocacao de Memoria com Circular-Fit (Lista Encadeada) ---\n");
    processar_requisicoes_arquivo(nome_arq_requisicoes);

    limpar_memoria();

    return 0;
}