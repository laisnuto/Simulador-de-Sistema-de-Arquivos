AUTOR:
Laís Nuto Rossman, NºUSP 12547274, laisnuto@usp.br


DESCRIÇÃO:
Este trabalho consiste na implementação de um um shell para interação entre usuário e sistema operacional e um simulador de processos com diferentes tipos de algotritmos de escalonamneto.
O principal objetivo é observar o comportamento desses algoritmos em diferentes cenários e analisar sua eficiência alocação de tempo da CPU.


COMO EXECUTAR:
Primeiramente, para compilar `Makefile` que vai gerar os dois executáveis (um do newsh e outro do ep1) basta rodar no terminal o comando: make
Depois para executar o programa, você pode executar o newsh usando o seguinte comando: ./newsh
Por fim, para compilar o ep1, basta rodar o seguinte comando: ./ep1 <escalonador> <nome arquivo de entrada> <nome arquivo de saída>


VISÃO GERAL:
O funcionamento geral do programa consiste em ler os argumentos de entrada e poder criar a estrutura de dados de fila que é mais apropriada de acordo com o escalonador.
Depois disso, uma thread é criada para, paralelamente ao programa principal, poder adicionar os processos na fila no instante correto
Enquanto os processos são adicionados na fila, os algoritmos de escalonamento começam a funcionar e selecionar processos para serem executados.
Cada algoritmo remove o processo da fila e cria uma thread para executar esse processo. A função de executar processo depende do algoritmo escalonador. Essa função faz durante o tempo de execução operações matemáticas para poder simular o uso da CPU
No Shortest Job Fist, a função apenas executa o processo durante o intervalo de tempo determinado (dt).
No Round Robin, a função executa o processo durante 1 segundo (quentum) e verifica se o processo foi concluido, se sim, finaliza, se não, insere na fila novamente
No Escalonador com prioridade, a função atualiza o valor do quantum do processo de acordo com a dealine e executa o valor do quantum (ou executa o tampo de tempo que resta para finalizar se for menor que o quantum) e verifica se o processo foi concluido, se sim, finaliza, se não, insere na fila novamente
Esse processo de remover da fila, executar e inserir de novo (no caso dos dois últimos), fica rodando até que as filas estejam vazias e todos os processos tenham sido executados.
Por fim, imprime as informações no arquivo de saída que o nome na linha de comando
E então espera as threads acabarem, libera memória e acaba o programa.

TESTES:
Os testes foram feitos de forma a tentar destacar características dos diferentes algoritmos escalonadores. Os arquivos do teste estão nesse mesmo diretório do ep (teste1, teste2 e teste3).
- Teste 1: Esse teste tinham 10 processos em que os mais urgentes tinham menor dt. 
- Teste 2: Esse teste tinha 15 processos mais uniformes com exceção do primeiro que tinha um maior dt
- Teste 3: Esse teste tinha 20 processos mais aleatorizados com exceção do primeiro que tinha um maior dt 


OBSERVAÇÕES E DECISÕES DE PROJETO:
-O cálculo dos tempos foi feito utilizando time(NULL), pois retorna numeros inteiros. Cheguei a tentar uma abordagem usando double para poder variar mais o quantum, porém gerava processos que ultrapassavam a deadline por questões de milisegundos, portanto optei por deixar tudo inteiro para evitar esses casos que ocorrem com a precisão do tempo
-As mudanças de contexto foram calculadas a partir da quantidade de vezes que os processos mudavam, contabilizei esse evento a partir da remoção da fila, pois isso indica que um novo processo será executado, portanto foi mudado
-As diferentes estruturas de dados (fila de prioridade e fila circular) foram implementadas dependendo do tipo de escalonador para se adequar melhor as demandas de cada algoritmos
-O cálculo do quantum tinha que levar em consideração a deadline do processo, por tanto fiz uma função que retornava um quantum de 1 a 10 que era proporcial a urgência do processo (quanto mais próximo da deadline, mais urgente mais quantum). Esse caculo foi feito a partir de um fator urgência entre 0 e 1 (representado por (deadline - tempo atual)/(deadline - t0)). Assim (quantun minimo) + ((quantun minimo- quantum maximo)*(1-fator urgencia)), temos uma função linear que retorna entre o range desejado o quantum de acordo com a proximidade da deadline
-Como o quantum mínimo do escalonador com prioridade é igual a 1 segundo, que é o quantum fixo do round robin, foi visto em todos os testes uma mudança de contexto do round robin maior ou igual do que a do escalonador com prioridade, além do cumprimento de deadline do round robin ter sido em todas menor ou igual a do escalonador com prioridade. Isso pode ser explicado por essa escolha de quantuns que, no geral, faz com que o escalonador com prioridade performe migual ou melhor que o round robin


REFERÊNCIAS:
Para fazer o trabalho, tive auxílio de vários sites e fóruns na internet, bem como de colegas de classe que deram dicas e ajudaram em muitas partes. Vou deixar aqui alguns dos principais sites que auxiliaram no ep:
https://www.ime.usp.br/~pf/analise_de_algoritmos/aulas/heap.html
https://www.geeksforgeeks.org/round-robin-scheduling-with-different-arrival-times/
https://man7.org/linux/man-pages/man3/pthread_setaffinity_np.3.html
    

DEPENDÊNCIAS:
  - Processador: i5-1135G7 2.40GHz x86_64
  - Versão do gcc: gcc 9.4.0
  - Sistema Operacional: Ubuntu 20.04.4 LTS 
  - Shell: bash 5.0.17 
  - Bibliotecas: 
    - `lpthread` para suporte a multithreading.

