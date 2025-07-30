#include "sched.h"
#include "teclado.h"
#include "uart.h"

extern void enable_irq(int);

/*
 * Ponto de entrada do sistema.
 */
void system_main(void) {

  char buffer[100];
  char c = 0;
  int i = 0;

  uart_puts("Pronto para receber (digite '|' sozinho para sair):\r\n");

  while (1) {
    i = 0;

    while (1) {
      c = uart_get();

      // Trata backspace ou DEL
      if (c == '\b' || c == 127) {
        if (i > 0) {
          i--;                // remove último caractere do buffer
          uart_puts("\b \b"); // move cursor para trás, apaga e volta
        }
        continue;
      }

      // Ecoa caractere normal
      uart_send(c);

      // Se for Enter, termina a entrada
      if (c == '\r' || c == '\n') {
        break;
      }

      // Armazena no buffer se houver espaço
      if (i < sizeof(buffer) - 1) {
        buffer[i++] = c;
      }
    }

    buffer[i] = '\0'; // termina a string

    // Se a mensagem for apenas "|", encerra
    if (i == 1 && buffer[0] == '|') {
      uart_puts("\r\nEncerrando...\r\n");
      break;
    }

    // Mostra resposta
    uart_puts("\r\nResposta:\r\n");
    uart_puts(buffer);
    uart_puts("\r\n");
  }



  init_keyboard(true);
  sched_init();
  asm volatile("b task_switch"); // transfere o controle ao primeiro thread
}

/*
 * Ponto de entrada do primeiro task.
 */
void user1_main(void) {
  read_keyboard();
}

/*
 * Ponto de entrada do segundo task.
 */
void user2_main(void) {
  int i;
  for (;;) {
    for (i = 0; i < 150; i++) {
      asm volatile("nop");
    }
    asm volatile("nop");
    sleep(4);
  }
}

void user3_main(void) {
  int i;
  for (i = 0; i < 3; i++) {
	int j;
    for (j = 0; j < 1500; j++) {
      asm volatile("nop");
    }
    asm volatile("nop");
    sleep(4);
  }

  pcs_exit(); // encerra após 3 iterações
}
