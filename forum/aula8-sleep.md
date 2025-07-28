Olá, pessoal!

Complementando a discussão anterior sobre a implementação da chamada `sleep()` e o problema do escalonador ficar "sem processos prontos para executar", eu, @, @, @, e @ resolvemos esse problema introduzindo uma tarefa idle no sistema, como fallback para quando todos os processos estão dormindo.

---

### 😴 Problema

Anteriormente, ao utilizar `sleep()`, existia o risco de todos os processos entrarem em estado `TASK_BLOCKED` ao mesmo tempo, o que levava o sistema a um estado onde nenhuma tarefa estava pronta (`TASK_READY`) e o escalonador ficava sem o que fazer.

Isso mandava o escalonador para o panic imedietamente, ficando por lá indefinidamente.

---

### ✅ Solução: Adição da tarefa `idle`

Implementamos uma tarefa especial de background, chamada `idle_task`, que serve de "último recurso" do escalonador. Ela roda sempre que nenhuma outra tarefa está disponível para execução.

#### Mudanças principais:

1. **Criação da estrutura da tarefa `idle`:**

A `idle_task` é uma instância global do tipo `tcb_ll_t`, com `tid = -1`, `preemptible = true` e seu PC apontando para o panic (loop simples, como um `while(1)`).

```c
idle_task.tid = -1;
idle_task.state = TASK_READY;
idle_task.tcb.regs[15] = (uint32_t)&panic; // PC da idle
```

2. **Atualização do escalonador (`find_next_head`)**:

No laço de varredura das tarefas, se nenhuma estiver `READY`, o escalonador retorna um ponteiro para a `idle_task`.

```c
if (i->state == TASK_READY) return i;

idle_task.next = head;
return &idle_task;
```

3. Lógica de desbloqueio automático:

Ao varrer os `TASK_BLOCKED`, o escalonador verifica se algum já pode ser liberado. Se sim, ele é automaticamente reclassificado para `TASK_READY`.

```c
if (i->state == TASK_BLOCKED) check_task_block(i);
```

```c
void check_task_block(volatile tcb_ll_t *task) {
  switch (task->block_info.type) {
    case(SLEEPING):
      if (ticks >= task->block_info.info){
        task->state = TASK_READY;
        block_info_t noblock = {NOT_BLOCKED, 0};
        task->block_info = noblock;
      }
      return;
    default:
      return;
  }
}
```

Isso garante que, eventualmente, uma tarefa voltará a estar disponível e o escalonador poderá retomar o funcionamento normal sem intervenção externa.

---

### Conclusão

Com a `idle_task` o sistema consegue lidar com estados de bloqueio completo e garante que o escalonador tenha sempre um caminho seguro a seguir.