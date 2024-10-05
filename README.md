AUTOR:
Laís Nuto Rossman, NºUSP 12547274, laisnuto@usp.br


DESCRIÇÃO:
Este trabalho consiste na implementação de um simulador de sistema de arquivos que opera diretamente sobre um arquivo binário, simulando operações básicas de um sistema de arquivos real.
O sistema cria um arquivo binário que funiona como um "disco", além disso, o sistema utiliza as estruturas do FAT e bitmap para gerenciar o espaço e permite criar, deletar, listar e manipular arquivos e diretórios.

  
COMO EXECUTAR:
Primeiramente, para compilar `Makefile` que vai gerar o executável basta rodar no terminal o comando: make
Por fim, para executar o ep3, basta rodar o seguinte comando: ./ep3


VISÃO GERAL:
Este simulador de sistema de arquivos foi projetado para imitar as operações fundamentais de um sistema de arquivos real usando um arquivo único como um disco virtual. 
Nele, os dados são gerenciados por uma tabela FAT (File Allocation Table), que ajuda a localizar os blocos de um arquivo dentro do "disco".
Para o gerenciamento de espaço livre, o sistema utiliza um bitmap, que marca os blocos como livres ou ocupados. Isso permite simular o armazenamento e a recuperação de dados como se fosse um sistema de arquivos real.
O sistema de arquivos é capaz de suportar operações tanto em arquivos quanto em diretórios, permitindo a criação, exclusão e listagem, assim como a manipulação de metadados associados a esses objetos, como tamanho, data de criação, última modificação e último acesso.
Vamos ver a seguir as funcionalidades implementadas e como elas foram desenvolvidas.


FUNCIONALIDADES:
- Monta (arquivo): Ao executar esse comando, verificamos  se o arquivo do sistema de arquivos existe. Se não existir, cria-se um novo, inicializando o bitmap indicando todas as posições livre e a FAT com todas as posições -1.
Para criar esse sistema de arquivo, é preciso de um diretório raiz, que é criado automaticamente. O diretório raiz é o unico que sabemos que sempre vai existir, e é o ponto de partida para todas as operações no sistema de arquivos. Ele tem fixo como bloco inicial o bloco 0, e marcamos o bloco 0 ocupado no bitmap assim que criamos o sistema de arquivos.
Se o arquivo passado como parêmetro já existir, o sistema carrega as estruturas de dados (bitmap e FAT) em memória. A função carrega_fat_bitmap é utilizada para ler esses dados do arquivo. Depois de carregar as informações do sistema já existente, a árvore de diretórios é impressa
- Copia (origem destino): Essa função consiste em copiar um arquivo do sistema de arquivos real para o sistema de arquivos simulado. Para isso, é necessário abrir o arquivo de origem, lê o conteúdo e escrever no arquivo de destino.
 Para isso, o primeiro passo é criar o arquivo destino com base no caminho passado como parametro da função. Depois de encontrar um primeiro bloco livre para esse arquivo, a função vai copiando os bytes lidos na origem nos blocos do arquivo destino, procurando novos blocos livres conforme necessário. Ao longo do processo vai atualizando a FAT e o bitmap.
