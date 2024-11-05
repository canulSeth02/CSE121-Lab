#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO    8    // GPIO number for I2C SCL
#define I2C_MASTER_SDA_IO    10    // GPIO number for I2C SDA
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000
#define PWR_MGMT0            0x1F
#define ICM42670_ADDR        0x68  // I2C address when AD0 is low

#define ACCEL_CONFIG0        0x21
#define ACCEL_DATA_X1        0x0B  // Register address for ACCEL_X high byte
#define ACCEL_DATA_X0        0x0C  // Register address for ACCEL_X low byte
#define ACCEL_DATA_Y1        0x0D  // Register address for ACCEL_Y high byte
#define ACCEL_DATA_Y0        0x0E  // Register address for ACCEL_Y low byte

#define THRESHOLD            10000  // Set an appropriate threshold for tilt sensitivity



const static char *TAG = "ACCELEROMETER";

// Initialize I2C
static esp_err_t i2c_master_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Write to a register
static esp_err_t write_register(uint8_t reg_addr, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Read 16-bit value from two consecutive registers (high and low bytes)
esp_err_t read_accel_data(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z) {
    uint8_t data[6];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Read high byte
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    //write to accel data 
    i2c_master_write_byte(cmd, ACCEL_DATA_X1, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 6, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if(ret == ESP_OK){

        *accel_x = ((data[0] << 8) | data[1]);
        *accel_y = ((data[2] << 8) | data[3]);
        *accel_z = ((data[4] << 8) | data[5]);
    }
    else{
        ESP_LOGE(TAG, "Shits fuckin up");
    }
    
    // Read low byte
    // cmd = i2c_cmd_link_create();
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_WRITE, true);
    // i2c_master_write_byte(cmd, low_reg_addr, true);
    // i2c_master_start(cmd);
    // i2c_master_write_byte(cmd, (ICM42670_ADDR << 1) | I2C_MASTER_READ, true);
    // i2c_master_read_byte(cmd, &data_l, I2C_MASTER_NACK);
    // i2c_master_stop(cmd);
    // i2c_cmd_link_delete(cmd);

    return ret;
}

// Configure the ICM-42670-P to enable the accelerometer and set the full-scale range
static void configure_icm42670() {
    // Set accelerometer to low-noise mode
    write_register(PWR_MGMT0, 0b00000011); // Enable accelerometer in low-noise mode
    
    // Set accelerometer full-scale range to ±4g
    vTaskDelay(pdMS_TO_TICKS(200));
    write_register(ACCEL_CONFIG0, 0b1111);  // Full scale: ±4g (ACCEL_UI_FS_SEL=2)
}

// Main function to check tilt and log direction
extern "C" void app_main() {
    ESP_ERROR_CHECK(i2c_master_init());
    configure_icm42670();

   int16_t accel_x = 0;
   int16_t accel_y = 0;
   int16_t accel_z = 0;



    while (true) {
        
        if(read_accel_data(&accel_x, &accel_y, &accel_z) == ESP_OK)
        {
            bool is_up = accel_y > THRESHOLD;
            bool is_down = accel_y < -THRESHOLD;
            bool is_right = accel_x > THRESHOLD;
            bool is_left = accel_x < -THRESHOLD;

            // Log based on tilt
            if (is_up && is_left) {
                ESP_LOGI("Tilt Direction", "UP LEFT");
            } else if (is_up && is_right) {
                ESP_LOGI("Tilt Direction", "UP RIGHT");
            } else if (is_down && is_left) {
                ESP_LOGI("Tilt Direction", "DOWN LEFT");
            } else if (is_down && is_right) {
                ESP_LOGI("Tilt Direction", "DOWN RIGHT");
            } else if (is_up) {
                ESP_LOGI("Tilt Direction", "UP");
            } else if (is_down) {
                ESP_LOGI("Tilt Direction", "DOWN");
            } else if (is_left) {
                ESP_LOGI("Tilt Direction", "LEFT");
            } else if (is_right) {
                ESP_LOGI("Tilt Direction", "RIGHT");
            } else {
                ESP_LOGI("Tilt Direction", "No significant tilt");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Adjust delay as needed
    }
}
