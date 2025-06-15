#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_MEM_PADRAO 256

// Estrutura de Block de memória (Buddy)
typedef struct BlockBuddy {
    int inicio;              // Início do Block na memória
    int tam;                 // Tamanho do Block
    int status;               // 1 se está status, 0 se está alocado
    char pid[10];            // ID do processo
    int tam_pedido;          // Tamanho realmente pedido pelo processo
    char lado;               // 'L', 'R' ou '-' para raiz
    struct BlockBuddy* next; // Próximo Block
} BlockBuddy;

// Cria e inicializa primeiro bloco do buddy
BlockBuddy* InitBlock(int tam_total) {
    BlockBuddy* Block = malloc(sizeof(BlockBuddy));
    Block->inicio = 0;
    Block->tam = tam_total;
    Block->lado = '-';
    Block->status = 1;
    strcpy(Block->pid, "");
    Block->tam_pedido = 0;
    Block->next = NULL;
    return Block;
}

// Divide recursivamente o Block até chegar no tamanho desejado
void SplitBlock(BlockBuddy* Block, int target) {
    while (Block->tam / 2 >= target) {
        BlockBuddy* buddy = malloc(sizeof(BlockBuddy));
        buddy->tam = Block->tam / 2;
        buddy->inicio = Block->inicio + buddy->tam;
        buddy->status = 1;
        strcpy(buddy->pid, "");
        buddy->tam_pedido = 0;
        buddy->lado = 'R';
        buddy->next = Block->next;

        Block->tam = buddy->tam;
        Block->next = buddy;
        Block->lado = 'L';

        printf("Dividindo Block de %d KB em %d L e %d R\n",
               Block->tam * 2, Block->tam, buddy->tam);
    }
}

// Aloca memória usando BUddy
int AllocBlock(BlockBuddy* head, char* pid, int tam_req) {
    int tam_alocado = 1;
    while (tam_alocado < tam_req) tam_alocado <<= 1;

    BlockBuddy* atual = head;
    while (atual) {
        if (atual->status && atual->tam > tam_alocado)
            SplitBlock(atual, tam_alocado);

        if (atual->status && atual->tam == tam_alocado) {
            atual->status = 0;
            atual->tam_pedido = tam_req;
            strcpy(atual->pid, pid);
            printf("> Alocando %s com tamanho %d para Block [%d %c]\n",
                   pid, tam_req, atual->tam, atual->lado);
            return atual->inicio;
        }
        atual = atual->next;
    }

    printf("ESPACO INSUFICIENTE DE MEMORIA\n");
    return -1;
}

// Libera memória associada a um processo e tenta juntar Blocks
void FreeBlock(BlockBuddy* head, char* pid) {
    BlockBuddy* atual = head;
    while (atual) {
        if (!atual->status && strcmp(atual->pid, pid) == 0) {
            atual->status = 1;
            atual->tam_pedido = 0;
            strcpy(atual->pid, "");
            break;
        }
        atual = atual->next;
    }

    atual = head;
    while (atual && atual->next) {
        if (atual->status && atual->next->status &&
            atual->tam == atual->next->tam &&
            atual->inicio % (2 * atual->tam) == 0) {

            atual->tam *= 2;
            BlockBuddy* temp = atual->next;
            atual->next = temp->next;
            free(temp);
            atual = head;
            continue;
        }
        atual = atual->next;
    }
}

// Calcula fragmentação interna
int InnerFrag(BlockBuddy* head) {
    int frag = 0;
    while (head) {
        if (!head->status && head->tam_pedido > 0)
            frag += head->tam - head->tam_pedido;
        head = head->next;
    }
    return frag;
}

