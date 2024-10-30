#include "DFRobot_RGBLCD1602.h"
#include "driver/i2c.h"
#include <stdio.h>

// Configure I2C parameters (assuming default I2C port and pins, please adjust if needed)
#define I2C_PORT I2C_NUM_0
#define LCD_ADDR LCD_ADDRESS
#define RGB_ADDR 0x62  // Example RGB address; adjust as necessary

extern "C" void app_main(void) {
    // Initialize the LCD object with addresses, 16 columns, and 2 rows
    DFRobot_RGBLCD1602 lcd(RGB_ADDR, 16, 2, LCD_ADDR);

    while (true) {
        // Initialize the LCD display and set up
        lcd.init();
        
        // Display "Hello CSE121!" on the first line
        lcd.setCursor(0, 0);
        lcd.printstr("Hello CSE121!");
        
        // Display last name "Renau" on the second line
        lcd.setCursor(0, 1);
        lcd.printstr("Canulito");

        // Optional: add delay or manage loop frequency
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
