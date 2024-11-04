/*!
 * @file DFRobot_LCD.cpp
 * @brief DFRobot's LCD
 * @n High Accuracy Ambient Light Sensor
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2016
 * @copyright	GNU Lesser General Public License
 *
 * @author [yangyang](971326313@qq.com)
 * @version  V1.0
 * @date  2017-2-10
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "DFRobot_RGBLCD1602_ESP32C3.h"

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 10
#define I2C_MASTER_SCL_IO 8
#define I2C_MASTER_FREQ_HZ 100000

const uint8_t color_define[4][3] = 
{
    {255, 255, 255},            // white
    {255, 0, 0},                // red
    {0, 255, 0},                // green
    {0, 0, 255},                // blue
};

/*******************************public*******************************/
DFRobot_LCD::DFRobot_LCD(uint8_t lcd_cols, uint8_t lcd_rows, uint8_t lcd_Addr, uint8_t RGB_Addr)
{
    _lcdAddr = lcd_Addr;
    _RGBAddr = RGB_Addr;
    _cols = lcd_cols;
    _rows = lcd_rows;
}

void i2c_master_init()
{
    i2c_driver_delete(I2C_MASTER_NUM);
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = 0;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void DFRobot_LCD::init()
{
    i2c_master_init();
    _showfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(_cols, _rows);
}

void DFRobot_LCD::clear()
{
    command(LCD_CLEARDISPLAY);        // clear display, set cursor position to zero
    vTaskDelay(pdMS_TO_TICKS(2));     // this command takes a long time!
}

void DFRobot_LCD::home()
{
    command(LCD_RETURNHOME);        // set cursor position to zero
    vTaskDelay(pdMS_TO_TICKS(2));   // this command takes a long time!
}

void DFRobot_LCD::noDisplay()
{
    _showcontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::display()
{
    _showcontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::stopBlink()
{
    _showcontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::blink()
{
    _showcontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::noCursor()
{
    _showcontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::cursor()
{
    _showcontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _showcontrol);
}

void DFRobot_LCD::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void DFRobot_LCD::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void DFRobot_LCD::leftToRight(void)
{
    _showmode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::rightToLeft(void)
{
    _showmode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::noAutoscroll(void)
{
    _showmode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::autoscroll(void)
{
    _showmode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _showmode);
}

void DFRobot_LCD::customSymbol(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    
    uint8_t data[9];
    data[0] = 0x40;
    for(int i = 0; i < 8; i++)
    {
        data[i+1] = charmap[i];
    }
    send(data, 9);
}

void DFRobot_LCD::setCursor(uint8_t col, uint8_t row)
{
    col = (row == 0 ? col|0x80 : col|0xc0);
    uint8_t data[3] = {0x80, col};

    send(data, 2);
}

void DFRobot_LCD::setRGB(uint8_t r, uint8_t g, uint8_t b)
{
    setReg(REG_RED, r);
    setReg(REG_GREEN, g);
    setReg(REG_BLUE, b);
}

void DFRobot_LCD::setColor(uint8_t color)
{
    if(color > 3) return;
    setRGB(color_define[color][0], color_define[color][1], color_define[color][2]);
}

void DFRobot_LCD::blinkLED(void)
{
    setReg(0x07, 0x17);  // blink every second
    setReg(0x06, 0x7f);  // half on, half off
}

void DFRobot_LCD::noBlinkLED(void)
{
    setReg(0x07, 0x00);
    setReg(0x06, 0xff);
}

inline size_t DFRobot_LCD::write(uint8_t value)
{
    uint8_t data[3] = {0x40, value};
    send(data, 2);
    return 1; // assume success
}

inline void DFRobot_LCD::command(uint8_t value)
{
    uint8_t data[3] = {0x80, value};
    send(data, 2);
}

void DFRobot_LCD::blink_on()
{
    blink();
}

void DFRobot_LCD::blink_off()
{
    stopBlink();
}

void DFRobot_LCD::cursor_on()
{
    cursor();
}

void DFRobot_LCD::cursor_off()
{
    noCursor();
}

void DFRobot_LCD::setBacklight(uint8_t new_val)
{
    if(new_val)
    {
        blinkLED();   // turn backlight on
    }
    else
    {
        noBlinkLED();  // turn backlight off
    }
}

void DFRobot_LCD::load_custom_character(uint8_t char_num, uint8_t *rows)
{
    customSymbol(char_num, rows);
}

void DFRobot_LCD::printstr(const char c[])
{
    for (size_t i = 0; i < strlen(c); i++) {
        write(c[i]); // Use the write method to send each character to the LCD
    }
}

/*******************************private*******************************/
void DFRobot_LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
    if (lines > 1) {
        _showfunction |= LCD_2LINE;
    }
    _numlines = lines;
    _currline = 0;

    if ((dotsize != 0) && (lines == 1)) {
        _showfunction |= LCD_5x10DOTS;
    }

    vTaskDelay(pdMS_TO_TICKS(50));

    command(LCD_FUNCTIONSET | _showfunction);
    vTaskDelay(pdMS_TO_TICKS(5));

    command(LCD_FUNCTIONSET | _showfunction);
    vTaskDelay(pdMS_TO_TICKS(5));

    command(LCD_FUNCTIONSET | _showfunction);

    _showcontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    clear();

    _showmode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    command(LCD_ENTRYMODESET | _showmode);

    setReg(REG_MODE1, 0);
    setReg(REG_OUTPUT, 0xFF);
    setReg(REG_MODE2, 0x20);
    
    setColorWhite();
}

void DFRobot_LCD::send(uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_lcdAddr << 1) | I2C_MASTER_WRITE, true);
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, data[i], true);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

void DFRobot_LCD::setReg(uint8_t addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (_RGBAddr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}


/************************unsupported API functions***************************/
void DFRobot_LCD::off(){}
void DFRobot_LCD::on(){}
void DFRobot_LCD::setDelay(int cmdDelay, int charDelay) {}
uint8_t DFRobot_LCD::status(){return 0;}
uint8_t DFRobot_LCD::keypad (){return 0;}
uint8_t DFRobot_LCD::init_bargraph(uint8_t graphtype){return 0;}
void DFRobot_LCD::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end){}
void DFRobot_LCD::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_row_end){}
void DFRobot_LCD::setContrast(uint8_t new_val){}
