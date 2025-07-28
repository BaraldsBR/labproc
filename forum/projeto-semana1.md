
## Semana 1 – Primeiros testes com o teclado matricial no projeto B.A.R.A.L.D.I.

#### 1. Identificação do teclado

Começamos com um teclado hexadecimal físico. A primeira tarefa foi identificar quais pinos correspondem às linhas e colunas do teclado. Essa identificação foi necessária para conectar corretamente os fios aos GPIOs da Raspberry Pi. Criamos uma imagem de referência para documentar essa correspondência.

[inserir imagem aqui]

#### 2. Mapeamento dos GPIOs

Selecionamos GPIOs da Raspberry Pi para conectar às linhas e colunas do teclado, garantindo que não houvesse conflitos com as portas JTAG. O mapeamento ficou assim:

* Colunas (saídas): GPIO 19, 13, 6, 5
* Linhas (entradas): GPIO 16, 20, 12, 21

[IMAGEM DA GPIO]

---

### 3. Código testado

O código a seguir foi escrito para ler o teclado através de uma técnica de varredura por colunas. A ideia é ativar uma coluna por vez e verificar, nas linhas, se houve alguma conexão (botão pressionado). Também foi implementado um controle de *debounce* para evitar múltiplas leituras causadas por ruído mecânico:

```c
#include "bcm.h"

// Mapeamento GPIO
#define COL1_OUTPUT_GPIO 19
#define COL2_OUTPUT_GPIO 13 
#define COL3_OUTPUT_GPIO 6
#define COL4_OUTPUT_GPIO 5

#define ROW1_INPUT_GPIO 16
#define ROW2_INPUT_GPIO 20 
#define ROW3_INPUT_GPIO 12
#define ROW4_INPUT_GPIO 21

#define DEBOUNCE_CYCLES 100
#define ROWS 4
#define COLS 4
#define OUTPUT_LENGTH 1024

int read_matrix[ROWS][COLS], last_output;
int output[OUTPUT_LENGTH];

int out_pins[ROWS] = {COL1_OUTPUT_GPIO, COL2_OUTPUT_GPIO, COL3_OUTPUT_GPIO, COL4_OUTPUT_GPIO};
int in_pins[COLS] = {ROW1_INPUT_GPIO, ROW2_INPUT_GPIO, ROW3_INPUT_GPIO, ROW4_INPUT_GPIO};

// Configura um GPIO como entrada (out=false) ou saída (out=true)
void setup_io(int gpio, bool out) {
    int index = gpio / 10;
    int offset = gpio % 10;
    if (out) {
        GPIO_REG(gpfsel[index]) = (GPIO_REG(gpfsel[index]) & (~(0x07 << (3 * offset)))) | (0x01 << (3 * offset));
    } else {
        GPIO_REG(gpfsel[index]) = GPIO_REG(gpfsel[index]) & (~(0x07 << (3 * offset)));
    }
}

// Lê o valor atual de um GPIO
bool read_io(int gpio) {
    int index = gpio / 32;
    int offset = gpio % 32;
    int read_gpio = (GPIO_REG(gplev[index]) & (0x01 << offset));
    return read_gpio == (0x01 << offset);
}

// Escreve valor alto (true) ou baixo (false) em um GPIO
void write_io(int gpio, bool gpio_on) {
    int index = gpio / 32;
    int offset = gpio % 32;
    if (gpio_on) {
        GPIO_REG(gpset[index]) = 0x01 << offset;
    } else {
        GPIO_REG(gpclr[index]) = 0x01 << offset;
    }
}

// Inicializa teclado: configura os GPIOs e desativa todas as colunas
void init_keyboard() {
    for (int i = 0; i < COLS; i++)
        setup_io(out_pins[i], true);
    
    for (int i = 0; i < ROWS; i++)
        setup_io(in_pins[i], false);

    for (int i = 0; i < COLS; i++)
        write_io(out_pins[i], false);  
    
    last_output = 0;
}

// Loop principal: faz varredura e armazena teclas pressionadas
void read_keyboard() {
    while (1) {
        for (int c = 0; c < COLS; c++) {
            write_io(out_pins[c], true); // Ativa coluna atual

            for (int r = 0; r < ROWS; r++) {
                if (read_io(in_pins[r])) { // Se a linha leu alto, tecla pressionada
                    if (read_matrix[r][c] < DEBOUNCE_CYCLES) {
                        read_matrix[r][c]++;
                    } else {
                        output[last_output++] = r * ROWS + c;
                        read_matrix[r][c] = 0;
                        if (last_output >= OUTPUT_LENGTH) {
                            last_output = 0;
                        }
                    }
                } else {
                    read_matrix[r][c] = 0;
                }
            }

            write_io(out_pins[c], false); // Desativa coluna
        }
    }
}
```

---

### 4. Debug com GDB

Testamos o código na Raspberry Pi com GDB e conseguimos ver, em tempo real, os valores sendo lidos da matriz e os efeitos do debounce.

\[inserir aqui a imagem capturada do GDB mostrando valores no buffer `output[]` e o incremento dos contadores de debounce]*

Ainda é preciso fazer um ajuste fino do debounce com é possível ver acima

---

### Próximos passos

Para a próxima semana, os objetivos são:

* Implementar a comunicação UART para visualização da digitação via terminal (TeraTerm)
* Ajustar e calibrar melhor o valor de debounce 
* Criar um buffer de entrada circular mais robusto

---

Qualquer sugestão para melhorar o código ou ideias de expansão do projeto são super bem-vindas!