void PrintBuddy(BlockBuddy* head) {
    printf("-----------------------------------------------------------------------\n");
    printf("Path:                       Size:      Allocated (PID):  Int. Frag.\n");
    printf("-----------------------------------------------------------------------\n");

    BlockBuddy* current = head;
    int total_allocated = 0;
    int total_frag = 0;
    int total_memory = 0;

    while (current) {
        // Path: e.g., 128L -> 64L -> 32L
        int path_start = current->inicio;
        int path_size = current->tam;
        char path[256] = "";
        char seg[32];
        int rev[10], depth = 0;

        while (path_size < TAM_MEM_PADRAO) {
            int parent_size = path_size * 2;
            int is_left = (path_start % parent_size == 0);
            rev[depth++] = is_left;
            path_size = parent_size;
            path_start = is_left ? path_start : path_start - path_size / 2;
        }

        int tmp_size = TAM_MEM_PADRAO;
        for (int i = depth - 1; i >= 0; i--) {
            tmp_size /= 2;
            snprintf(seg, sizeof(seg), "%d%s -> ", tmp_size, rev[i] ? "L" : "R");
            strcat(path, seg);
        }

        int len = strlen(path);
        if (len >= 4) path[len - 4] = '\0'; 

        char size_str[16];
        snprintf(size_str, sizeof(size_str), "%d KB", current->tam);
        total_memory += current->tam;

        char alloc_str[32] = "-";
        char frag_str[16] = "-";

        if (!current->status) {
            snprintf(alloc_str, sizeof(alloc_str), "%d KB (%s)", current->tam_pedido, current->pid);
            snprintf(frag_str, sizeof(frag_str), "%d KB", current->tam - current->tam_pedido);
            total_allocated += current->tam_pedido;
            total_frag += current->tam - current->tam_pedido;
        }

        printf("%-27s %-10s %-17s %s\n", path, size_str, alloc_str, frag_str);
        current = current->next;
    }

    printf("-----------------------------------------------------------------------\n");
    printf("Total                      %3d KB      %3d KB            %d KB\n", total_memory, total_allocated, total_frag);
    printf("-----------------------------------------------------------------------\n");
    printf("\n\n");
}


// Executa um comando lido da entrada
void execBuddy(BlockBuddy* memoria, const char* linha) {
    char pid[10];
    int tam;

    if (sscanf(linha, "IN(%[^,],%d)", pid, &tam) == 2) {
        printf("> Alocando: (IN %s, %d)\n", pid, tam);
        AllocBlock(memoria, pid, tam);
    } else if (sscanf(linha, "OUT(%[^)])", pid) == 1) {
        printf("> Liberando: (OUT%s)\n", pid);
        FreeBlock(memoria, pid);
    } else {
        //printf("> Linha ignorada: %s\n", linha);
    }
    PrintBuddy(memoria);
}

// Roda o particionamento buddy
void Runbuddy(int tam_mem) {
    BlockBuddy* memoria = InitBlock(tam_mem);
    FILE* arq = fopen("entrada.txt", "r");
    if (!arq) {
        perror("Erro ao abrir entrada.txt");
        return;
    }

    char linha[50];
    while (fgets(linha, sizeof(linha), arq)) {
        if (linha[strlen(linha) - 1] == '\n')
            linha[strlen(linha) - 1] = '\0';
        execBuddy(memoria, linha);
    }
    fclose(arq);

    printf("Fragmentacao interna total: %d KB\n", InnerFrag(memoria));
}

void Runworst_fit(int tam_mem) {
    printf("[!] Particionamento worst-fit.\n");
}

void Run_circular_fit(int tam_mem) {
    printf("[!] Particionamento circular-fit.\n");
}

// Função principal
int main() {
    int tipo_part;
    int politica = 0;
    int tam_mem;

    printf("Escolha o tipo de particionamento:\n");
    printf("1 - Particionamento Variavel\n");
    printf("2 - Particionamento Definido (Buddy)\n> ");
    scanf("%d", &tipo_part);

    if (tipo_part == 1) {
        printf("Escolha a politica:\n");
        printf("1 - Worst-Fit\n");
        printf("2 - Circular-Fit\n> ");
        scanf("%d", &politica);
    }

    printf("Informe o tamanho da memoria principal (em KB, potencia de 2):\n> ");
    scanf("%d", &tam_mem);
    if (tam_mem <= 0) tam_mem = TAM_MEM_PADRAO;

    switch (tipo_part) {
        case 1:
            if (politica == 1)
                rodar_worst_fit(tam_mem);
            else if (politica == 2)
                rodar_circular_fit(tam_mem);
            else
                printf("Politica invalida.\n");
            break;
        case 2:
            Runbuddy(tam_mem);
            break;
        default:
            printf("Tipo de particionamento invalido.\n");
    }

    return 0;
}
