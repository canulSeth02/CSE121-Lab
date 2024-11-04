#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO    22    // GPIO number for I2C SCL
#define I2C_MASTER_SDA_IO    21    // GPIO number for I2C SDA
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000
#define ICM42670_ADDR        0x68  // I2C address when AD0 is low

#define ACCEL_X_OUT_H        0x1F  // Register address for ACCEL_X
#define ACCEL_Y_OUT_H        0x21  // Register address for ACCEL_Y

#define THRESHOLD            2000  // Set an appropriate threshold for tilt sensitivity

// Initialize I2C
static esp_err_t i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Read 16-bit value from two consecutive registers (high and low bytes)
int16_t read_accel_data(uint8_t reg_addr) {
    uint8_t data_h, data_l;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data_h, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data_l, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_cmd_link_delete(cmd);

    return (int16_t)((data_h << 8) | data_l);
}

// Main function to check tilt and log direction
extern "C" void app_main() {
    ESP_ERROR_CHECK(i2c_master_init());

    while (true) {
        int16_t accel_x = read_accel_data(ACCEL_X_OUT_H);
        int16_t accel_y = read_accel_data(ACCEL_Y_OUT_H);

        bool is_up = accel_y > THRESHOLD;
        bool is_down = accel_y < -THRESHOLD;
        bool is_right = accel_x > THRESHOLD;
        bool is_left = accel_x < -THRESHOLD;

        // Log based on tilt
        if (is_up && is_left) {
            ESP_LOGI("Tilt Direction", "UP LEFT");
        } else if (is_up && is_right) {
            ESP_LOGI("Tilt Direction", "UP RIGHT");
        } else if (is_down && is_left) {
            ESP_LOGI("Tilt Direction", "DOWN LEFT");
        } else if (is_down && is_right) {
            ESP_LOGI("Tilt Direction", "DOWN RIGHT");
        } else if (is_up) {
            ESP_LOGI("Tilt Direction", "UP");
        } else if (is_down) {
            ESP_LOGI("Tilt Direction", "DOWN");
        } else if (is_left) {
            ESP_LOGI("Tilt Direction", "LEFT");
        } else if (is_right) {
            ESP_LOGI("Tilt Direction", "RIGHT");
        } else {
            ESP_LOGI("Tilt Direction", "No significant tilt");
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Adjust delay as needed
    }
}
