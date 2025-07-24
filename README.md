# Teclado B.A.R.A.L.D.I

O projeto consiste no desenvolvimento de um sistema de entrada bare-metal na Raspberry Pi, projeto carinhosamente apelidado de **B.A.R.A.L.D.I.**, sigla muito bem pensada para **B**runo **A**pproved **R**ealtime **A**ccess to **L**ow-level **D**igital Inputs.A proposta é criar um teclado matricial (inicialmente 4x4, mas que poderá ser expandido conforme avançamos) conectado diretamente aos GPIOs da Raspberry Pi, programado em C e funcionando sem sistema operacional. A matriz de botões será escaneada pelo software, através da conhecida técnica de varredura por linhas e colunas. Inicialmente, os dados pressionados serão armazenados em memória, como se estivéssemos simulando um terminal ou buffer de entrada (eventualmente, se houver tempo, poderemos considerar o uso da UART para facilitar a visualização).

## Desenvolvido por

- Enzo Tassini das Neves
- Gabriel Baraldi Souza
- João Felipe Pereira Carvalho
