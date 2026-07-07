#include "config.h"
#include "regs.h"
#include <stdint.h>

#define MPU_6050_ADDR 0x68
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

#include "i2c_imu.h"

void init_i2c_imu(void) {
  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SDA_IO], hw_xor) =
      (1 << 3) | (1 << 2); // xor the pullup/pulldown bits, at reset its pulled
                           // down after xor it becomes pull up enabled

  ACCESS_OFFSET(PADS_BANK0->GPIO[I2C0_SCL_IO], hw_xor) = (1 << 3) | (1 << 2);

  I2C0->TAR = MPU_6050_ADDR;
  I2C0->FS_SCL_HCNT = 100;
  I2C0->FS_SCL_LCNT = 150;
  I2C0->FS_SPKLEN = 9;
  I2C0->SDA_HOLD = 31;
  I2C0->ENABLE = 1;

  // now we try talking to mpu6050, but since it might take a while to finish
  // setting up, we delay till its ready
  for (uint32_t i = 0; i < 20; i++) {
    I2C0->DATA_CMD = PWR_MGMT_1;
    uint32_t time = TIMER->TIMERAWL;
    while (TIMER->TIMERAWL - time < 1 * 1000)
      ;
    if (!I2C0->TX_ABRT_SOURCE)
      break;
    (void)I2C0->CLR_TX_ABRT;
  }
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = GYRO_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
  I2C0->DATA_CMD = ACCEL_CONFIG;
  I2C0->DATA_CMD = (0x00) | (1 << 9);
}

enum imu_error read_imu(imu_readings *imu_struct) {
  uint32_t start = TIMER->TIMERAWL;
  uint32_t timeout = 1000;

  I2C0->DATA_CMD = ACCEL_XOUT_H;

  while (I2C0->TXFLR > (16 - 14))
    if (TIMER->TIMERAWL - start > timeout) {
      (void)I2C0->CLR_TX_ABRT;
      return IMU_TIMEOUT;
    }

  for (uint8_t i = 0; i < 13; i++)
    I2C0->DATA_CMD = (1 << 8);
  I2C0->DATA_CMD = (1 << 8) | (1 << 9);

  // SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
  while (I2C0->RXFLR < 14)
    if (TIMER->TIMERAWL - start > timeout) {
      (void)I2C0->CLR_TX_ABRT;
      return IMU_TIMEOUT;
    }

  uint8_t *ptr =
      (uint8_t *)imu_struct + sizeof(*imu_struct); // end of imu_struct

  while (ptr != (uint8_t *)imu_struct)
    *--ptr = (uint8_t)I2C0->DATA_CMD; // write the struct in reverse order (to
                                      // account for big endiannes of imu data)

  return IMU_OK;
}
