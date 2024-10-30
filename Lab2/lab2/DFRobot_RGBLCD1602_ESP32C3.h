#ifndef DFRobot_RGBLCD1602_H
#define DFRobot_RGBLCD1602_H

#include <stdint.h>
#include "driver/i2c.h" // Include ESP-IDF I2C driver

// Define LCD commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// Function Set flags
#define LCD_4BITMODE       0x00
#define LCD_2LINE          0x08
#define LCD_5x8DOTS       0x00
#define LCD_5x10DOTS      0x04

// Display Control flags
#define LCD_DISPLAYON      0x04
#define LCD_CURSORON       0x02
#define LCD_BLINKON        0x01

// Entry Mode flags
#define LCD_ENTRYLEFT      0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01

// Shift flags
#define LCD_MOVELEFT       0x00
#define LCD_MOVERIGHT      0x04

// Class definition
class DFRobot_RGBLCD1602 {
public:
    DFRobot_RGBLCD1602(uint8_t RGBAddr, uint8_t lcdCols, uint8_t lcdRows, i2c_port_t i2cPort, uint8_t lcdAddr);
    
    void init();
    void clear();
    void home();
    void noDisplay();
    void display();
    void stopBlink();
    void blink();
    void noCursor();
    void cursor();
    void scrollDisplayLeft();
    void scrollDisplayRight();
    void leftToRight();
    void rightToLeft();
    void noAutoscroll();
    void autoscroll();
    void customSymbol(uint8_t location, uint8_t charmap[]);
    void setCursor(uint8_t col, uint8_t row);
    void setRGB(uint8_t r, uint8_t g, uint8_t b);
    void setColor(uint8_t color);
    void setBacklight(bool mode);
    
    size_t print(const char* str); // New print method

private:
    void begin(uint8_t rows, uint8_t charSize);
    void send(uint8_t *data, uint8_t len);
    void command(uint8_t value);
    inline size_t write(uint8_t value);
    void setReg(uint8_t addr, uint8_t data);
    
    // Member variables
    uint8_t _lcdAddr;
    uint8_t _RGBAddr;
    uint8_t _cols;
    uint8_t _rows;
    i2c_port_t _i2cPort;
    uint8_t _showFunction;
    uint8_t _showControl;
    uint8_t _showMode;
    uint8_t _numLines;
    uint8_t _currLine;
    
    // RGB Control Register Addresses
    uint8_t REG_RED;
    uint8_t REG_GREEN;
    uint8_t REG_BLUE;
    uint8_t REG_ONLY;
};

#endif // DFRobot_RGBLCD1602_H
