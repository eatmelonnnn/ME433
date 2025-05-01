/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_READ_VOLTAGE 0
#define FLAG_LED_ON       1
#define FLAG_LED_OFF      2
#define FLAG_VOLTAGE_DONE 3

volatile float voltage = 0.0f;

void core1_entry() {
    // Init ADC and GPIO
    adc_init();
    adc_gpio_init(26); // A0 is GPIO26
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 1);

    while (1) {
        uint32_t flag = multicore_fifo_pop_blocking();

        if (flag == FLAG_READ_VOLTAGE) {
            adc_select_input(0);
            uint16_t result = adc_read();
            voltage = 3.3f * result / 4095.0f;

            multicore_fifo_push_blocking(FLAG_VOLTAGE_DONE);
        }
        else if (flag == FLAG_LED_ON) {
            gpio_put(15, 0);
        }
        else if (flag == FLAG_LED_OFF) {
            gpio_put(15, 1);
        }
    }
}

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);

    // Wait for it to start up

    int input;
    while (1) {
        printf("Type 0 to read voltage on pin 26, 1 to turn on LED on pin 20, 2 to turn off LED on pin 20:\n");
        scanf("%d", &input);
        printf("Command entered: %d\n", input);

        if (input == 0) {
            multicore_fifo_push_blocking(FLAG_READ_VOLTAGE);
            uint32_t ack = multicore_fifo_pop_blocking();
            if (ack == FLAG_VOLTAGE_DONE) {
                printf("Voltage on pin 15: %.2f V\n", voltage);
            }
        } else if (input == 1) {
            multicore_fifo_push_blocking(FLAG_LED_ON);
            printf("LED on pin 20 on.\n");
        } else if (input == 2) {
            multicore_fifo_push_blocking(FLAG_LED_OFF);
            printf("LED on pin 20 off.\n");
        } else {
            printf("Invalid input.\n");
        }
    }
}