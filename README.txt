AUTOR:
Laís Nuto Rossman, NºUSP 12547274, laisnuto@usp.br


DESCRIÇÃO:
Este trabalho consiste na implementação de um simulador de uma corrida de ciclistas por eliminação


COMO EXECUTAR:
Primeiramente, para compilar `Makefile` que vai gerar o executável basta rodar no terminal o comando: make

Por fim, para executar o ep2, basta rodar o seguinte comando: ./ep2 <tamanho da pista> <número de ciclistas> 

Também é possível executar o programa com uma flag de debug para printar a pista e os ciclistas a cada rodada,
se esse for o modo desejado, para executar basta rodar o seguinte comando: ./ep2 <tamanho da pista> <número de ciclistas> -debug


VISÃO GERAL:
O funcionamento geral do programa consiste criar uma pista que guarda a posição atual dos ciclistas e criar uma thread para cada ciclista simular seu movimento em paralelo. Os ciclistas verificam as posições e espaços livres na psita antes de se moverem, vem como a possibilidade de ultrapassegem.
As alterações na pista são protegidas por um mutex. Além disso, foram usadas barreiras para fazer com que esperassem todos os ciclistas se movessem uma vez para então iniciar uma nova rodada de movimento. 
A cada 6 voltas um ciclista pode quebrar com chance de 15%, se ele quebra, ele sai da pista e vai para o final do ranking. A cada 2 voltas, o primeiro ciclista que cruzar a posição 0 termina a corrida e entra no ranking.
Quando sobrar apenas 1 ciclista correndo, a corrida é finalizada e o ranking final é impresso

TESTES:
Os testes foram feitos em 3 tamanhos de pistas diferentes, com 3 quantidades de ciclistas diferentes, com o objetivo de verificar o desempenho de memória e tempo nos diferentes cenários
- Teste 1: Pista de 625 metros em 3 situações: 125, 250 e 500 ciclistas
- Teste 2: Pista de 1250 metros em 3 situações: 125, 250 e 500 ciclistas
- Teste 3: Pista de 2500 metros em 3 situações: 125, 250 e 500 ciclistas


OBSERVAÇÕES E DECISÕES DE PROJETO:
-Cada ciclista tem sua thread indiviidual e por causa do uso de pthread barrier, decidi não destruir a thread quando finaliza a corrida ou quebra, já que isso mudaria o número de threads que travam na barreira.
Decidi colocar na struct de ciclista "terminou" que indicaria quando um ciclista não está mais na corrida (seja porque quebrou ou porque já finalizou o percurso), e então a thread de um ciclista que já terminou fica rodando mas ela não participa ativamente da corrida pois o ciclista já oi removido da pista
Para poder sinalizar quando os ciclistas pudessem ter suas threads destruidas, como todas as threads ficam rodando até a corrida acabar para não atrapalahar o funcionamento da barreira. 
Cada ciclista tem um flag "pode_sair" que só é verdadeira quando a corrida termina, enquanto a corrida estiver acontecendo, quem finalizou fica presso num looping esperando os movimentos de quem está correndo, assim a corrida continua funcionando normalmente
Dessa forma depois que finaliza a corrida, todas as threads são liberadas e não ficam mais presas nas barreiras, o programa principal acaba e finaliza, encerrando a execução de todas as threads.
- Pra imprimir o relatório por volta, uso como referência de volta "global" a volta do último colocado correndo até o momento
- O ponto é marca uma volta completa é quando o ciclista passa pela posição 0 que é a largada, exceto a primeira vez que ele cruza a posição 0 (pois todos iniciam em posições antes do 0, então quando eles cruzarem o 0 vai marcar que eles começaram a corrida e então na próximas vez que cruzarem, o numero de voltas será incrementado)
- Os ciclistas se movem da esquerda para direita


REFERÊNCIAS:
Para fazer o trabalho, tive auxílio de vários sites e fóruns na internet, bem como de colegas de classe que deram dicas e ajudaram em muitas partes. Vou deixar aqui alguns dos principais sites que auxiliaram no ep:
https://gist.github.com/shelterz/4b13459668eec743f15be6c200aa91b2 (foi um exemplo de uso do pthread barrier)
https://stackoverflow.com/questions/61647896/unknown-type-name-pthread-barrier-t (para resolver um bug que estava tendo um o ptread barrier)
    

DEPENDÊNCIAS:
  - Processador: i5-1135G7 2.40GHz x86_64
  - Versão do gcc: gcc 9.4.0
  - Sistema Operacional: Ubuntu 20.04.4 LTS 
  - Shell: bash 5.0.17 
  - Bibliotecas: 
    - `lpthread` para suporte a multithreading.