Por fim, depois de escrever o conteúdo do arquivo, encontramos o bloco pai do arquivo com a função encontrar_bloco_diretorio_pai e o caminho do parâmetro, e então escrevemos no diretório pai os metadados do arquivo destino
- Criadir (diretorio): Cria um novo diretório verificando a existência do diretório pai e a disponibilidade de espaço. Utiliza as funções encontrar_bloco_diretorio_pai para localizar o diretório pai e escreve_metadados para atualizar a FAT e o bitmap com os metadados do novo diretório
- Apagadir (diretorio): Remove um diretório e todo o seu conteúdo recursivamente, utilizando limpa_diretorio para limpar subdiretórios e arquivos contidos nele, e apaga_metadados_pai para remover os metadados do diretório apagado no diretório pai.
- Mostra (arquivo):  É a função que exibe o conteúdo de um arquivo, primeiro passo é localizar o diretório pai do arquivo com a função encontrar_bloco_diretorio_pai e localizar os metadados do arquivo desejado. Com os metadados, conseguimos o primeiro bloco, e então lemos o conteúdo do arquivo e imprimimos na tela.
- Toca (arquivo): Se o arquivo existir, atualiza a data de acesso do arquivo para a data atual nos metadados que etsão no diretório pai. Se não existir, criamos um novo arquivo com conteúdo vazio
- Apaga (arquivo): Essa função remove um arquivo do sistema, liberando os blocos ocupados no bitmap, atualizando a fat e zerando os blocos de conteúdo dele.
- Lista (diretorio): Essa função mostra todos os arquivos e subdiretórios dentro de um diretório especificado, incluindo metadados detalhados. Utilizando a função le_bloco para ler os dados diretamente do sistema de arquivos, é possivel obter as informações de todos os filhos do diretório passado como parâmetro.
- Atualizadb: Essa função constroi uma lista ligada que vai representar os dados do sistema de arquivos. Para isso, vamos percorrendo a árvore de diretórios e adicionando em cada nó da lista ligada o caminho do arquivo e o seu nome.
- Busca (string): Essa função busca por arquivos e diretórios que contenham a string passada como parâmetro. Para isso, utilizamos a função busca_recursiva que percorre a árvore de diretórios e achar o Nó com o nome do arquivo ou diretório contém a string passada como parâmetro, e estão basta retornar o caminho que está no Nó encomntrado.
- Status:  Fornece as estatísticas do sistema de arquivos, como quantidade de arquivos e diretórios, espaço usado e livre. As funções conta_recursivo para contar recursivamente arquivos e diretórios e o espaço ocupado por cada um.
Assim o espaço livre é caulado a partir de todos blocos do bitmap que estão marcados como livres vezes o tamanho em bytes do bloco. Já o espaço usado é calculado a partir do tamanho real ocupado pelos arquivos e diretórios que foi visto a partir dos metadados dos arquivos percorridos na contagem recursiva. Além disso, o epaço do fat e bitmao também é contabilizado (pois ele é sempre usado).
Já o espaço desperdiçado a partir a partir do espaço total do espaço total menos o espaço livre e o espaço usado. O que também pode ser medido pelos total do bitmap marcado como usado vezes o tamanho do bloco menos o tamnho de fato usado que calculamos.
- Desmonta: Salva todas as alterações feitas no sistema de arquivos no arquivo que simula o disco e fecha o arquivo. Além disso, limpa o banco de dados em memória, liberando a memória alocada para as estruturas de dados.
- Sai: Desmonta o sistema de arquivos se já não tiver sido feito, dá um break no looping de comandos do sistema de arquivos e encerra o programa.
 

OBSERVAÇÕES:
- É muito importante passar como parametro das funções necessárias o caminho completo começando da raiz "/"

TESTE VÍDEO:
Segue o link para o teste:
https://youtu.be/bHjpOj2BUaY

Observação: No final do video, sem querer encerro o sistema com ^C ao inves do desmonta, mas meu sistema, apesar de suportar o ^C naquele contexto (já que tava tudo escrito no arquivo), não foi implementado para lidar com isso, o jeito certo de encerrar com desmonta e sai como foi feito no final


TESTES ESPECÍFICOS:
Os testes foram feitos a partir de um script que caclulava a média de 30 execuções, além disso para gerar todos os comandos necessários para o teste, usei o gerador de comandos fornecido por um dos alunos no fórum de dúvidas.


RESULTADOS DOS TESTES
Aqui estão os resultados reformatados dos testes de operações de cópia e remoção no sistema de arquivos simulado, organizados por estado do sistema (vazio, 10MB ocupados, 50MB ocupados) e tipo de operação:

