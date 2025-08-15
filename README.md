# Teclado B.A.R.A.L.D.I

O projeto consiste no desenvolvimento de um sistema de entrada bare-metal na Raspberry Pi, projeto carinhosamente apelidado de **B.A.R.A.L.D.I.**, sigla muito bem pensada para **B**runo **A**pproved **R**ealtime **A**ccess to **L**ow-level **D**igital **I**nputs. A proposta é criar um teclado matricial (inicialmente 4x4, mas que poderia ser expandido com poucas mudanças no código) conectado diretamente aos GPIOs da Raspberry Pi, programado em C e funcionando sem sistema operacional. A matriz de botões será escaneada pelo software, através da conhecida técnica de varredura por linhas e colunas. Os dados pressionados são armazenados em memória, como se estivéssemos simulando um terminal ou buffer de entrada, e são escritos na saída UART com baudrate de 9600.

## Como utilizar

1. Para utilizar o código do projeto, primeiro defina as GPIOs que devem ser utilizda como entrada e saída da Raspberry Pi, e susbstitua os valores desejados nas constantes `COL<n>_OUTPUT_GPIO` e `ROW<n>_INPUT_GPIO`.
2. Realize as conexões das GPIOs selecionadas com as linhas e colunas do teclado.
3. Então compile o projeto utilizando o comando `make` e carregue o código na placa (utilizando uma conexão JTAG ou utilizando o kernel.img gerado na compilação como kernel no cartão SD).

A partir daqui a raspberry já deve funcionar como um teclado, salvando os valores digitados na string `output` que pode ser acessado pelo gdb. Para ler o resultado em tempo real, conecte a UART da placa ao computador e leia a entrada com baudrate de 9600.

## Funcionamento e técnicas do projeto

### Varredura por linhas e colunas

Para implementar a leitura do teclado matricial, utilizamos a técnica de varredura por linhas e colunas, na qual uma linha da matriz é energizada e todas as colunas são lidas, então a linha atual é desligada e a próxima é ativada, repetindo o mesmo processo indefinidamente. Em cada medição, uma posição do teclado (determinada pela linha de entrada e posição de saída) é reconhecida pelo sistema, dessa forma um ciclo de leitura obtém o estado de todas as teclas no formato `teclado[linha][coluna]`.

### Debounce e tecla pressionada

Para compatibilizar a velocidade do sistema com o uso humano, definimos que a leitura de uma tecla pressionada não seria imediatamente traduzida como uma escrita. Ao invés disso, consideramos que uma escrita seria realizada somente depois que `FIRST_DEBOUNCE_CYCLES` ciclos de leituras tivessem sido realizados com a tecla pressionada, depois o teclado deveria esperar `SECOND_DEBOUNCE_DELAY` ciclos para realizar a próxima escrita daquela tecla, que então seria escrita uma vez a cada `REMAINING_DEBOUNCE_CYCLES` ciclos. Em qualquer momento, se a tecla é lida como não pressionada, essa retorna ao início do algorítimo.

### Transmissão UART

Para entregar uma resposta em tempo real ao usuário, o sistema, além de armazenar a string escrita, transmite cada caractere registrado pela UART.

## Desenvolvido por

- Enzo Tassini das Neves
- Gabriel Baraldi Souza
- João Felipe Pereira Carvalho
