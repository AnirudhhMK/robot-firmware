#include "config.h"
#include "multicore.h"
// #include "nRF.h"
#include "config.h"
#include "regs.h"
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

  SIO->GPIO_OE_SET = (1 << DEBUG_LED_IO) | (1 << RMOTOR_OUT_F_IO) |
                     (1 << RMOTOR_OUT_B_IO) | (1 << LMOTOR_OUT_F_IO) |
                     (1 << LMOTOR_OUT_B_IO);
  launch_core1();
  pwm_motor_init();
  int16_t avg = 0;
  uint32_t time = TIMER->TIMERAWL;
  uint8_t s = 1;
  while (1) {
    while (TIMER->TIMERAWL - time < 500)
      ;
    time += 500;
    movement_control(avg, 0);
    if (s)
      avg += 1;
    else
      avg -= 1;
    if (avg > 5000 || avg < -5000)
      s ^= 1;
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
  int32_t clip = 5001;
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
