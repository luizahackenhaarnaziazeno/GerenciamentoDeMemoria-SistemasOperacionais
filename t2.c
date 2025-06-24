#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

//------------ Circular-Fit -------------------
typedef struct Bloco {
    char id;            // ID do processo (ou ' ' para livre)
    int tam;            // Tamanho em KB
    int end_ini;        // Endereço inicial
    bool alocado;       // True se alocado, False se livre
    struct Bloco *prox; // Próximo bloco na lista
    struct Bloco *ant;  // Bloco anterior na lista
} Bloco;

Bloco *primeiro = NULL;     // Ponteiro para o primeiro bloco da lista
Bloco *ultimo_alocado = NULL;  // Ponteiro para o último bloco onde uma alocação ocorreu 

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
    ultimo_alocado = primeiro;    // Começa a busca do início
    printf("Memoria inicializada com %d KB.\n", tam_total);
}

void mostrar_memoria() {
    printf("\n--- Estado da Memoria ---\n");
    if (primeiro == NULL) {
        printf("Memoria vazia.\n");
        return;
    }

    Bloco *atual = primeiro;
    printf("|");
    do {
        printf(" %d KB ", atual->tam);
        if (atual->alocado) {
            printf("(Proc %c) ", atual->id);
        } else {
            printf("(Livre) ");
        }
        printf("End: %d |", atual->end_ini);
        atual = atual->prox;
    } while (atual != primeiro); // Percorre toda a lista circular

    printf("\n");
    printf("-------------------------\n");

    printf("\n--- Blocos Livres Contiguos ---\n");
    Bloco *p_atual = primeiro;
    int contador_contiguos_livres = 0; // Para contar o número de sequências contíguas de blocos livres

    do {
        if (!p_atual->alocado) {
            // Encontrou o início de um bloco livre ou continuação de uma sequência
            Bloco *inicio_sequencia = p_atual;
            int tamanho_total_sequencia = 0;
            int num_blocos_na_sequencia = 0;

            // Percorre a sequência de blocos livres
            Bloco *temp = inicio_sequencia;
            do {
                if (!temp->alocado) {
                    tamanho_total_sequencia += temp->tam;
                    num_blocos_na_sequencia++;
                    temp = temp->prox;
                } else {
                    break; // A sequência de blocos livres terminou
                }
            } while (temp != primeiro && temp != inicio_sequencia); // Evita loop infinito em caso de memória totalmente livre

            // Se a sequência de blocos livres existe
            if (num_blocos_na_sequencia > 0) {
                contador_contiguos_livres++;
                printf("Sequencia %d: %d KB livres (comeca em End: %d)\n", 
                       contador_contiguos_livres, tamanho_total_sequencia, inicio_sequencia->end_ini);
            }
            
            // Avança o ponteiro principal para o final da sequência já processada,para não repetir blocos
            p_atual = temp; 

        } else {
            p_atual = p_atual->prox;
        }
    } while (p_atual != primeiro);

    if (contador_contiguos_livres == 0) {
        printf("Nenhum bloco livre contiguo encontrado.\n");
    }
    printf("-----------------------------------\n");
}

// Aloca memória usando a política Circular-Fit
void alocar_memoria_circular(char id_proc, int tam_req) {
    printf("\nRequisicao: IN(%c, %d)\n", id_proc,tam_req);

    if (primeiro == NULL) {
        printf("Erro: Memoria nao inicializada.\n");
        return;
    }

    Bloco *busca_atual = ultimo_alocado;
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
        ultimo_alocado = bloco_encontrado; // Atualiza o ponteiro para o último bloco alocado
    } else {
        printf("ESPACO INSUFICIENTE DE MEMORIA para o processo %c.\n", id_proc);
    }
    mostrar_memoria(); 
}

