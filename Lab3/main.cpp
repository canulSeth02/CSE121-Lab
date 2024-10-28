#include <Arduino.h>
#include "DFRobot_RGBLCD1602.h"

// Create an instance of the DFRobot_RGBLCD1602 class
DFRobot_RGBLCD1602 lcd(0x60, 16, 2, 0x27);  // RGB address, columns, rows, LCD address

void setup() {
    lcd.init();  // Initialize the LCD

    // Set the cursor to the first line and print the first message
    lcd.setCursor(0, 0);  
    lcd.print("Hello CSE121!");  // Print on the first line

    // Set the cursor to the second line and print the last name
    lcd.setCursor(0, 1);  
    lcd.print("YourLastName");    // Print on the second line
}

