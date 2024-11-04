#include "DFRobot_RGBLCD1602.cpp"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>


#define I2C_PORT I2C_NUM0
#define LCD_ADDR LCD_ADDRESS
#define RGB_ADDR RGB_ADDRESS


// Configure I2C par
//
//
//
extern "C" void app_main(void) {
    // Initialize I2C master
    //i2c_master_init();

    // Initialize the LCD object with addresses, 16 columns, and 2 rows
    DFRobot_LCD lcd(16, 2, LCD_ADDR, RGB_ADDR);

    // Initialize the LCD display and set up
    lcd.init();
    lcd.setRGB(0, 255, 0);

    while (true) {
        // Clear the display
        lcd.clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

        // Display "Hello CSE121!" on the first line
        lcd.setCursor(0, 0);
        lcd.printstr("Hello CSE121!");
	vTaskDelay(100 / portTICK_PERIOD_MS);

        // Display last name "Canulito" on the second line
        lcd.setCursor(0, 1);
        lcd.printstr("Canulito");

        // Optional: add delay or manage loop frequency
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