// Libera memória e tenta juntar blocos livres adjacentes
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
        atual = atual = atual->prox; // Corrigido: 'atual = atual->prox;'
    } while (atual != primeiro); // Percorre toda a lista circular

    if (bloco_liberado != NULL) {
        // Tenta juntar com o bloco anterior se for livre
        Bloco *bloco_anterior = bloco_liberado->ant;
        // Verifica se o bloco anterior não é o próprio bloco_liberado (caso seja o único na lista)
        // e se o bloco anterior está livre
        if (bloco_anterior != bloco_liberado && !bloco_anterior->alocado) {
            bloco_anterior->tam += bloco_liberado->tam; // Aumenta o tamanho do bloco anterior

            // Ajusta os ponteiros da lista
            bloco_anterior->prox = bloco_liberado->prox;
            bloco_liberado->prox->ant = bloco_anterior;

            // Atualiza primeiro e ultimo_alocado se necessário
            if (primeiro == bloco_liberado) {
                primeiro = bloco_anterior;
            }
            if (ultimo_alocado == bloco_liberado) {
                ultimo_alocado = bloco_anterior;
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
            if (ultimo_alocado == proximo_bloco) {
                ultimo_alocado = bloco_liberado;
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

    while (atual != NULL) { // O loop deve parar quando 'atual' for NULL, ou quando voltar ao 'primeiro' se a quebra falhar.
        temp = atual;
        atual = atual->prox;
        free(temp);
        if (temp == primeiro) { // Se por algum motivo o primeiro não foi liberado antes (caso de um único bloco)
            break;
        }
    }
    free(primeiro); // Libera o próprio primeiro bloco
    primeiro = NULL;
    ultimo_alocado = NULL;
    printf("\nMemoria limpa.\n");
}

void Runcircularfit(int tam_total_memoria) {
    iniciar_memoria(tam_total_memoria);

    char nome_arq_requisicoes[100];
    printf("Digite o nome do arquivo com as requisicoes (ex: requests.txt): ");
    scanf("%s", nome_arq_requisicoes);

    processar_requisicoes_arquivo(nome_arq_requisicoes);

    limpar_memoria();
}

// ----- Worst-Fit -------------------
void alocar_memoria_worst_fit(char id_proc, int tam_req) {
    printf("\nRequisicao: IN(%c, %d) - Worst-Fit\n", id_proc, tam_req);

    if (primeiro == NULL) {
        printf("Erro: Memoria nao inicializada.\n");
        return;
    }

    Bloco *atual = primeiro;
    Bloco *bloco_worst_fit = NULL; // Este será o bloco final escolhido
    int maior_tam_livre = -1;     // Para rastrear o tamanho do maior bloco livre elegível

    // Passo 1: Percorrer todos os blocos para encontrar o maior bloco livre que se encaixa
    do {
        // Verifica se o bloco atual está livre e se o tamanho dele é suficiente para a requisição
        if (!atual->alocado && atual->tam >= tam_req) {
            // Se o tamanho do bloco atual é maior do que o maior tamanho livre encontrado até agora
            if (atual->tam > maior_tam_livre) {
                maior_tam_livre = atual->tam; // Atualiza o maior tamanho livre
                bloco_worst_fit = atual;      // Este bloco é o novo candidato a pior ajuste
            }
        }
        atual = atual->prox;
    } while (atual != primeiro); // Continua até ter percorrido a lista inteira

    // Passo 2: Tentar alocar no bloco encontrado (se houver)
    if (bloco_worst_fit != NULL) {
        if (bloco_worst_fit->tam == tam_req) {
            // Encaixe perfeito no maior bloco livre (não há fragmentação interna)
            bloco_worst_fit->id = id_proc;
            bloco_worst_fit->alocado = true;
            printf("Alocado %d KB para o processo %c (encaixe perfeito no maior bloco).\n", tam_req, id_proc);
        } else {
            // Divide o maior bloco existente
            Bloco *novo_bloco_livre = criar_bloco(
                ' ',
                bloco_worst_fit->tam - tam_req,
                bloco_worst_fit->end_ini + tam_req,
                false
            );

            // Insere o novo bloco livre APÓS o bloco alocado
            novo_bloco_livre->prox = bloco_worst_fit->prox;
            novo_bloco_livre->ant = bloco_worst_fit;
            if (bloco_worst_fit->prox != NULL) { // Garante que não é o único bloco na lista circular
                bloco_worst_fit->prox->ant = novo_bloco_livre;
            }
            bloco_worst_fit->prox = novo_bloco_livre;

            // Atualiza o bloco original (agora alocado)
            bloco_worst_fit->id = id_proc;
            bloco_worst_fit->tam = tam_req;
            bloco_worst_fit->alocado = true;

            printf("Alocado %d KB para o processo %c (maior bloco dividido, %d KB restantes).\n", tam_req, id_proc, novo_bloco_livre->tam);
        }
    } else {
        printf("ESPACO INSUFICIENTE DE MEMORIA para o processo %c.\n", id_proc);
    }
    mostrar_memoria();
}

void processar_requisicoes_arquivo_worst_fit(const char *nome_arquivo) {
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
        linha[strcspn(linha, "\n")] = 0; 

        if (sscanf(linha, "IN(%c,%d)", &id_processo, &tamanho) == 2) {
            alocar_memoria_worst_fit(id_processo, tamanho); 
        }
        else if (sscanf(linha, "OUT(%c)", &id_processo) == 1) {
            liberar_memoria(id_processo); 
        } else {
            printf("Linha invalida no arquivo: %s\n", linha);
        }
    }

    fclose(arquivo);
}

void Runworst_fit(int tam_mem){
    iniciar_memoria(tam_mem);

    char nome_arq_requisicoes[100];
    printf("Digite o nome do arquivo com as requisicoes para Worst-Fit (ex: requests.txt): ");
    scanf("%s", nome_arq_requisicoes);

    processar_requisicoes_arquivo_worst_fit(nome_arq_requisicoes);

    limpar_memoria();
}
// ------------- BUDDY -------------------
#define TAM_MEM_PADRAO 256

// Estrutura de Block de memória (Buddy)
typedef struct BlockBuddy {
    int inicio;          // Início do Block na memória
    int tam;             // Tamanho do Block
    int status;          // 1 se está livre, 0 se está alocado
    char pid[10];        // ID do processo
    int tam_pedido;      // Tamanho realmente pedido pelo processo
    char lado;           // 'L', 'R' ou '-' para raiz
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
        BlockBuddy* buddy = malloc(sizeof(BlockBuddy)); // Cria um novo bloco de metade do tamanho 
        buddy->tam = Block->tam / 2;                    
        buddy->inicio = Block->inicio + buddy->tam;     
        buddy->status = 1;
        strcpy(buddy->pid, "");
        buddy->tam_pedido = 0;
        buddy->lado = 'R';
        buddy->next = Block->next;

        Block->tam = buddy->tam;
        Block->next = buddy;
        Block->lado = 'L';         // Prioriza o bloco esquerdo a cada split

        printf("Dividindo Block de %d KB em %d L e %d R\n",
                       Block->tam * 2, Block->tam, buddy->tam);
    }
}

// Aloca memória usando Buddy
int AllocBlock(BlockBuddy* head, char* pid, int tam_req) {
    int tam_alocado = 1;
    while (tam_alocado < tam_req) tam_alocado <<= 1; // Arredonda para a potência de 2 mais próxima (ex: Req 30kb, tam 32kb)

    BlockBuddy* atual = head;
    while (atual) {
        if (atual->status && atual->tam > tam_alocado) // Divide bloco se o tamanho ainda não é o mínimo para o processo 
            SplitBlock(atual, tam_alocado);

        if (atual->status && atual->tam == tam_alocado) {  // Aloca se bloco livre com o menor tamanho
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
void FreeBlock(BlockBuddy *head, char *pid)
{
    BlockBuddy *atual = head;
    while (atual)   // Libera memória do processo 
    {
        if (!atual->status && strcmp(atual->pid, pid) == 0)
        {
            atual->status = 1;        
            atual->tam_pedido = 0;
            strcpy(atual->pid, "");
            break;
        }
        atual = atual->next;
    }

    atual = head;
    while (atual && atual->next) // Junta blocos adjacentes se ambos livres
    {
        if (atual->status && atual->next->status &&
            atual->tam == atual->next->tam &&
            atual->inicio % (2 * atual->tam) == 0)
        {

            atual->tam *= 2;
            BlockBuddy *temp = atual->next;
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
            frag += head->tam - head->tam_pedido; // Diferença entre tamanho alocado e o da requisição
        head = head->next;
    }
    return frag;
}

void PrintBuddy(BlockBuddy* head) {
    printf("-----------------------------------------------------------------------\n");
    printf("Path:                        Size:      Allocated (PID): Int. Frag.\n");
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
        int rev[10], depth = 0;         // Profundidade de árvore e Caminho inverso do bloco

         while (path_size < TAM_MEM_PADRAO)
        {
            int parent_size = path_size * 2;
            int is_left = (path_start % parent_size == 0);
            rev[depth++] = is_left;
            path_size = parent_size;
            if (!is_left) {
                path_start = path_start - path_size / 2;
            }
        }

        // Mostra caminho do bloco raiz -> bloco alocado
       int tmp_size = TAM_MEM_PADRAO;
        for (int i = depth - 1; i >= 0; i--) {
            tmp_size /= 2;
            snprintf(seg, sizeof(seg), "%d%s -> ", tmp_size, rev[i] ? "L" : "R");
            strcat(path, seg);
        }

        int len = strlen(path);
        if (len >= 4) path[len - 4] = '\0'; // Remove o " -> " final

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
    // O total_memory aqui é a soma dos tamanhos dos blocos ATUAIS na lista, não o tamanho inicial da memória.
    printf("Total                       %3d KB      %3d KB           %d KB\n", total_memory, total_allocated, total_frag);
    printf("-----------------------------------------------------------------------\n");
    printf("\n\n");
}


// Executa um comando lido da entrada
void execBuddy(BlockBuddy* memoria, const char* linha) {
    char pid[10];
    int tam;

    if (sscanf(linha, "IN(%9[^,],%d)", pid, &tam) == 2) { // Use %9[^,] para evitar buffer overflow
        printf("> Requisicao: IN(%s, %d)\n", pid, tam);
        AllocBlock(memoria, pid, tam);
    } else if (sscanf(linha, "OUT(%9[^)])", pid) == 1) { // Use %9[^)] para evitar buffer overflow
        printf("> Requisicao: OUT(%s)\n", pid);
        FreeBlock(memoria, pid);
    } else {
        printf("> Linha invalida: %s\n", linha);
    }
    PrintBuddy(memoria);
}

// Roda o particionamento buddy
void Runbuddy(int tam_mem) {
    BlockBuddy* memoria = InitBlock(tam_mem);
    FILE* arq = fopen("entrada.txt", "r"); // Nome do arquivo fixo
    if (!arq) {
        perror("Erro ao abrir entrada.txt");
        return;
    }

    char linha[50];
    while (fgets(linha, sizeof(linha), arq)) {
        if (linha[strlen(linha) - 1] == '\n')
            linha[strlen(linha) - 1] = '\0'; // Remove o '\n'
        execBuddy(memoria, linha);
    }
    fclose(arq);

    printf("Fragmentacao interna total: %d KB\n", InnerFrag(memoria));

}

// Função principal
int main() {
    int tipo_part;
    int politica = 0;
    int tam_mem;
    int menu = 1;

    while(menu == 1){
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

    // Validação: Tamanho deve ser potência de 2
    if (tipo_part == 2 && (tam_mem <= 0 || (tam_mem & (tam_mem - 1)) != 0)) {
        printf("Erro: Para o Buddy System, o tamanho da memoria deve ser uma potencia de dois e positivo.\n");
    } else if (tipo_part == 1 && (tam_mem <= 0 || (tam_mem & (tam_mem - 1)) != 0)){
        printf("Erro: O tamanho da memoria deve ser uma potencia de dois e positivo.\n");
    } else {
        menu = 0; 
        break;
    }

    switch (tipo_part) {
        case 1:
            if (politica == 1) { 
                printf("[!] Particionamento worst-fit.\n");
                Runworst_fit(tam_mem);
            } else if (politica == 2) {
                printf("[!] Particionamento circular-fit.\n");
                Runcircularfit(tam_mem);
            } else { 
                printf("Politica invalida.\n");
            }
            break;
        case 2:
            printf("[!] Particionamento Buddy System.\n"); 
            Runbuddy(tam_mem);
            break;
        default:
            printf("Tipo de particionamento invalido.\n");
    }

    return 0;
}
