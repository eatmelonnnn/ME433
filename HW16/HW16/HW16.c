#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>

#define AIN1 18
#define AIN2 19
#define BIN1 20
#define BIN2 21

#define PWM_WRAP 6250  // 20kHz

int duty = 0; 

void setup_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, 1.0f);
    pwm_set_enabled(slice, true);
}

void set_motor_pwm(int duty, uint pin_fwd, uint pin_rev) {
    if (duty > 0) {
        pwm_set_gpio_level(pin_fwd, duty * PWM_WRAP / 100);
        pwm_set_gpio_level(pin_rev, 0);
    } else if (duty < 0) {
        pwm_set_gpio_level(pin_fwd, 0);
        pwm_set_gpio_level(pin_rev, -duty * PWM_WRAP / 100);
    } else {
        pwm_set_gpio_level(pin_fwd, 0);
        pwm_set_gpio_level(pin_rev, 0);
    }
}

int main() {
    stdio_usb_init();
    sleep_ms(2000);  // Allow time for USB connection

    setup_pwm(AIN1);
    setup_pwm(AIN2);
    setup_pwm(BIN1);
    setup_pwm(BIN2);

    printf("Two-motor control (GPIO18–21, 20kHz PWM).\n");
    printf("Use '+' and '-' to adjust both motors together.\n");

    while (true) {
        int ch = getchar_timeout_us(0);

        if (ch == '=' && duty < 100) duty++; // + didnt work for some reason :(
        if (ch == '-' && duty > -100) duty--;

        set_motor_pwm(duty, AIN1, AIN2);
        set_motor_pwm(duty, BIN1, BIN2);

        printf("Duty cycle: %d%%\r", duty);
        sleep_ms(50);
    }

    return 0;
}