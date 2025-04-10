#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define BUTTON_PIN 2                   // GPIO for button input
#define LED_PIN PICO_DEFAULT_LED_PIN   // Onboard LED


int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, true);  // LED on

    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    printf("Press the button to begin.\n");
    while (gpio_get(BUTTON_PIN)) {
        sleep_ms(10);
    }
    gpio_put(LED_PIN, false);  // Turn off LED once button is pressed

    while (1) {
        int num_samples = 0;
        printf("Enter number of samples to take from 1 to 100: ");
        scanf("%d", &num_samples);

        printf("Taking %d samples at 100Hz...\n", num_samples);
        for (int i = 0; i < num_samples; ++i) {
            uint16_t raw = adc_read(); // read adc value
            float voltage = (3.3 * raw) / 4095.0; // convert to voltage
            printf("Sample %d: %.3f V\n", i + 1, voltage);
            sleep_ms(10); // 100Hz = 10ms interval
        }
    }
}
