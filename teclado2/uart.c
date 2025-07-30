#include "uart.h"
#include <stdint.h>

#define UART0_BASE 0x3F201000

#define UART0_DR   (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART0_FR   (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define UART0_IBRD (*(volatile uint32_t *)(UART0_BASE + 0x24))
#define UART0_FBRD (*(volatile uint32_t *)(UART0_BASE + 0x28))
#define UART0_LCRH (*(volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART0_CR   (*(volatile uint32_t *)(UART0_BASE + 0x30))
#define UART0_IMSC (*(volatile uint32_t *)(UART0_BASE + 0x38))
#define UART0_ICR  (*(volatile uint32_t *)(UART0_BASE + 0x44))


void uart_init(void) {
    UART0_CR = 0x00000000;
    
    UART0_ICR = 0x7FF;

    UART0_IBRD = 312;
    UART0_FBRD = 32;
    
    UART0_LCRH = (3 << 5);
    UART0_CR = 0x301;
}

void uart_send(char c){
    while(UART0_FR & (1 << 5));
    UART0_DR = c;
}

char uart_get(void){
    while (UART0_FR & (1 << 4));
    return (char)(UART0_DR & 0xFF);
}

void uart_puts(const char *str){
    while (*str)
        uart_send(*str++);
}