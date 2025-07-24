
/**
 * Salva o contexto do processador na posição de memória
 * especificada pelo ponteiro tcb
 */
.macro save_context
   push {r0}
   ldr r0, =tcb               // &tcb atual
   ldr r0, [r0]
   stmib r0, {r1-r14}^        // salva registradores do usuário no tcb
   str lr, [r0, #60]          // salva endereço de retorno (futuro pc) no tcb
   mrs r1, spsr
   str r1, [r0, #64]          // salva cpsr do usuário no tcb
   pop {r1}
   str r1, [r0]               // salva o valor original de r0 no tcb
.endm

/**
 * Restaura o contexto da posição de memória
 * especificada pelo ponteiro tcb
 */
.macro restore_context
   ldr r0, =tcb               // &tcb atual
   ldr r0, [r0]
   ldr r1, [r0, #64]          // recupera cpsr do usuário (spsr)
   msr spsr, r1
   ldmib r0, {r1-r14}^        // recupera valores dos registradores do usuário
   ldr lr, [r0, #60]          // recupera endereço de retorno
   ldr r0, [r0]               // recupera r0 original
.endm

.text
/**
 * Espera tempo definido pelo valor de r0.
 */
.global delay
delay:
   subs r0, r0, #1
   bne delay
   mov pc, lr

/**
 * Habilita ou desabilita interrupções
 * param r0 0 = desabilita, diferente de zero = habilita
 */
.global enable_irq
enable_irq:
   movs r0, r0
   beq disable
   mrs r0, cpsr
   bic r0, r0, #(1 << 7)
   msr cpsr, r0
   mov pc, lr
disable:
   mrs r0, cpsr
   orr r0, r0, #(1 << 7)
   msr cpsr, r0
   mov pc, lr

/**
 * Trava o processador (panic)
 */
.global panic
panic:
   wfe
   b panic

/*
 * Tratamento da interrupção de software (chamada de sistema).
 */
.global swi
swi:
   cpsid if                   // desabilita interrupções no modo swi (região crítica)
   save_context               // salva o contexto do processo em execução
   mov r0, r1                 // parâmetro da chamada de sistema
   bl trata_swi               // chama função trata_swi em C
   b task_switch              // retorna ao task (ou a outro task)

/*
 * Tratamento das interrupções.
 */
.global irq
irq:
   sub lr, lr, #4
   save_context               // salva o contexto do processo em execução
   bl trata_irq               // chama função trata_irq em C
   b task_switch              // retorna ao task (ou a outro task)

/*
 * Restaura o contexto original e retorna para o task apontado por tcb.
 */
.global task_switch
task_switch:
   restore_context            // retorna o contexto salvo em tcb
   movs pc, lr                // retorna e muda o modo, restaurando cpsr (flag S)
   
