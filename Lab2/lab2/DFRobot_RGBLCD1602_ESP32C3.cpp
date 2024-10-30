/*
 * @file DFRobot_RGBLCD1602.cpp
 * @brief DFRobot_RGBLCD1602 class infrastructure, modified for ESP32C3 using ESP-IDF
 */

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "DFRobot_RGBLCD1602.h"

// RGB color definitions
const uint8_t color_define[4][3] = {
    {255, 255, 255}, // white
    {255, 0, 0},     // red
    {0, 255, 0},     // green
    {0, 0, 255}      // blue
};

// Constructor
DFRobot_RGBLCD1602::DFRobot_RGBLCD1602(uint8_t RGBAddr, uint8_t lcdCols, uint8_t lcdRows, uint8_t lcdAddr) {
    _lcdAddr = lcdAddr;
    _RGBAddr = RGBAddr;
    _cols = lcdCols;
    _rows = lcdRows;
}

// I2C initialization for ESP32C3
void DFRobot_RGBLCD1602::init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)21; // Set to correct SDA GPIO
    conf.scl_io_num = (gpio_num_t)22; // Set to correct SCL GPIO
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    if (_RGBAddr == 0x60) {
        REG_RED = 0x04;
        REG_GREEN = 0x03;
        REG_BLUE = 0x02;
    } else if (_RGBAddr == (0x60 >> 1)) {
        REG_RED = 0x06;
        REG_GREEN = 0x07;
        REG_BLUE = 0x08;
    } else if (_RGBAddr == 0x6B) {
        REG_RED = 0x06;
        REG_GREEN = 0x05;
        REG_BLUE = 0x04;
    }
}

// Write byte to I2C
esp_err_t DFRobot_RGBLCD1602::i2cWriteByte(uint8_t addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Set RGB color
void DFRobot_RGBLCD1602::setRGB(uint8_t r, uint8_t g, uint8_t b) {
    i2cWriteByte(_RGBAddr, REG_RED);
    i2cWriteByte(_RGBAddr, r);
    i2cWriteByte(_RGBAddr, REG_GREEN);
    i2cWriteByte(_RGBAddr, g);
    i2cWriteByte(_RGBAddr, REG_BLUE);
    i2cWriteByte(_RGBAddr, b);
}

// Delay function for ESP32C3
void DFRobot_RGBLCD1602::delayMS(int ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}
