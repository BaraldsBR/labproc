#include "sched.h"

extern void enable_irq(int);

/*
 * Ponto de entrada do sistema.
 */
void system_main(void) {
  sched_init();
  asm volatile("b task_switch"); // transfere o controle ao primeiro thread
}

/*
 * Ponto de entrada do primeiro task.
 */
void user1_main(void) {
  int i;
  for (;;) {
    for (i = 0; i < 100; i++) {
      asm volatile("nop");
    }
    asm volatile("nop");
    sleep(4);
  }
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
