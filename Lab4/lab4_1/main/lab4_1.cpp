#include "esp_log.h"
#include "driver/i2c.h"
#include <string>

#define TAG "TiltDetector"

// Replace with your ICM-42670-P's I2C address and register addresses
#define ICM42670_ADDR 0x68
#define ICM42670_XOUT_H 0x1D
#define ICM42670_YOUT_H 0x1F

// Configure your I2C settings
#define I2C_MASTER_SCL_IO 10
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

// Thresholds for detecting tilt direction
#define TILT_THRESHOLD 1000

// Function to initialize I2C communication
esp_err_t i2c_master_init() {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Function to read two bytes from a register (for X or Y axis data)
int16_t read_axis_data(uint8_t reg) {
    uint8_t data[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 2, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_cmd_link_delete(cmd);

    // Combine high and low bytes
    return (int16_t)((data[0] << 8) | data[1]);
}

// Function to detect tilt direction and log it
void detect_and_log_tilt() {
    int16_t x = read_axis_data(ICM42670_XOUT_H);
    int16_t y = read_axis_data(ICM42670_YOUT_H);
    std::string direction;

    // Determine X direction (LEFT or RIGHT)
    if (x > TILT_THRESHOLD) {
        direction += "RIGHT ";
    } else if (x < -TILT_THRESHOLD) {
        direction += "LEFT ";
    }

    // Determine Y direction (UP or DOWN)
    if (y > TILT_THRESHOLD) {
        direction += "UP";
    } else if (y < -TILT_THRESHOLD) {
        direction += "DOWN";
    }

    // Log the direction if any tilt is detected
    if (!direction.empty()) {
        ESP_LOGI(TAG, "%s", direction.c_str());
    }
}

extern "C" void app_main() {
    // Initialize I2C
    ESP_ERROR_CHECK(i2c_master_init());

    // Main loop to continually check tilt
    while (true) {
        detect_and_log_tilt();
        vTaskDelay(pdMS_TO_TICKS(500));  // Adjust delay as needed
    }
}
