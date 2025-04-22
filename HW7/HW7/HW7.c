#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9



int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        char message[50];
        t = read time
        //ssd1306_clear();
        sprintf(message, "Hello 4");
        drawMessage(10,20,message);
        ssd1306_update();
        t2 = read time;
        tdiff = t2 - t; //in microseconds
        // Find out how long it takes to write to the screen

        sleep_ms(1000);
    }
}


void drawMessage(int x, int y, char *message) {
    int i = 0;
    while (message[i] != '\0') {
        drawLetter(x + i*5 + 1, y, message[i]);
        i++;
    }
}


void drawLetter(int x, int y, char c){
    int j;
    for (j=0; j<5; j++){
        char col = ASCII[c-0x20][j];
        int i;
        for (i=0, i<8, i++){
            char bit = (col >> i) & 0b1;
                ssd1306_drawPixel(x+j ,y+i, bit);
            }
    }
}

