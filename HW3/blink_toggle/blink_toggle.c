#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define GPIO_WATCH_PIN 2               // GPIO for button input
#define LED_PIN PICO_DEFAULT_LED_PIN  // LED pin on PICO

volatile int press_count = 0;
volatile bool led_state = false;

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == GPIO_WATCH_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        press_count++;
        led_state = !led_state;
        gpio_put(LED_PIN, led_state);
        printf("Button pressed %u times\n", press_count);
    }
}

int main() {
    stdio_init_all(); 
    gpio_init(GPIO_WATCH_PIN);
    gpio_set_dir(GPIO_WATCH_PIN, GPIO_IN);


    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, false);

    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Wait forever
    while (1) {
    }
}
