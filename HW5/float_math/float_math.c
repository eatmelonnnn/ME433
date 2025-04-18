#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(10000); // Time to run screen

    volatile float f1, f2;
    volatile float f_add, f_sub, f_mult, f_div;
    const int N = 1000;
    uint64_t dt;

    printf("Enter two floats to use: \n");
    scanf("%f %f", &f1, &f2);
    printf("Float 1: %f\nFloat 2: %f\n", f1, f2);

    // Addition
    absolute_time_t t1 = get_absolute_time();
    for (int i = 0; i < N; i++) {
        f_add = f1 + f2;
    }
    absolute_time_t t2 = get_absolute_time();
    dt = to_us_since_boot(t2) - to_us_since_boot(t1);
    printf("Addition: %d clock cycles\n", (int)((dt * 150) / N));

    // Subtraction
    t1 = get_absolute_time();
    for (int i = 0; i < N; i++) {
        f_sub = f1 - f2;
    }
    t2 = get_absolute_time();
    dt = to_us_since_boot(t2) - to_us_since_boot(t1);
    printf("Subtraction: %d clock cycles\n", (int)((dt * 150) / N));

    // Multiplication
    t1 = get_absolute_time();
    for (int i = 0; i < N; i++) {
        f_mult = f1 * f2;
    }
    t2 = get_absolute_time();
    dt = to_us_since_boot(t2) - to_us_since_boot(t1);
    printf("Multiplication: %d clock cycles\n", (int)((dt * 150) / N));

    // Division
    t1 = get_absolute_time();
    for (int i = 0; i < N; i++) {
        f_div = f1 / f2;
    }
    t2 = get_absolute_time();
    dt = to_us_since_boot(t2) - to_us_since_boot(t1);
    printf("Division: %d clock cycles\n", (int)((dt * 150) / N));

    return 0;
}
