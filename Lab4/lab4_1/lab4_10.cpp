#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

// I2C configuration
#define I2C_MASTER_SCL_IO 22      /*!< GPIO number for I2C master clock */
#define I2C_MASTER_SDA_IO 21      /*!< GPIO number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define ICM42670_ADDR 0x68         /*!< I2C address for ICM-42670-P */
#define WHO_AM_I_REG 0x75          /*!< WHO_AM_I register */
#define ACCEL_XOUT_H 0x2D          /*!< ACCEL_XOUT_H register */
#define ACCEL_CONFIG 0x1C          /*!< ACCEL_CONFIG register */
#define PWR_MGMT_1 0x6B            /*!< Power Management 1 register */

static const char *TAG = "ICM42670";

// Function to initialize I2C
esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Function to write to ICM42670
esp_err_t write_register(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Function to read from ICM42670
esp_err_t read_register(uint8_t reg, uint8_t *data, size_t length) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    if (length > 1) {
        i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + length - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Function to initialize the ICM42670 sensor
void icm42670_init() {
    // Wake up the sensor
    write_register(PWR_MGMT_1, 0x00);

    // Configure the accelerometer
    write_register(ACCEL_CONFIG, 0x00); // Set accelerometer range to ±2g
}

// Function to read accelerometer data
void read_accelerometer(int16_t *accelX, int16_t *accelY, int16_t *accelZ) {
    uint8_t data[6];
    read_register(ACCEL_XOUT_H, data, 6);
    *accelX = ((int16_t)data[0] << 8) | data[1];
    *accelY = ((int16_t)data[2] << 8) | data[3];
    *accelZ = ((int16_t)data[4] << 8) | data[5];
}

// Function to determine direction based on accelerometer data
void determine_direction(int16_t accelX, int16_t accelY) {
    bool isUp = accelY > 10000;
    bool isDown = accelY < -10000;
    bool isLeft = accelX < -10000;
    bool isRight = accelX > 10000;

    if (isUp) {
        ESP_LOGI(TAG, "UP");
    }
    if (isDown) {
        ESP_LOGI(TAG, "DOWN");
    }
    if (isLeft) {
        ESP_LOGI(TAG, "LEFT");
    }
    if (isRight) {
        ESP_LOGI(TAG, "RIGHT");
    }
}

// Main task
void icm42670_task(void *pvParameter) {
    icm42670_init();

    while (1) {
        int16_t accelX, accelY, accelZ;
        read_accelerometer(&accelX, &accelY, &accelZ);
        determine_direction(accelX, accelY);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Read every second
    }
}

void app_main() {
    ESP_ERROR_CHECK(i2c_master_init());
    xTaskCreate(&icm42670_task, "icm42670_task", 2048, NULL, 5, NULL);
}
