#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

// ==== DEFINITIONS ====
#define IS_RGBW false
#define NUM_PIXELS 4
#define WS2812_PIN 16
#define SERVO_PIN 15

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor;


// Convert HSB (Hue, Saturation, Brightness) to RGB
wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0, green = 0.0, blue = 0.0;

    if (sat == 0.0) {
        red = green = blue = brightness;
    } else {
        if (hue == 360.0) hue = 0;
        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;
        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0: red = brightness; green = cc; blue = aa; break;
            case 1: red = bb; green = brightness; blue = aa; break;
            case 2: red = aa; green = brightness; blue = cc; break;
            case 3: red = aa; green = bb; blue = brightness; break;
            case 4: red = cc; green = aa; blue = brightness; break;
            case 5: red = brightness; green = aa; blue = bb; break;
            default: red = green = blue = 0; break;
        }
    }

    wsColor c;
    c.r = red * 255.0;
    c.g = green * 255.0;
    c.b = blue * 255.0;
    return c;
}

// Send a pixel color to the LED strip
static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Combine RGB into a single 24-bit value
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Initialize the servo PWM
void servo_init() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_wrap(slice_num, 24999); // for 50Hz
    pwm_set_clkdiv(slice_num, 60.f); // divider
    pwm_set_enabled(slice_num, true);
}

// Set servo to a given angle between 0 and 180
void set_servo_angle(float angle) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    float min_duty = 0.025;
    float max_duty = 0.125;
    float duty = min_duty + (angle / 180.0) * (max_duty - min_duty);
    uint16_t level = duty * 24999; // match wrap
    pwm_set_gpio_level(SERVO_PIN, level);
}

// ==== MAIN PROGRAM ====
int main() {
    stdio_init_all();
    printf("HW8 Start\n");

    // Initialize WS2812
    PIO pio;
    uint sm;
    uint offset;
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Initialize Servo
    servo_init();

    const int total_time_ms = 5000; // 5 seconds
    const int step_time_ms = 20;    // smooth updates
    const int total_steps = total_time_ms / step_time_ms;

    while (true) {
        for (int step = 0; step < total_steps; step++) {
            float t = (float)step / (float)total_steps;

            // Servo sweep: 0 -> 180 -> 0 degrees
            float angle;
            if (t < 0.5) {
                angle = t * 2.0 * 180.0; // ramp up
            } else {
                angle = (1.0 - (t-0.5)*2.0) * 180.0; // ramp down
            }
            set_servo_angle(angle);

            // LED rainbow
            for (int i = 0; i < NUM_PIXELS; i++) {
                float hue = fmod((360.0 * t * 2) + (i * (360.0/NUM_PIXELS)), 360.0);
                wsColor c = HSBtoRGB(hue, 1.0, 0.2); // Brightness = 0.2 (dim)
                put_pixel(pio, sm, urgb_u32(c.r, c.g, c.b));
            }

            sleep_ms(step_time_ms);
        }
    }
}
