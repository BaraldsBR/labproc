
SECTIONS {
  .boot 0x8000 : {
     *(.boot)
   }

   . = ALIGN(8);
  .text : {
    *(.text)
  }

   . = ALIGN(8);
  .rodata : {
    *(.rodata)
  }

  . = ALIGN(8);
  data_begin = .;
  .data : { *(.data) }

  . = ALIGN(8);
  bss_begin = .;
  .bss : { *(.bss*) }
  bss_end = .;

  .heapstack : {
     . = ALIGN(8);
     heap_addr = .;
     . = . + 32K;
     stack_sys = .;
     . = . + 4K;
     stack_irq = .;
     . = . + 4K;
     stack_user1 = .;
     . = . + 4K;
     stack_user2 = .;
     . = . + 4K;
     stack_user3 = .;
   }
}
