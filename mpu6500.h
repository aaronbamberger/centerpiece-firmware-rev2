/* 
 * File:   mpu6500.h
 * Author: Aaron
 *
 * Created on April 10, 2016, 9:17 PM
 */

#ifndef MPU6500_H
#define	MPU6500_H

// Read/Write
#define REG_READ (1 << 7)
#define REG_WRITE 0

// Registers
#define SMPLRT_DIV_REG 25
#define CONFIG_REG 26
#define GYRO_CONFIG_REG 27
#define INT_CFG_REG 55
#define INT_ENABLE_REG 56
#define GYRO_XOUT_H_REG 67
#define GYRO_XOUT_L_REG 68
#define GYRO_YOUT_H_REG 69
#define GYRO_YOUT_L_REG 70
#define GYRO_ZOUT_H_REG 71
#define GYRO_ZOUT_L_REG 72
#define SIGNAL_PATH_RESET_REG 104
#define USER_CTRL_REG 106
#define PWR_MGMT_1_REG 107
#define PWR_MGMT_2_REG 108
#define WHO_AM_I_REG 117

// Register bits
// Config bits
#define DLPF_CFG_250_HZ 0
#define DLPF_CFG_184_HZ 1
#define DLPF_CFG_92_HZ 2
#define DLPF_CFG_41_HZ 3
#define DLPF_CFG_20_HZ 4
#define DLPF_CFG_10_HZ 5
#define DLPF_CFG_5_HZ 6
#define DLPF_CFG_3600_HZ 7

// Gyro config bits
#define GYRO_FS_SEL_250_DPS (0 << 3)
#define GYRO_FS_SEL_500_DPS (1 << 3)
#define GYRO_FS_SEL_1000_DPS (2 << 3)
#define GYRO_FS_SEL_2000_DPS (3 << 3)

// Int config bits
#define INT_ACTIVE_LOW (1 << 7)
#define INT_ACTIVE_HIGH 0
#define INT_OPEN_DRAIN (1 << 6)
#define INT_PUSH_PULL 0
#define INT_LATCH (1 << 5)
#define INT_PULSE 0
#define INT_CLEAR_ANY (1 << 4)
#define INT_CLEAR_STATUS 0

// Int enable bits
#define WOM_EN (1 << 6)
#define FIFO_OVERFLOW_EN (1 << 4)
#define FSYNC_INT_EN (1 << 3)
#define RAW_RDY_EN (1 << 0)

// Signal path reset bits
#define GYRO_RST (1 << 2)
#define ACCEL_RST (1 << 1)
#define TEMP_RST (1 << 0)

// User Ctrl Register
#define DMP_EN (1 << 7)
#define FIFO_EN (1 << 6)
#define I2C_MST_EN (1 << 5)
#define I2C_IF_DIS (1 << 4)
#define DMP_RST (1 << 3)
#define FIFO_RST (1 << 2)
#define I2C_MST_RST (1 << 1)
#define SIG_COND_RST (1 << 0)

// Power management 1 register
#define DEVICE_RESET (1 << 7)
#define SLEEP_BIT (1 << 6)
#define CYCLE (1 << 5)
#define GYRO_STANDBY (1 << 4)
#define TEMP_DIS (1 << 3)
#define CLKSEL_INTERNAL 0
#define CLKSEL_AUTO 1
#define CLKSEL_RESET 7

// Power management 2 register
#define DISABLE_XA (1 << 5)
#define DISABLE_YA (1 << 4)
#define DISABLE_ZA (1 << 3)
#define DISABLE_XG (1 << 2)
#define DISABLE_YG (1 << 1)
#define DISABLE_ZG (1 << 0)

#ifdef	__cplusplus
extern "C" {
#endif

void init_mpu();
void start_read_gyro_z();

#ifdef	__cplusplus
}
#endif

#endif	/* MPU6500_H */

