#include "sched.h"
#include "bcm.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"

#define MAX_TASKS 1024

/*
 * Símbolos definidos pelo linker (stacks)
 */
extern uint8_t stack_user1;
extern uint8_t stack_user2;
extern uint8_t stack_user3;
extern uint8_t panic;

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
 * Estados possíveis de um processo
 */
typedef enum {
  TASK_READY,
  TASK_RUNNING,
  TASK_BLOCKED,
  TASK_TERMINATED
} task_state_t;

/**
 * Possíveis razões de um block
 */
typedef enum {
	NOT_BLOCKED,
  SLEEPING,
  LOCKED
} block_type_t;

typedef struct block_info_s {
	block_type_t type;
	uint32_t info; // informação relevante para o block
	/* *
	 * sleep: tick na qual o processo pode voltar a executar
	 * lock: posição da tranca
	 */
} block_info_t;

/**
 *
 * Estrutura dados lista ligada de tasks
 *
 */
typedef struct tcb_ll {
  tcb_t tcb;
  int tid;
  volatile struct tcb_ll *next;
  uint32_t timeslice;    // numero de ticks para a task
  bool preemptible;      // se 1, pode ser preemptado no irq do timer
  task_state_t state;    // Estado da tarefa
  block_info_t block_info; // Informações sobre o bloqueio da tarefa
} tcb_ll_t;

/**
 * Lista estática dos tasks definidos no sistema.
 */

volatile tcb_ll_t tcb_list[MAX_TASKS];
volatile tcb_ll_t *head = &tcb_list[0];
volatile tcb_ll_t idle_task;

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
 * Termina o processo atual.
 * Chama o kernel com swi, a função "exit" (r0 = 4).
 */
void __attribute__((naked)) pcs_exit(void) {
  asm volatile("push {lr}  \n\t"
               "mov r0, #4 \n\t"
               "swi #0     \n\t"
               "pop {pc}");
}


/**
 * Adormece a thread.
 */
void __attribute__((naked)) sleep(unsigned ticks_to_sleep) {
   asm volatile("push {lr}  \n\t"
                "mov r1, r0 \n\t"
                "mov r0, #5 \n\t"
                "swi #0     \n\t"
                "pop {pc}");
}


/**
 * Inicia uma tranca.
 */
