#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"

/**
 * NOTE:
 *  Take into consideration if your WS2812 is a RGB or RGBW variant.
 *
 *  If it is RGBW, you need to set IS_RGBW to true and provide 4 bytes per 
 *  pixel (Red, Green, Blue, White) and use urgbw_u32().
 *
 *  If it is RGB, set IS_RGBW to false and provide 3 bytes per pixel (Red,
 *  Green, Blue) and use urgb_u32().
 *
 *  When RGBW is used with urgb_u32(), the White channel will be ignored (off).
 *
 */

#define IS_RGBW false
#define NUM_PIXELS 4
#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 16 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 16
#endif

// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define SERVO_PIN 15

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor;


// adapted from https://forum.arduino.cc/index.php?topic=8498.0
// hue is a number from 0 to 360 that describes a color on the color wheel
// sat is the saturation level, from 0 to 1, where 1 is full color and 0 is gray
// brightness sets the maximum brightness, from 0 to 1
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

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}

void pattern_snakes(PIO pio, uint sm, uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(pio, sm, urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(pio, sm, urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(pio, sm, urgb_u32(0, 0, 0xff));
        else
            put_pixel(pio, sm, 0);
    }
}

void pattern_random(PIO pio, uint sm, uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(pio, sm, rand());
}

void pattern_sparkle(PIO pio, uint sm, uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(pio, sm, rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(PIO pio, uint sm, uint len, uint t) {
    uint max = 100; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i) {
        put_pixel(pio, sm, t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(PIO pio, uint sm, uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};

// Initialize the servo PWM
void servo_init() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    float div = 50;
    pwm_set_clkdiv(slice_num, div);
    uint16_t wrap = 59999;
    pwm_set_wrap(slice_num, wrap); // for 50Hz
    pwm_set_enabled(slice_num, true);
}

// Set servo angle between 0 and 180
void set_servo_angle(float angle) {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    float min_pulse_ms = 0.5; 
    float max_pulse_ms = 2.5; 
    float pulse_ms = min_pulse_ms + (angle / 180.0) * (max_pulse_ms - min_pulse_ms);
    uint16_t duty = (pulse_ms / 20.0) * 59999; // 20ms
    pwm_set_gpio_level(SERVO_PIN, duty);
}


int main() {
    //set_sys_clock_48();
    stdio_init_all();

    // todo get free sm
    PIO pio;
    uint sm;
    uint offset;
    
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Servo
    servo_init();

    const int total_time_ms = 5000; // 5 seconds
    const int step_time_ms = 20;    // smooth updates
    const int total_steps = total_time_ms / step_time_ms;

    while (true) {
        for (int step = 0; step < total_steps; step++) {
            float t = (float)step / (float)total_steps;

            // Servo angle
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
                wsColor c = HSBtoRGB(hue, 1.0, 0.2);
                put_pixel(pio, sm, urgb_u32(c.r, c.g, c.b));
            }

            sleep_ms(step_time_ms);
        }
    }
}
