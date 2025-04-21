#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C configuration
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define MCP23008_ADDR 0x20  //xs A0, A1, A2 are all tied to GND
#define IODIR 0x00
#define OLAT  0x0A
#define GPIO  0x09

#define LED_PIN 25  // Onboard LED

void write_register(uint8_t addr, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, addr, buf, 2, false);
}

uint8_t read_register(uint8_t addr, uint8_t reg) {
    i2c_write_blocking(I2C_PORT, addr, &reg, 1, true);
    uint8_t val;
    i2c_read_blocking(I2C_PORT, addr, &val, 1, false);
    return val;
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(I2C_PORT, 400 * 1000); 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    sleep_ms(100); 

    // Set GP0 as input, GP7 as output, others as inputs
    write_register(MCP23008_ADDR, IODIR, 0b01111111);

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);

        uint8_t gpio_val = read_register(MCP23008_ADDR, GPIO);
        bool button_pressed = (gpio_val & 0b00000001) == 0;

        if (button_pressed) {
            write_register(MCP23008_ADDR, OLAT, 0b10000000); 
        } else {
            write_register(MCP23008_ADDR, OLAT, 0b00000000); 
        }

        sleep_ms(400);
    }
}
