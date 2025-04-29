#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define LED_PIN 25

// Function prototypes
void drawLetter(int x, int y, char c);
void drawMessage(int x, int y, char *message);

int main() {
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize ADC
    adc_init();
    adc_gpio_init(26); // GPIO26 = ADC0
    adc_select_input(0);

    // Initialize SSD1306
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    char message[50];
    char fps_message[50];
    unsigned int t, t2, tdiff;
    float adc_voltage;
    const float conversion_factor = 3.3f / (1 << 12); // 12-bit ADC

    while (true) {
        // Toggle LED
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);


        t = to_us_since_boot(get_absolute_time());

        uint16_t result = adc_read();
        adc_voltage = result * conversion_factor;
        ssd1306_clear();
        sprintf(message, "ADC0 = %.2f V", adc_voltage);
        drawMessage(0, 0, message);
        ssd1306_update();

        t2 = to_us_since_boot(get_absolute_time());
        tdiff = t2 - t;

        // FPS
        float fps = 1000000.0f / tdiff;
        sprintf(fps_message, "FPS = %.2f", fps);
        
        drawMessage(0, 24, fps_message);
        ssd1306_update();

        sleep_ms(500);
    }
}

void drawMessage(int x, int y, char *message) {
    int i = 0;
    while (message[i] != '\0') {
        drawLetter(x + i * 6, y, message[i]); // 5 pixels per char + 1 pixel spacing
        i++;
    }
}

void drawLetter(int x, int y, char c) {
    int j;
    for (j = 0; j < 5; j++) {
        char col = ASCII[c - 0x20][j];
        for (int i = 0; i < 8; i++) {
            char bit = (col >> i) & 0b1;
            ssd1306_drawPixel(x + j, y + i, bit);
        }
    }
}
