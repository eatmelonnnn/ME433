#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

union FloatInt {
    float f;
    uint32_t i;
    };


void init_ram(){
    uint8_t buff[2];
    buff[0] = 0b00000101; // I want to change the status register
    buff[1] = 0b01000000; // to sequential mode

    // Set the CS pin low to select the device
    // cs low
    spi_write_blocking(spi_default, buff, 2);
    // cd high
    //
}

void ram_write(uint16_t address,float v){
    uint8_t buff[7];
    buff[0] = 0b00000010; // I want to write to the memory (instrction)
    buff[1] = (uint8_t)(address >> 8); // high byte
    buff[2] = (uint8_t)(address & 0xFF); // low byte

    union FloatInt num;
    num.f = v;

    buff[3] = num.i >> 24; // data
    buff[4] ...
    buff[5] ...
    buff[6] ...

    // Set the CS pin low to select the device
    // cs low
    spi_write_blocking(spi_default, buff, 7);
    // cd high
}

float ram_read(uint16_t){
    uint8_t out_buff[7], in_buff[7];

    out_buff[0] = instruction
    out_buff[1] = address highbyte
    out_buff[2] = address lowbyte
    // Set the CS in low to select the device
    // cs low
    spi_write_read_blocking(spi_default, out_buff, in_buff,7);
    // cd high
    union FloatInt num;
    num.i = 0;


    v = in_buff[3] << 24 | (in_buff[4] << 16) 
    return num.f
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    init_ram();


    for (i = 0 to 1000);
        calulate v = sin(t);
        ram_write(address, v)

    while (true) {
        //read or one address
        float v = ram_read(address)

        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
