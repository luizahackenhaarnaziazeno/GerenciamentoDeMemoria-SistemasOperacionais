<div align="center">

# 💻 Gerenciamento de Memória - Sistemas Operacionais

Este repositório contém o código-fonte em C para o Trabalho Prático 2 (TP2) da disciplina de Sistemas Operacionais. O projeto foca na exploração e implementação de abordagens de alocação de espaço contíguo para processos, utilizando o Particionamento Variável (com políticas Worst-Fit e Circular-Fit) e o Sistema Buddy.

## 🎯 Objetivo do Projeto

O objetivo principal deste trabalho é desenvolver uma ferramenta que simule e visualize o gerenciamento de memória contígua, permitindo a exploração das seguintes estratégias de alocação:

* **Particionamento Variável**: Permite ao usuário escolher a política de alocação `Worst-Fit` ou `Circular-Fit` em tempo de execução.
* **Sistema Buddy**: Implementa a alocação de memória com base no sistema Buddy, onde a memória é alocada em unidades dimensionadas como potências de 2, e requisições são arredondadas para a próxima potência de 2 mais alta.

## 🧠 Conceitos de Gerenciamento de Memória

### Tamanho da Memória Principal

* O tamanho total inicial da memória principal a ser empregada para alocação é definido pelo usuário e **deve ser uma potência de dois** e positivo.

### Particionamento Variável

Esta abordagem lida com blocos de memória de tamanhos diversos que podem ser alocados para processos. Quando um processo libera memória, o bloco liberado tenta coalescer com blocos livres adjacentes para formar um bloco maior.

* **Política Circular-Fit**:
    * A busca por um bloco livre adequado para uma requisição de alocação começa a partir do ponto onde a última alocação ocorreu (`ultimo_alocado`).
    * Se um bloco livre encontrado for maior que a requisição, ele é dividido em um bloco alocado e um novo bloco livre.
* **Política Worst-Fit**:
    * A alocação é realizada no maior bloco livre disponível que pode satisfazer a requisição.
    * Similar ao Circular-Fit, se o maior bloco livre for maior que o necessário, ele é dividido.

### Sistema Buddy

O sistema Buddy aloca memória a partir de um segmento de tamanho fixo que consiste em páginas fisicamente contíguas. A memória é alocada nesse segmento usando um alocador que atende às solicitações em unidades dimensionadas como potência de 2 (4 KB, 8 KB, 16 KB e assim por diante).

* **Arredondamento de Requisições**: Uma solicitação em unidades não dimensionadas adequadamente é arredondada para a próxima potência mais alta de 2. Por exemplo, uma solicitação de 11 KB é satisfeita com um segmento de 16 KB.
* **Divisão de Blocos**: O sistema divide recursivamente os blocos pela metade (buddies) até encontrar um bloco livre do tamanho apropriado (arredondado para potência de 2) para a requisição.
* **Coalescência**: Uma vantagem do sistema buddy é a rapidez com que buddies adjacentes podem ser combinados para formar segmentos maiores usando uma técnica conhecida como coalescência. Quando um bloco é liberado, o sistema tenta combinar o bloco liberado com seu "buddy" adjacente se este também estiver livre, formando um bloco maior. Este processo pode continuar recursivamente, potencialmente restaurando o segmento original. Por exemplo, quando o kernel libera a unidade $C_L$ que foi alocada, o sistema pode unir $C_L$ e $C_R$ um segmento de 64 KB. Este segmento, $B_L$, pode, por sua vez, ser unido ao seu companheiro $B_R$ para formar um segmento de 128 KB.
* **Fragmentação Interna**: A diferença entre o tamanho do bloco alocado (potência de 2) e o tamanho realmente solicitado pelo processo é calculada e exibida como fragmentação interna total.

### Tratamento de Espaço Insuficiente

* Alocações que excedam a quantidade de espaço disponível na memória principal são notificadas com a mensagem "ESPAÇO INSUFICIENTE DE MEMÓRIA".

## ⚙️ Requisições de Alocação e Liberação

As sequências de requisições de alocação e liberação de memória são lidas de um arquivo chamado `entrada.txt`.

* **Alocação**: Representada pelo comando `IN(ID,TAM)`, onde `ID` é o nome do processo e `TAM` é o tamanho requisitado.
    * Ex: `IN(A,10)` - Requisita a alocação de 10 espaços para o processo A.
* **Liberação**: Representada pelo comando `OUT(ID)`, onde `ID` é a identificação do processo a ser liberado.
    * Ex: `OUT(A)` - Libera o espaço alocado pelo processo A.

## 📊 Visualização de Resultados

A ferramenta permite a visualização dos espaços livres para alocação a cada passo da execução.

* **Particionamento Variável**: Exibe o estado atual da memória (blocos alocados com ID e tamanho, e blocos livres com endereço inicial). Também lista o total de blocos contíguos livres para alocação a cada linha do arquivo lido.
* **Sistema Buddy**:
    * Mostra o "Path" (caminho de divisão do bloco, ex: `128L -> 64L -> 32L`), o tamanho do bloco, o status de alocação (com PID e tamanho pedido), e a fragmentação interna para cada bloco.
    * O total de fragmentação interna é apresentado ao final da execução.

## 🚀 Compilação e Execução

Para instruções detalhadas sobre como compilar e executar o programa, por favor, consulte o **manual do usuário** (`manual_usuario.pdf`) incluído neste repositório (ou conforme especificado nas entregas do projeto).


### 👥 Autores:
| [<img loading="lazy" src="https://avatars.githubusercontent.com/u/125413722?v=4" width="115"><br><sub>Eduardo Carlesso Silveira</sub>](https://github.com/EduardoCarlesso) |  [<img loading="lazy" src="https://avatars.githubusercontent.com/u/143823107?v=4" width="115"><br><sub>Giovanna Plácido da Cunha e Borba</sub>](https://github.com/GiovannaBorba) | [<img loading="lazy" src="https://avatars.githubusercontent.com/u/142232479?v=4" width="115"><br><sub>Luiza Hackenhaar Naziazeno</sub>](https://github.com/luizahackenhaarnaziazeno) | [<img loading="lazy" src="https://avatars.githubusercontent.com/u/89554510?v=4" width="115"><br><sub>Sophia Mendes Da Silveira</sub>]([https://github.com/RobertoG400](https://github.com/SophiaSilveira)) |
| :-------------------------------: | :-------------------------------: | :-------------------------------: | :-------------------------------: |
