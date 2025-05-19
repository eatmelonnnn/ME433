#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "font.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define MPU6050_ADDR 0x68

// MPU6050 Registers
#define PWR_MGMT_1    0x6B
#define ACCEL_CONFIG  0x1C
#define GYRO_CONFIG   0x1B
#define ACCEL_XOUT_H  0x3B
#define WHO_AM_I      0x75

// Function Prototypes
void imu_init();
void imu_read_raw(int16_t *ax, int16_t *ay);
void draw_vector(int16_t ax, int16_t ay);
void drawLine(int x0, int y0, int x1, int y1, char color);

int main() {
    stdio_init_all();

    // Init I2C
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Init OLED
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // Init IMU
    imu_init();

    int16_t ax, ay;

    while (true) {
        imu_read_raw(&ax, &ay);
        draw_vector(ax, ay);
        sleep_ms(10);
    }
}

// Initialize the MPU6050
void imu_init() {
    uint8_t buf[2];

    // Wake up chip
    buf[0] = PWR_MGMT_1;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    // Set accel to ±2g
    buf[0] = ACCEL_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    // Set gyro to ±2000 deg/s
    buf[0] = GYRO_CONFIG;
    buf[1] = 0x18;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

// Read X and Y acceleration from MPU6050
void imu_read_raw(int16_t *ax, int16_t *ay) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[6];
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 6, false);

    *ax = (int16_t)(data[0] << 8 | data[1]);
    *ay = (int16_t)(data[2] << 8 | data[3]);
}

// Draw acceleration vector
void draw_vector(int16_t ax, int16_t ay) {
    float fx = ax * 0.000061;  // ±2g
    float fy = ay * 0.000061;

    int cx = 64, cy = 16;
    int dx = (int)(fx * 40);  // scale factor
    int dy = (int)(fy * 40);

    ssd1306_clear();
    drawLine(cx, cy, cx + dx, cy + dy, 1);
    ssd1306_update();
}

// Bresenham line algorithm
void drawLine(int x0, int y0, int x1, int y1, char color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        ssd1306_drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
