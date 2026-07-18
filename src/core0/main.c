#include "config.h"
#include "multicore.h"
// #include "nRF.h"
#include "config.h"
#include "i2c_imu.h"
#include "math.h"
#include "regs.h"
#include "uart_comms.h"
#include <stdint.h>

void main_core1(void);

void pwm_motor_init(void);

void movement_control(int16_t avg, int16_t turn);

int main() {
  // pull all peripherals out of reset
  uint32_t reset_mask = (1 << 1) |  // busctrl
                        (1 << 2) |  // DMA
                        (1 << 3) |  // I2C0
                        (1 << 5) |  // IOBank0
                        (1 << 14) | // PWM
                        (1 << 16) | // SPI0
                        (1 << 21) | // timer
                        (1 << 22);  // UART0

  RESETS_CLR->RESET = reset_mask;
  while (!(RESETS->RESET_DONE & reset_mask))
    ;

  // we can give core0 priority in busctrl

  IO_BANK0->GPIO[DEBUG_LED_IO].CTRL = 5; // SIO
  IO_BANK0->GPIO[RMOTOR_OUT_F_IO].CTRL = 5;
  IO_BANK0->GPIO[RMOTOR_OUT_B_IO].CTRL = 5;
  IO_BANK0->GPIO[LMOTOR_OUT_F_IO].CTRL = 5;
  IO_BANK0->GPIO[LMOTOR_OUT_B_IO].CTRL = 5;

  IO_BANK0->GPIO[RMOTOR_OUT_PWM_IO].CTRL = 4; // PWM
  IO_BANK0->GPIO[LMOTOR_OUT_PWM_IO].CTRL = 4;

  IO_BANK0->GPIO[I2C0_SCL_IO].CTRL = 3; // I2C
  IO_BANK0->GPIO[I2C0_SDA_IO].CTRL = 3;

  IO_BANK0->GPIO[UART0_TX_IO].CTRL = 2;
  IO_BANK0->GPIO[UART0_RX_IO].CTRL = 2;

  SIO->GPIO_OE_SET = (1 << DEBUG_LED_IO) | (1 << RMOTOR_OUT_F_IO) |
                     (1 << RMOTOR_OUT_B_IO) | (1 << LMOTOR_OUT_F_IO) |
                     (1 << LMOTOR_OUT_B_IO);
  launch_core1();

  q16_16_t theta_g = 0, theta_a = 0, theta = 0;
  q16_16_t dtheta = 0;

  init_i2c();
  while (init_imu() != IMU_OK)
    (void)I2C0->CLR_TX_ABRT;
  calibrate_imu();

  //  SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
  uint64_t magic = 275028984878; // dt/131 * (2^55)
                                 // dt = 1/1000
  uint32_t x = 0;
  uint8_t first_iter = 1;
  while (1) {
    uint32_t start = TIMER->TIMERAWL;
    imu_readings *imu_data = &imu_payload_dbuf.buf[imu_payload_dbuf.writer];
    if (read_imu(imu_data) == IMU_OK) {
      x++;
      imu_payload_dbuf.writer ^= 1;
      if (x > 100) {
        SIO->GPIO_OUT_XOR = (1 << DEBUG_LED_IO);
        x = 0;
      }
      theta_a =
          arctan((q16_16_t)imu_data->accel_z, (q16_16_t)imu_data->accel_x);
      if (first_iter) {
        first_iter = 0;
        theta_g = theta_a;
      } else {
        dtheta = ((int64_t)magic * (int64_t)imu_data->gyro_y) >> (55 - 16);
        theta_g += dtheta;
      }
      theta = ((int64_t)65405 * (int64_t)(theta + dtheta) +
               (int64_t)131 * (int64_t)theta_a) >>
              16; // 0.002 in q16_16_t is 131
                  // 0.998 in q16_16_t is 65405
      angle_estimate_dbuf.buf[angle_estimate_dbuf.writer] =
          (angle_estimate_payload_t){theta_a, theta_g, theta};
      angle_estimate_dbuf.writer ^= 1;
    }

    (void)I2C0->CLR_TX_ABRT;
    uint32_t time = TIMER->TIMERAWL;
    uint32_t loop_time = time - start;
    uint32_t *min =
        &loop_time_payload_dbuf.buf[loop_time_payload_dbuf.writer].min_time;
    uint32_t *max =
        &loop_time_payload_dbuf.buf[loop_time_payload_dbuf.writer].max_time;
    if (*min > loop_time || *min == 0)
      *min = loop_time;
    if (*max < loop_time)
      *max = loop_time;
    loop_time_payload_dbuf.writer ^= 1;
    while (TIMER->TIMERAWL - start < 1000)
      ;
  }
}

void pwm_motor_init(void) {
  // assuming PWM is out of reset
  PWM->CH[MOTOR_PWM_SLICE].TOP = 5000; // freq = 20kHz
  PWM->CH[MOTOR_PWM_SLICE].CSR = 1;
}

void movement_control(int16_t avg, int16_t turn) {
  int32_t rmotor = avg - turn;
  int32_t lmotor = avg + turn;
  int32_t clip = PWM->CH[MOTOR_PWM_SLICE].TOP + 1;
  if (rmotor > 0) {
    SIO->GPIO_OUT_SET = (1 << RMOTOR_OUT_F_IO);
    SIO->GPIO_OUT_CLR = (1 << RMOTOR_OUT_B_IO);
  } else {
    SIO->GPIO_OUT_SET = (1 << RMOTOR_OUT_B_IO);
    SIO->GPIO_OUT_CLR = (1 << RMOTOR_OUT_F_IO);
    rmotor = -rmotor;
  }
  if (rmotor > clip) {
    rmotor = clip;
  }

  if (lmotor > 0) {
    SIO->GPIO_OUT_SET = (1 << LMOTOR_OUT_F_IO);
    SIO->GPIO_OUT_CLR = (1 << LMOTOR_OUT_B_IO);
  } else {
    SIO->GPIO_OUT_SET = (1 << LMOTOR_OUT_B_IO);
    SIO->GPIO_OUT_CLR = (1 << LMOTOR_OUT_F_IO);
    lmotor = -lmotor;
  }
  if (lmotor > clip) {
    lmotor = clip;
  }

  PWM->CH[MOTOR_PWM_SLICE].CC =
      (rmotor << (16 * RMOTOR_PWM_CH)) | (lmotor << (16 * LMOTOR_PWM_CH));

  pwm_data_dbuf.buf[pwm_data_dbuf.writer] = (pwm_data){lmotor, rmotor};
  pwm_data_dbuf.writer ^= 1; // swap
}

void spi0_init() {
  SPI0->SSPCPSR = 50; // cpsdvsr
  SPI0->SSPCR0 =
      (7 | (0 << 6) | (1 << 8)); //  8-bit data size, Motorala mode 0,scr=1

  SPI0->SSPCR1 = (1 << 1); // master mode, enable ssp
}
