#ifndef SHTC3_H
#define SHTC3_H

#include "driver/i2c.h"

// Define SHTC3 I2C address and GPIO pins
#define SHTC3_ADDR                 0x70     // SHTC3 I2C address
#define SHTC3_WAKEUP_CMD           0x3517
#define SHTC3_MEASURE_CMD          0x7CA2
#define SHTC3_SLEEP_CMD            0xB098

// Function declarations
esp_err_t i2c_master_init(void);
void read_shtc3(float *temperature, float *humidity);

#endif // SHTC3_H






/*
#ifndef SHTC3_SENSOR_H
#define SHTC3_SENSOR_H

#include "driver/i2c.h"
#include "esp_err.h"

// I2C configuration
#define SHTC3_ADDR 0x70
#define SHTC3_WAKEUP_CMD 0x3517
#define SHTC3_MEASUREMENT_CMD 0x7CA2
#define SHTC3_SLEEP_CMD 0xB098

// Function Prototypes
esp_err_t shtc3_init(i2c_port_t i2c_num);
void shtc3_wake_up(i2c_port_t i2c_num);
void shtc3_sleep(i2c_port_t i2c_num);
void shtc3_read(i2c_port_t i2c_num, float *temperature, float *humidity);

#endif // SHTC3_SENSOR_H
*/
