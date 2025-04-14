#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   20
#define PIN_SCK  18
#define PIN_MOSI 19

#define VREF 3.3f
#define MAX_DAC_VAL 1023

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop");
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}

void writeDac(int channel, float voltage){
    if (voltage < 0) voltage = 0;
    if (voltage > VREF) voltage = VREF;

    uint16_t value = (uint16_t)((voltage / VREF) * MAX_DAC_VAL);

    uint16_t command = 0;
    command |= (channel & 0x01) << 15;       // Channel select A=0, B=1
    command |= 0b111 << 12;                  // Buffer=1, Gain=1x, Shutdown=active
    command |= (value & 0x3FF) << 2;         // 10-bit data shifted to bits 11-2

    uint8_t data[2];
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();

    // SPI initialization
    spi_init(SPI_PORT, 1000 * 1000); // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    const float dt = 1.0f / 500.0f; // 500Hz update
    float time = 0.0f;

    while (true) {
        // 2Hz sine wave on channel A
        float sine_v = (sinf(2 * M_PI * 2.0f * time) + 1.0f) * 0.5f * VREF;
        writeDac(0, sine_v);

        // 1Hz triangle wave on channel B
        float tri_t = fmodf(time, 1.0f);
        float triangle_v = tri_t < 0.5f ? (2.0f * tri_t * VREF) : (2.0f * (1.0f - tri_t) * VREF);
        writeDac(1, triangle_v);

        sleep_us(2000); // 500Hz
        time += dt;
    }

    return 0;
}
