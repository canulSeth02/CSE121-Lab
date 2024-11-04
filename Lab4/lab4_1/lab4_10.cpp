#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO           8               /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           10               /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_1        /*!< I2C master i2c port number */
#define I2C_MASTER_FREQ_HZ          100000           /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
#define ICM42670_ADDRESS            0x68             /*!< I2C address of the ICM-42670-P */
#define ACCEL_DATA_X1               0x1F             /*!< Register address for ACCEL_X1 */
#define ACCEL_DATA_X0               0x20             /*!< Register address for ACCEL_X0 */
#define ACCEL_DATA_Y1               0x21             /*!< Register address for ACCEL_Y1 */
#define ACCEL_DATA_Y0               0x22             /*!< Register address for ACCEL_Y0 */

static const char *TAG = "ICM42670";

static esp_err_t i2c_master_init(void) {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_master_read_reg(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    esp_err_t ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (i2c_addr << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

void app_main(void) {
    ESP_ERROR_CHECK(i2c_master_init());

    uint8_t accel_data[4];
    int16_t accel_x, accel_y;

    while (1) {
        // Read accelerometer X and Y data
        i2c_master_read_reg(ICM42670_ADDRESS, ACCEL_DATA_X1, &accel_data[0], 1);
        i2c_master_read_reg(ICM42670_ADDRESS, ACCEL_DATA_X0, &accel_data[1], 1);
        i2c_master_read_reg(ICM42670_ADDRESS, ACCEL_DATA_Y1, &accel_data[2], 1);
        i2c_master_read_reg(ICM42670_ADDRESS, ACCEL_DATA_Y0, &accel_data[3], 1);

        accel_x = (accel_data[0] << 8) | accel_data[1];
        accel_y = (accel_data[2] << 8) | accel_data[3];

        const int16_t threshold = 1000; // Adjust the threshold according to your needs
        bool up = false, down = false, left = false, right = false;

        if (accel_x > threshold) {
            right = true;
        } else if (accel_x < -threshold) {
            left = true;
        }
        if (accel_y > threshold) {
            up = true;
        } else if (accel_y < -threshold) {
            down = true;
        }

        if (up) ESP_LOGI(TAG, "UP");
        if (down) ESP_LOGI(TAG, "DOWN");
        if (left) ESP_LOGI(TAG, "LEFT");
        if (right) ESP_LOGI(TAG, "RIGHT");

        vTaskDelay(500 / portTICK_RATE_MS); // Adjust the delay according to your needs
    }
}
