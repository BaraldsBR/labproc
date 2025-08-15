#ifndef UART_H
#define UART_H

void uart_init(void);
void uart_send(char c);
char uart_get(void);
void uart_puts(const char *str);

#endif