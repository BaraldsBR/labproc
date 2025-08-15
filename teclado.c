#include "bcm.h"
#include "teclado.h"
#include "uart.h"

int read_matrix[ROWS][COLS], last_output;
bool already_read_matrix[ROWS][COLS];

int output[OUTPUT_LENGTH];

int out_pins[ROWS] = {COL1_OUTPUT_GPIO, COL2_OUTPUT_GPIO, COL3_OUTPUT_GPIO, COL4_OUTPUT_GPIO};
int in_pins[COLS] = {ROW1_INPUT_GPIO, ROW2_INPUT_GPIO, ROW3_INPUT_GPIO, ROW4_INPUT_GPIO};
bool uart_enabled;

void setup_io(int gpio, bool out) {
    int index = gpio / 10;
    int offset = gpio % 10;
    if (out) {
        GPIO_REG(gpfsel[index]) = (GPIO_REG(gpfsel[index]) & (~(0x07 << (3 * offset)))) | (0x01 << (3 * offset));
    } else {
        GPIO_REG(gpfsel[index]) = GPIO_REG(gpfsel[index]) & (~(0x07 << (3 * offset)));
    }
}

bool read_io(int gpio) {
    int index = gpio / 32;
    int offset = gpio % 32;
    int read_gpio = (GPIO_REG(gplev[index]) & (0x01 << offset));
    return read_gpio == (0x01 << offset);
}

void write_io(int gpio, bool gpio_on) {
    int index = gpio / 32;
    int offset = gpio % 32;
    if (gpio_on) {
        GPIO_REG(gpset[index]) = 0x01 << offset;
    } else {
        GPIO_REG(gpclr[index]) = 0x01 << offset;
    }
}

void init_keyboard(bool use_uart) {
    uart_enabled = use_uart;
    if (uart_enabled){
        uart_init();
    }

    setup_io(COL1_OUTPUT_GPIO, true);
    setup_io(COL2_OUTPUT_GPIO, true);
    setup_io(COL3_OUTPUT_GPIO, true);
    setup_io(COL4_OUTPUT_GPIO, true);
    
    setup_io(ROW1_INPUT_GPIO, false);
    setup_io(ROW2_INPUT_GPIO, false);
    setup_io(ROW3_INPUT_GPIO, false);
    setup_io(ROW4_INPUT_GPIO, false);

    for (int i = 0; i < COLS; i++)
        write_io(out_pins[i], false);  
    
    last_output = 0;
}

char keyboard_position_conversion(int row, int col) {
    bool is_letter = (row == 3) || ((row == 2) && (col > 1));
    if (is_letter) {
        return (char) (col-2) + ((row-2) * ROWS) + 65;
    } else {
        return (char) col + (row * ROWS) + 48;
    }
}

void read_keyboard() {
    while (1) {
        for (int c = 0; c < COLS; c++) {
            write_io(out_pins[c], true);
    
            for(int r = 0; r < ROWS; r++) {
                if (read_io(in_pins[r])) {
                    if ((read_matrix[r][c] < FIRST_DEBOUNCE_CYCLES && !already_read_matrix[r][c]) 
                        || (read_matrix[r][c] < REMAINING_DEBOUNCE_CYCLES && already_read_matrix[r][c])
                    ) 
                        read_matrix[r][c]++;
                    else {
                        output[last_output++] = r * ROWS + c;
                        
                        if (uart_enabled)
                            uart_send(keyboard_position_conversion(r, c));
                        
                        if (!already_read_matrix[r][c]) {
                            read_matrix[r][c] = -SECOND_DEBOUNCE_DELAY;
                            already_read_matrix[r][c] = true;
                        } else{
                            read_matrix[r][c] = 0;
                        }

                        if (last_output >= OUTPUT_LENGTH) {
                            last_output = 0;
                        }
                    }
                } else {
                    already_read_matrix[r][c] = false;
                    read_matrix[r][c] = 0;
                }
            }
    
            write_io(out_pins[c], false);
        }
    }
}
