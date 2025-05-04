#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_CS_RAM 21
#define PIN_CS_DAC 20
#define VREF 3.3

union FloatInt {
    float f;
    uint32_t i;
};

void cs_select(uint cs_pin) {
    gpio_put(cs_pin, 0);
}
void cs_deselect(uint cs_pin) {
    gpio_put(cs_pin, 1);
}

void write_dac(float voltage) {
    uint16_t value = (uint16_t)(voltage / VREF * 4095); // 12-bit resolution
    uint16_t command = 0b0011000000000000 | (value & 0x0FFF); // Channel A, buffered, gain=1x, active

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS_DAC);
}

void spi_ram_init() {
    uint8_t buff[2] = {0b00000001, 0b01000000}; // Write status register, sequential mode
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buff, 2);
    cs_deselect(PIN_CS_RAM);
}

void ram_write(uint16_t address, float v) {
    union FloatInt num;
    num.f = v;
    uint8_t buff[7] = {
        0b00000010,               // write command
        (address >> 8) & 0xFF,    // high byte
        address & 0xFF,           // low byte
        (num.i >> 24) & 0xFF,
        (num.i >> 16) & 0xFF,
        (num.i >> 8) & 0xFF,
        num.i & 0xFF
    };
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, buff, 7);
    cs_deselect(PIN_CS_RAM);
}

float ram_read(uint16_t address) {
    uint8_t cmd[3] = {
        0b00000011,
        (address >> 8) & 0xFF,
        address & 0xFF
    };
    uint8_t data[4];

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, cmd, 3);
    spi_read_blocking(SPI_PORT, 0x00, data, 4);
    cs_deselect(PIN_CS_RAM);

    union FloatInt num;
    num.i = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    return num.f;
}

int main() {
    stdio_init_all();

    // SPI initialization    
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    gpio_set_function(PIN_CS_RAM, GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);
    gpio_put(PIN_CS_DAC, 1);

    spi_ram_init();

    // Sine Wave
    for (int i = 0; i < 1000; i++) {
        float theta = 2 * M_PI * i / 1000.0;
        float v = (sinf(theta) + 1.0f) * (VREF / 2.0f); // scale 0 to 3.3V
        ram_write(i * 4, v); // 4 bytes per float
    }

    // Read and write to DAC
    int index = 0;
    while (true) {
        float v = ram_read(index * 4);
        write_dac(v);
        sleep_ms(1); // 1ms = 1000Hz / 1000 samples = 1Hz full sine
        index = (index + 1) % 1000;
    }

    return 0;
}
