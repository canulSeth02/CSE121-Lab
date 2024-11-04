#include "DFRobot_RGBLCD1602_ESP32C3.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "temphumread.c"

#define I2C_PORT I2C_NUM0
#define LCD_ADDR LCD_ADDRESS
#define RGB_ADDR RGB_ADDRESS

extern "C" void app_main(void) {
    // Initialize I2C master

    // Initialize the LCD object with addresses, 16 columns, and 2 rows
    DFRobot_LCD lcd(16, 2, LCD_ADDR, RGB_ADDR);

    // Initialize the LCD display and set up
    lcd.init();
    lcd.setRGB(255, 0, 0);

    while (true) {

	shtc3_wake_up();

        // Variables to hold temperature and humidity
        float temperature = 0.0, humidity = 0.0;
        
        // Read temperature and humidity from sensor
        shtc3_read(&temperature, &humidity);

        // Clear the display
        lcd.clear();
        vTaskDelay(100 / portTICK_PERIOD_MS);

        // Display temperature on the first line
        lcd.setCursor(0, 0);
        char temp_str[15];
        snprintf(temp_str, sizeof(temp_str), "Temp: %.0f C", temperature);
        lcd.printstr(temp_str);

        // Display humidity on the second line
        lcd.setCursor(0, 1);
        char hum_str[15];
        snprintf(hum_str, sizeof(hum_str), "Humidity: %.0f%%", humidity);
        lcd.printstr(hum_str);

        // Delay before updating the readings
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
