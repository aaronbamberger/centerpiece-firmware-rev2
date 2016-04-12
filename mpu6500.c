#include "mpu6500.h"
#include "spi.h"

#define _XTAL_FREQ 8000000

#include <stdint.h>
#include <string.h>
#include <xc.h>

void init_mpu()
{
    uint8_t send_buf[2];
    uint8_t recv_buf[2];
    
    memset(send_buf, 0x00, 2);
    memset(recv_buf, 0x00, 2);
    
    BlockingSPITransaction transaction;
    transaction.send_buf = send_buf;
    transaction.recv_buf = recv_buf;
    transaction.recv_count = 0;
    
    // Disable the I2C interface
    send_buf[0] = USER_CTRL_REG | REG_WRITE;
    send_buf[1] = 0;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Reset sequence per note on page 42 of MPU-6500 register map
    
    // Reset the device
    send_buf[0] = PWR_MGMT_1_REG | REG_WRITE;
    send_buf[1] = DEVICE_RESET;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    __delay_ms(100);
    
    // Reset the signal path
    send_buf[0] = SIGNAL_PATH_RESET_REG | REG_WRITE;
    send_buf[1] = GYRO_RST | ACCEL_RST | TEMP_RST;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    __delay_ms(100);
    
    // Disable the accelerometers, and X and Y gyros
    send_buf[0] = PWR_MGMT_2_REG | REG_WRITE;
    send_buf[1] = DISABLE_XA | DISABLE_YA | DISABLE_ZA | DISABLE_XG | DISABLE_YG;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Configure the low pass filter
    send_buf[0] = CONFIG_REG | REG_WRITE;
    send_buf[1] = DLPF_CFG_250_HZ;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Configure the gyro full scale range
    send_buf[0] = GYRO_CONFIG_REG | REG_WRITE;
    send_buf[1] = GYRO_FS_SEL_250_DPS;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Configure the sample rate divider
    send_buf[0] = SMPLRT_DIV_REG | REG_WRITE;
    send_buf[1] = 9; // 100Hz, Sample rate = 1kHz / 1 + SMPLRT_DIV
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Configure the gyro interrupt
    send_buf[0] = INT_CFG_REG | REG_WRITE;
    send_buf[1] = INT_ACTIVE_HIGH | INT_PUSH_PULL | INT_PULSE | INT_CLEAR_ANY;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Configure the interrupt enable
    send_buf[0] = INT_ENABLE_REG | REG_WRITE;
    send_buf[1] = RAW_RDY_EN;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    // Disable the temperature sensor, and select the gyro PLL as clock
    send_buf[0] = PWR_MGMT_1_REG | REG_WRITE;
    send_buf[1] = TEMP_DIS | CLKSEL_AUTO;
    transaction.send_count = 2;
    perform_blocking_spi_transaction(&transaction);
    
    /*
    // Read the who am I register
    send_buf[0] = WHO_AM_I_REG | REG_READ;
    send_buf[1] = 0;
    transaction.send_count = 2;
    transaction.recv_count = 2;
    perform_blocking_spi_transaction(&transaction);
    */
    
    return;
}

void start_read_gyro_z()
{
    send_spi_byte(GYRO_ZOUT_H_REG | REG_READ);
}
