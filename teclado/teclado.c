#include "bcm.h"

#define LINE1_OUTPUT_GPIO 9
#define LINE2_OUTPUT_GPIO 25 
#define LINE3_OUTPUT_GPIO 11
#define LINE4_OUTPUT_GPIO 8

#define LINE1_INPUT_GPIO 19
#define LINE2_INPUT_GPIO 16 
#define LINE3_INPUT_GPIO 26
#define LINE4_INPUT_GPIO 20

void setup_io(int gpio, bool out) {
    int index = gpio / 10;
    int offset = gpio % 10;
    if (out) {
        GPIO_REG(gpfsel[index]) = GPIO_REG(gpfsel[index]) | (0x07 << (3 * offset));
    } else {
        GPIO_REG(gpfsel[index]) = GPIO_REG(gpfsel[index]) & (~(0x07 << (3 * offset)));
    }
}

bool read_io(int gpio) {
    int index = gpio / 32;
    int offset = gpio % 32;
    int  = GPIO_REG(gplev[index]) & (0x01 << offset);
}

void init_keyboard() {
    setup_io(LINE1_OUTPUT_GPIO, true);
    setup_io(LINE2_OUTPUT_GPIO, true);
    setup_io(LINE3_OUTPUT_GPIO, true);
    setup_io(LINE4_OUTPUT_GPIO, true);
    
    setup_io(LINE1_INPUT_GPIO, false);
    setup_io(LINE2_INPUT_GPIO, false);
    setup_io(LINE3_INPUT_GPIO, false);
    setup_io(LINE4_INPUT_GPIO, false);


}