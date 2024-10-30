#include "DFRobot_LCD.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

// Configure I2C parameters (assuming default I2C port and pins, please adjust if needed)
#define I2C_PORT I2C_NUM_0
#define LCD_ADDR LCD_ADDRESS
#define RGB_ADDR RGB_ADDRESS

// I2C configuration
#define I2C_MASTER_SCL_IO 22    /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21    /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_PORT /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

// Function to initialize I2C
static void i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

extern "C" void app_main(void) {
    // Initialize I2C master
    i2c_master_init();

    // Initialize the LCD object with addresses, 16 columns, and 2 rows
    DFRobot_LCD lcd(16, 2, LCD_ADDR, RGB_ADDR);

    // Initialize the LCD display and set up
    lcd.init();

    while (true) {
        // Clear the display
        lcd.clear();

        // Display "Hello CSE121!" on the first line
        lcd.setCursor(0, 0);
        lcd.printstr("Hello CSE121!");

        // Display last name "Canulito" on the second line
        lcd.setCursor(0, 1);
        lcd.printstr("Canulito");

        // Optional: add delay or manage loop frequency
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