void __attribute__((naked)) start_lock(unsigned position) {
   asm volatile("push {lr}  \n\t"
                "mov r1, r0 \n\t"
                "mov r0, #6 \n\t"
                "swi #0     \n\t"
                "pop {pc}");

/**
 * Tranca um recurso.
 */
void __attribute__((naked)) lock(unsigned position) {
   asm volatile("push {lr}  \n\t"
                "mov r1, r0 \n\t"
                "mov r0, #7 \n\t"
                "swi #0     \n\t"
                "pop {pc}");
}

/**
 * Destranca um recurso.
 */
void __attribute__((naked)) unlock(unsigned position) {
   asm volatile("push {lr}  \n\t"
                "mov r1, r0 \n\t"
                "mov r0, #8 \n\t"
                "swi #0     \n\t"
                "pop {pc}");
}

void check_task_block(volatile tcb_ll_t *task) {
  switch (task->block_info.type) {
    case(SLEEPING):
      if (ticks >= task->block_info.info){
        task->state = TASK_READY;
        block_info_t noblock = {NOT_BLOCKED, 0};
        task->block_info = noblock;
      }
      return;
    case(LOCKED):
      if (*task->block_info.info == 0){
        task->state = TASK_READY;
        block_info_t noblock = {NOT_BLOCKED, 0};
        task->block_info = noblock;
      }
      return;
    default:
      return;
  }
}

volatile tcb_ll_t* find_next_head(){
  if (tid == -1) {
	  head = head->next;  
  }
  
  volatile tcb_ll_t *i = head->next;

  while (i != head) {
    if (i->state == TASK_BLOCKED) {
      check_task_block(i);
    }
    if (i->state == TASK_READY) {
      return i;
    }
    i = i->next;
  }

  if (i->state == TASK_READY) {
    return i; // Retorna o primeiro processo que não foi finalizado e não esta dormindo
  }

  idle_task.next = head;
  
  return &idle_task; // Se todos os processos foram finalizados ou estão dormindo, retorna NULL
}

void terminate_current_task(void) {
  head->state = TASK_TERMINATED; // marca o processo atual como finalizado
}

/**
 * Escalador:
 * Escolhe o próximo thread.
 */
void schedule(void) {
  // Muda estado do processo executando
  if (head->state == TASK_RUNNING) {
    head->state = TASK_READY;
  }

  // Atualiza o ponteiro global para o novo "head"
  head = find_next_head();

  tid = head->tid;
  tcb = &head->tcb;
  head->state = TASK_RUNNING;

  if (head->timeslice > 0) {
    TIMER_REG(load) = head->timeslice;
  }
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

  /*
   * exit
   */
  case 4:
    terminate_current_task();
    schedule(); // Passa o controle a outro processo
    break;
  /*
   * sleep
   */
  case 5:
    block_info_t block_info = {SLEEPING, ticks + tcb->regs[1]};
    head->block_info = block_info;
    head->state = TASK_BLOCKED;
    schedule();
    break;
  /*
   * start lock
   */
  case 6:
    *tcb->regs[1] = 0;
  /*
   * lock
   */
  case 7:
    if (*tcb->regs[1] != 0) {
      block_info_t block_info = {LOCKED, tcb->regs[1]};
      head->block_info = block_info;
      head->state = TASK_BLOCKED;
      schedule();
    } else {
      *tcb->regs[1] = 1;
    }
    break;
  /*
   * unlock
   */
  case 8:
    *tcb->regs[1] = 0;
    break;
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

    if (head->preemptible) {
      schedule();       // troca de contexto, caso preemptivo.
    }
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

  tcb_t tcb_idle = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (uint32_t)&panic, 0x10 };


  tcb_list[0].tcb = tcb1;
  tcb_list[0].tid = 0;
  tcb_list[0].next = &tcb_list[0];
  tcb_list[0].state = TASK_READY;
  tcb_list[0].timeslice   = 200000;  // 200 ms
  tcb_list[0].preemptible = true;
  
  block_info_t noblock = {NOT_BLOCKED, 0};
  tcb_list[0].block_info = noblock;
  
  
  insert_tcb(&tcb_list[0], tcb2, 100000, true);
  insert_tcb(tcb_list[0].next, tcb3, 0, false);

  // Criação da tarefa idle (última da lista)
  last_tid++;             // Incrementa o identificador global para a nova tarefa
  ll_size++;              // Incrementa o tamanho da lista de tarefas

  int idle_idx = ll_size; // Guarda o índice onde a tarefa idle será inserida


  idle_task.tcb = tcb_idle; // Atribui o contexto salvo em tcb_idle (registradores, pilha, PC, CPSR, etc.)
  idle_task.tid = -1; // Define o identificador da tarefa idle

  idle_task.next = NULL;
  idle_task.state = TASK_READY; // Define o estado inicial como TASK_READY (pronta para rodar)


  idle_task.timeslice = 200000;
  idle_task.preemptible = true; // Define que a tarefa idle pode ser preemptada por interrupção
  idle_task.block_info = noblock;
  
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
void insert_tcb(volatile tcb_ll_t *head, tcb_t tcb, uint32_t timeslice, bool preemptible) {
  last_tid++;
  ll_size++;

  if (ll_size >= MAX_TASKS)
    return;

  block_info_t noblock = {NOT_BLOCKED, 0};

  tcb_list[ll_size].tcb = tcb;
  tcb_list[ll_size].tid = last_tid;
  tcb_list[ll_size].next = head->next;
  tcb_list[ll_size].state = TASK_READY;
  tcb_list[ll_size].timeslice = timeslice;
  tcb_list[ll_size].preemptible = preemptible;
  tcb_list[ll_size].block_info = noblock;
  head->next = &tcb_list[ll_size];

  return;
}
