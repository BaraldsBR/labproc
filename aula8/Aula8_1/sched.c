#include "sched.h"
#include "bcm.h"
#include <stdint.h>

#define MAX_TASKS 1024

/*
 * Símbolos definidos pelo linker (stacks)
 */
extern uint8_t stack_user1;
extern uint8_t stack_user2;
extern uint8_t stack_user3;

/*
 * Pontos de entrada dos tasks (em main.c)
 */
int user1_main(void);
int user2_main(void);
int user3_main(void);

volatile uint32_t ticks; // contador de ticks

/**
 * Estrutura do
 * Task control block (TCB).
 */
typedef struct tcb_s {
  uint32_t regs[17]; // Contexto (r0-r15, cpsr)
} tcb_t;

/**
 *
 * Estrutura dados lista ligada de tasks
 *
 */
typedef struct tcb_ll {
  tcb_t tcb;
  int tid;
  volatile struct tcb_ll *next;
} tcb_ll_t;

/**
 * Lista estática dos tasks definidos no sistema.
 */

volatile tcb_ll_t tcb_list[MAX_TASKS];
volatile tcb_ll_t *head = &tcb_list[0];

/*
 * Variáveis globais, acessadas em boot.s
 */
volatile int tid;
volatile tcb_t *tcb;

/*
 * Variáveis globais de controle da lista de tasks
 */
volatile int ll_size = 0;
volatile int last_tid = 0;

/**
 * Chama o kernel com swi, a função "yield" (r0 = 1).
 * Devolve o controle ao sistema executivo, que pode escalar outro thread.
 */
void __attribute__((naked)) yield(void) {
  asm volatile("push {lr}  \n\t"
               "mov r0, #1 \n\t"
               "swi #0     \n\t"
               "pop {pc}");
}

/**
 * Retorna o identificador do thread atual (tid).
 */
int __attribute__((naked)) getpid(void) {
  asm volatile("push {lr}  \n\t"
               "mov r0, #2 \n\t"
               "swi #0     \n\t"
               "pop {pc}");
}

/**
 * Retorna o valor de ticks.
 */
unsigned __attribute__((naked)) getticks(void) {
  asm volatile("push {lr}  \n\t"
               "mov r0, #3 \n\t"
               "swi #0     \n\t"
               "pop {pc}");
}

/**
 * Escalador:
 * Escolhe o próximo thread.
 */
void schedule(void) {
  head = head->next;
  tid = head->tid;
  tcb = &head->tcb;
}

/**
 * Chamado pelo serviço de interrupção swi.
 * Implementa as chamadas do sistema operacional, conforme o valor do parâmetro
 * (R0).
 */
void trata_swi(unsigned op) {
  switch (op) {
  /*
   * yield
   */
  case 1:
    /*
     * Escala o próximo task.
     */
    schedule();
    break;

  /*
   * getpid
   */
  case 2:
    /*
     * Devolve o tid no r0 do usuário.
     */
    tcb->regs[0] = tid;
    break;

  /*
   * getticks
   */
  case 3:
    /*
     * Devolve o tid no r0 do usuário.
     */
    tcb->regs[0] = ticks;
    break;
  }
}

/**
 * Chamado pelo serviço de interrupção irq.
 * Deve ser a interrupção do core timer.
 */
void trata_irq(void) {
  /*
   * Interrupção do timer a cada 200 ms.
   */
  if (bit_is_set(IRQ_REG(pending_basic), 0)) {
    TIMER_REG(ack) = 1; // reconhece a interrupção
    ticks = ticks + 1;  // atualiza contador de ticks
    schedule();         // escala outro thread
  }

  // outras interrupções aqui...
}

/**
 * Inicialização do escalador.
 */
void sched_init(void) {
  /*
   * Configura o primeiro task.
   */
  tcb_t tcb1 = {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,                      // r0-r12
      (uint32_t)&stack_user1, // sp
      0,                      // lr inicial
      (uint32_t)user1_main,   // pc = lr = ponto de entrada
      0x10 // valor do cpsr (modo usuário, interrupções habilitadas)
  };

  tcb_t tcb2 = {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,                      // r0-r12
      (uint32_t)&stack_user2, // sp
      0,                      // lr inicial
      (uint32_t)user2_main,   // pc = lr = ponto de entrada
      0x10 // valor do cpsr (modo usuário, interrupções habilitadas)
  };

  tcb_t tcb3 = {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,                      // r0-r12
      (uint32_t)&stack_user3, // sp
      0,                      // lr inicial
      (uint32_t)user3_main,   // pc = lr = ponto de entrada
      0x10 // valor do cpsr (modo usuário, interrupções habilitadas)
  };

  tcb_list[0].tcb = tcb1;
  tcb_list[0].tid = 0;
  tcb_list[0].next = &tcb_list[0];

  insert_tcb(&tcb_list[0], tcb2);
  insert_tcb(tcb_list[0].next, tcb3);

  tid = 0;
  tcb = &head->tcb;

  /*
   * Configura interrupção do timer.
   */
  TIMER_REG(load) = 200000L;       // 1MHz / 200000 = 5 Hz (200 ms)
  TIMER_REG(control) = __bit(9)    // habilita free-running counter
                       | __bit(7)  // habilita timer
                       | __bit(5)  // habilita interrupção
                       | __bit(1); // timer de 23 bits

  IRQ_REG(enable_basic) = __bit(0); // habilita interrupção básica 0 (timer)
}

/*
 * Cria novo no da lista ligada de tasks.
 */
void insert_tcb(volatile tcb_ll_t *head, tcb_t tcb) {
  last_tid++;
  ll_size++;

  if (ll_size >= MAX_TASKS)
    return;

  tcb_list[ll_size].tcb = tcb;
  tcb_list[ll_size].tid = last_tid;
  tcb_list[ll_size].next = head->next;

  head->next = &tcb_list[ll_size];

  return;
}
