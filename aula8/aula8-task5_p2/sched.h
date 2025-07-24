
#pragma once

typedef struct tcb_s tcb_t;
typedef struct tcb_ll tcb_ll_t;

extern volatile int tid;
extern volatile tcb_t *tcb;

void yield(void);
int getpid(void);
unsigned getticks(void);
void pcs_exit(void);
void sleep(unsigned ticks_to_sleep);

void insert_tcb(volatile tcb_ll_t *, tcb_t);
void schedule(void);
void trata_swi(unsigned);
void trata_irq(void);
void sched_init(void);
