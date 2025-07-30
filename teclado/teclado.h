#ifndef TECLADO
#define TECLADO

#include <stdbool.h>

#define COL1_OUTPUT_GPIO 19
#define COL2_OUTPUT_GPIO 13 
#define COL3_OUTPUT_GPIO 6
#define COL4_OUTPUT_GPIO 5

#define ROW1_INPUT_GPIO 16
#define ROW2_INPUT_GPIO 20 
#define ROW3_INPUT_GPIO 12
#define ROW4_INPUT_GPIO 21

#define DEBOUNCE_CYCLES 2000
#define ROWS 4
#define COLS 4

#define OUTPUT_LENGTH 1024

void setup_io(int gpio, bool out);
bool read_io(int gpio);
void write_io(int gpio, bool gpio_on);
void init_keyboard(bool use_uart);
char keyboard_position_conversion(int row, int col);
void read_keyboard(void);

#endif //TECLADO
