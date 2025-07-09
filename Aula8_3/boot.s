
.section .boot, "ax"
.global start
start:
  /*
   * Vetor de interrupções
   * Deve ser copiado no enderço 0x0000
   */
  ldr pc, _reset
  ldr pc, _undef
  ldr pc, _swi
  ldr pc, _iabort
  ldr pc, _dabort
  nop
  ldr pc, _irq
  ldr pc, _fiq

  _reset:    .word   reset
  _undef:    .word   panic
  _swi:      .word   swi
  _iabort:   .word   panic
  _dabort:   .word   panic
  _irq:      .word   irq
  _fiq:      .word   panic

/*
 * Vetor de reset: início do programa aqui.
 */
reset:
.if RPICPU == 2 
  /*
   * CPU v.7 (BCM2836 ou BCM2837).
   * Verifica priviégio de execução EL2 (HYP) ou EL1 (SVC)
   */
  mrs r0, cpsr
  and r0, r0, #0x1f
  cmp r0, #0x1a
  bne continua

  /*
   * Sai do modo EL2 (HYP)
   */
  mrs r0, cpsr
  bic r0, r0, #0x1f
  orr r0, r0, #0x13
  msr spsr_cxsf, r0
  add lr, pc, #4       // aponta o rótulo 'continua'
  msr ELR_hyp, lr
  eret                 // 'retorna' do privilégio EL2 para o EL1

continua:
  /*
   * Executa somente o core 0.
   */
  mrc p15,0,r0,c0,c0,5    // registrador MPIDR
  tst r0, #0x03
  beq core0
  b panic                 // se algum núcleo executar daqui, bloqueia

core0:
.endif

  /*
   * configura os stacks pointers do sistema
   */
  mov r0, #0xd2           // Modo IRQ
  msr cpsr, r0
  ldr sp, =stack_irq

  mov r0, #0xd3           // Modo SVC, interrupções mascaradas
  msr cpsr, r0
  ldr sp, =stack_sys

  // Continua executando no modo supervisor (SVC), interrupções desabilitadas

  /*
   * Copia o vetor de interrupções para o endereço 0
   */
  ldr r0, =start
  mov r1, #0x0000
  ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9}
  stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9}
  ldmia r0!, {r2,r3,r4,r5,r6,r7,r8,r9}
  stmia r1!, {r2,r3,r4,r5,r6,r7,r8,r9}

   /*
    * Zera segmento BSS
    */
   ldr r0, =bss_begin
   ldr r1, =bss_end
   mov r2, #0
loop_bss:
   cmp r0, r1
   bge done_bss
   strb r2, [r0], #1
   b loop_bss

done_bss:
  /*
   * Executa a função main
   */
  b system_main

