#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define MPU6050_ADDR 0x68

#define PWR_MGMT_1    0x6B
#define ACCEL_CONFIG  0x1C
#define GYRO_CONFIG   0x1B
#define ACCEL_XOUT_H  0x3B

void imu_init();
void imu_read(int16_t *ax, int16_t *ay, int16_t *az,
                  int16_t *gx, int16_t *gy, int16_t *gz);
void draw_vector(int16_t ax, int16_t ay);
void drawLine(int x0, int y0, int x1, int y1, char color);

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    imu_init();
    sleep_ms(100); // Let chip stabilize

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    int16_t ax, ay, az, gx, gy, gz;

    while (true) {
        imu_read(&ax, &ay, &az, &gx, &gy, &gz);
        draw_vector(ax, ay);  // Still visualizing XY accel
        sleep_ms(20);
    }
}

void imu_init() {
    uint8_t buf[2];

    // Wake chip
    buf[0] = PWR_MGMT_1;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    // Accel ±2g
    buf[0] = ACCEL_CONFIG;
    buf[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);

    // Gyro ±2000 dps
    buf[0] = GYRO_CONFIG;
    buf[1] = 0x18;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

void imu_read(int16_t *ax, int16_t *ay, int16_t *az,
                  int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t reg = ACCEL_XOUT_H;
    uint8_t data[14];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 14, false);

    *ax = (int16_t)(data[0] << 8 | data[1]);
    *ay = (int16_t)(data[2] << 8 | data[3]);
    *az = (int16_t)(data[4] << 8 | data[5]);
    *gx = (int16_t)(data[8] << 8 | data[9]);
    *gy = (int16_t)(data[10] << 8 | data[11]);
    *gz = (int16_t)(data[12] << 8 | data[13]);
}

void draw_vector(int16_t ax, int16_t ay) {
    float fx = ax * 0.000061;
    float fy = ay * 0.000061;

    int cx = 64, cy = 16;
    int dx = (int)(-fx * 60);  // adjust sensitivity
    int dy = (int)(fy * 60);

    ssd1306_clear();
    drawLine(cx, cy, cx + dx, cy + dy, 1);
    ssd1306_update();
}

void drawLine(int x0, int y0, int x1, int y1, char color) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int sx, sy;

    if (dx < 0) {
        dx = -dx;
        sx = -1;
    } else {
        sx = 1;
    }

    if (dy < 0) {
        dy = -dy;
        sy = -1;
    } else {
        sy = 1;
    }

    int err = dx - dy;

    while (1) {
        ssd1306_drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
