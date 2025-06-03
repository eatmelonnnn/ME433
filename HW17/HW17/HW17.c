#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "cam.h"

// === Motor H-Bridge PWM Pins ===
#define AIN1 18
#define AIN2 19
#define BIN1 20
#define BIN2 21

#define PWM_WRAP 6250  // 20kHz PWM

// === Line Sensor Constants ===
#define CAMERA_CENTER 40

// === Control Parameters ===
#define BASE_DUTY_PCT 75.0f  // % duty when going straight
#define MAX_DUTY_PCT 100.0f
#define GAIN 1.0f            // increase this for sharper turns
#define DEAD_BAND 4

void setup_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_clkdiv(slice, 1.0f);
    pwm_set_enabled(slice, true);
}

// Drive motor in forward/reverse using two pins
// REVERSED: PWM forward is actually on the *reverse* pin
void set_motor_pwm_percent(float duty_pct, uint pin_fwd, uint pin_rev) {
    if (duty_pct > 0.0f) {
        pwm_set_gpio_level(pin_fwd, 0);  // previously forward
        pwm_set_gpio_level(pin_rev, (uint16_t)(PWM_WRAP * duty_pct / 100.0f));
    } else if (duty_pct < 0.0f) {
        pwm_set_gpio_level(pin_fwd, (uint16_t)(PWM_WRAP * -duty_pct / 100.0f));
        pwm_set_gpio_level(pin_rev, 0);
    } else {
        pwm_set_gpio_level(pin_fwd, 0);
        pwm_set_gpio_level(pin_rev, 0);
    }
}

int main() {
    stdio_init_all();  // optional for debugging
    init_camera_pins();

    setup_pwm(AIN1);
    setup_pwm(AIN2);
    setup_pwm(BIN1);
    setup_pwm(BIN2);

    while (true) {
        // Capture and process one frame
        setSaveImage(1);
        while (getSaveImage()) {}
        convertImage();

        int center = findLine(IMAGESIZEY / 2);  // white line center
        int error = center - CAMERA_CENTER;

        float left_duty = BASE_DUTY_PCT;
        float right_duty = BASE_DUTY_PCT;

        if (error > DEAD_BAND) {
            right_duty -= GAIN * (error - DEAD_BAND);
        } else if (error < -DEAD_BAND) {
            left_duty -= GAIN * (-error - DEAD_BAND);
        }

        // Clamp duty cycles
        if (left_duty > MAX_DUTY_PCT) left_duty = MAX_DUTY_PCT;
        if (right_duty > MAX_DUTY_PCT) right_duty = MAX_DUTY_PCT;
        if (left_duty < 0.0f) left_duty = 0.0f;
        if (right_duty < 0.0f) right_duty = 0.0f;

        // Send PWM to motors
        set_motor_pwm_percent(left_duty, AIN1, AIN2);   // Left motor
        set_motor_pwm_percent(right_duty, BIN2, BIN1);  // Right motor

        sleep_ms(10);  // run loop ~100Hz
    }

    return 0;
}
