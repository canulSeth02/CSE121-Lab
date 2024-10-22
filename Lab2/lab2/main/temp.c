#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "temphumread.h"

#define I2C_MASTER_SCL_IO 8    // SCL pin
#define I2C_MASTER_SDA_IO 10   // SDA pin
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define SHTC3_ADDR 0x70

#define SHTC3_WAKEUP_CMD 0x3517
#define SHTC3_MEASUREMENT_CMD 0x7CA2
#define SHTC3_SLEEP_CMD 0xB098

static const char *TAG = "SHTC3";

esp_err_t i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    return i2c_param_config(I2C_MASTER_NUM, &conf) || i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

static void shtc3_wake_up() {
    uint8_t cmd[2];
    cmd[0] = (SHTC3_WAKEUP_CMD >> 8) & 0xFF;
    cmd[1] = SHTC3_WAKEUP_CMD & 0xFF;
    i2c_master_write_to_device(I2C_MASTER_NUM, SHTC3_ADDR, cmd, sizeof(cmd), 1000 / portTICK_PERIOD_MS);
}

static void shtc3_sleep() {
    uint8_t cmd[2];
    cmd[0] = (SHTC3_SLEEP_CMD >> 8) & 0xFF;
    cmd[1] = SHTC3_SLEEP_CMD & 0xFF;
    i2c_master_write_to_device(I2C_MASTER_NUM, SHTC3_ADDR, cmd, sizeof(cmd), 1000 / portTICK_PERIOD_MS);
}

static void shtc3_read(float *temperature, float *humidity) {
    uint8_t measurement_cmd[2] = {
        (SHTC3_MEASUREMENT_CMD >> 8) & 0xFF,
        SHTC3_MEASUREMENT_CMD & 0xFF
    };
    i2c_master_write_to_device(I2C_MASTER_NUM, SHTC3_ADDR, measurement_cmd, sizeof(measurement_cmd), 1000 / portTICK_PERIOD_MS);

    vTaskDelay(30 / portTICK_PERIOD_MS); // Wait for measurement

    uint8_t data[6];
    i2c_master_read_from_device(I2C_MASTER_NUM, SHTC3_ADDR, data, sizeof(data), 1000 / portTICK_PERIOD_MS);

    uint16_t temp_data = (data[0] << 8) | data[1];
    uint16_t hum_data = (data[3] << 8) | data[4];

    // Convert raw data to temperature and humidity
    *temperature = (float)((temp_data * 175) / 65536.0) - 45;
    *humidity = (float)(hum_data * 100) / 65536.0;
}

void app_main() {
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed");
        return;
    }

    while (1) {
        float temperature, humidity;
        shtc3_wake_up();
        shtc3_read(&temperature, &humidity);
        shtc3_sleep();

        // Convert temperature to Fahrenheit
        float temperature_f = (temperature * 9.0 / 5.0) + 32.0;

        ESP_LOGI(TAG, "Temperature is %.0fC (or %.0fF) with a %.0f%% humidity", temperature, temperature_f, humidity);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Wait for 2 seconds
    }
}