-Sistema de Arquivos Vazio:
  Cópia de Arquivos:
    1MB: Tempo médio para cópia = 0.002744 segundos
    10MB: Tempo médio para cópia = 0.016610 segundos
    30MB: Tempo médio para cópia = 0.084262 segundos
  Remoção de Arquivos:
    1MB: Tempo médio para remoção = 0.002525 segundos
    10MB: Tempo médio para remoção = 0.008180 segundos
    30MB: Tempo médio para remoção = 0.020087 segundos
  Remoção de Hierarquias:
    Vazia: Tempo médio = 0.002569 segundos
    Com 60 por nível: Tempo médio = 0.035058 segundos

-Sistema de Arquivos com 10MB Ocupados:
  Cópia de Arquivos:
    1MB: Tempo médio para cópia = 0.004231 segundos
    10MB: Tempo médio para cópia = 0.029292 segundos
    30MB: Tempo médio para cópia = 0.122581 segundos
  Remoção de Arquivos:
    1MB: Tempo médio para remoção = 0.002520 segundos
    10MB: Tempo médio para remoção = 0.007950 segundos
    30MB: Tempo médio para remoção = 0.019994 segundos
  Remoção de Hierarquias:
    Vazia: Tempo médio = 0.002614 segundos
    Com 60 por nível: Tempo médio = 0.034032 segundos

-Sistema de Arquivos com 50MB Ocupados:
  Cópia de Arquivos:
    1MB: Tempo médio para cópia = 0.009512 segundos
    10MB: Tempo médio para cópia = 0.082708 segundos
    30MB: Tempo médio para cópia = 0.280577 segundos
  Remoção de Arquivos:
    1MB: Tempo médio para remoção = 0.002611 segundos
    10MB: Tempo médio para remoção = 0.008159 segundos
    30MB: Tempo médio para remoção = 0.020041 segundos
  Remoção de Hierarquias:
    Vazia: Tempo médio = 0.002545 segundos
    Com 60 por nível: Tempo médio = 0.032788 segundos
    
A partir dos resultados dos testes, podemos ter algumas conslusões sobre o sistema de arquivos simulado:
- Tamanho do Arquivo: Foi possível perceber que o necessário para copiar e remover arquivos em um mesmo estado aumenta com o tamanho do arquivo copiado/removido. Isso pode ser explicado pelo fato de que o processo de escrita e limpeza são mais complexos e demorados em arquivos que ocupam mais espaço.
- Estado do Sistema: A cópia de arquivos em sistemas de arquivos demora significamente mais a medida que o estado do sistema tinha mais espaço já ocupado. O motivo disso pode ser atribuído ao processo de busca por espaço livre ser mais demorado, já que tem mais espaço ocupado.
- Remoção de Hierarquias: A remoção de diretórios com muitos subníveis e arquivos leva significativamente mais tempo do que a remoção de diretórios vazios, o que é esperado, já que a remoção de arquivos e diretórios é feita de forma recursiva, e quanto mais subníveis, mais operações são necessárias.

Dessa forma, vemos que os resultados mostram que num geral as operações foram bem rápidas, mostrando a eficiência das operações do sistema de arquivos, além disso os resultados saíram dentro do esperado, o que mostra que o sistema de arquivos simulado está funcionando corretamente.


REFERÊNCIAS:
Para fazer o trabalho, tive auxílio de vários sites e fóruns na internet, bem como de colegas de classe que deram dicas e ajudaram em muitas partes. Vou deixar aqui alguns dos principais sites que auxiliaram no ep:
- https://www.geeksforgeeks.org/fseek-in-c-with-example/
- https://stackoverflow.com/questions/24975928/extract-the-file-name-and-its-extension-in-c
- https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
    

DEPENDÊNCIAS:
  - Processador: i5-1135G7 2.40GHz x86_64
  - Versão do gcc: gcc 9.4.0
  - Sistema Operacional: Ubuntu 20.04.4 LTS 
  - Shell: bash 5.0.17 

