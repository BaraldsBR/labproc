#include "teclado.h"
#include "uart.h"

/*
 * Ponto de entrada do sistema.
 */
void main(void) {
  init_keyboard(true);

  read_keyboard();
}
