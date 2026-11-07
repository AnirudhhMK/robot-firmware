#pragma once
// TODO:add cpp specific constexpr configs with config namespace
#include <stdint.h>
#define PLL_SYS_FBDIV
#define PLL_SYS_PD1
#define PLL_SYS_PD2

#define CLK_PERI_SRC 0

#define CE 6

#define DEBUG_LED_IO 25
#define MOTOR_PWM_SLICE 0
#define LMOTOR_PWM_CH 0
#define RMOTOR_PWM_CH 1

#define LMOTOR_OUT_PWM_IO 0
#define RMOTOR_OUT_PWM_IO 1

#define LMOTOR_OUT_F_IO 2
#define LMOTOR_OUT_B_IO 3

#define RMOTOR_OUT_F_IO 4
#define RMOTOR_OUT_B_IO 5

#define UART0_TX_IO 12
#define UART0_RX_IO 13

#define I2C0_SDA_IO 8
#define I2C0_SCL_IO 9

// #ifdef __cplusplus
#define STATIC_ASSERT static_assert
namespace config {
struct UartConfig { // TODO: Need to decide whether to go with a seperate
                    // constexpr struct for uart0 and uart1, or a general struct
                    // with default values that can be modified for uart1 and 0
                    // after initialization
  uint32_t uartibrd = 54;
  uint32_t uartfbrd = 25;

  uint32_t uartlcr_h = (1 << 1) | // enable parity check, default to odd parity
                       (1 << 4) | // enable fifo
                       (0b11 << 5); // 8 bit data size

  uint32_t uartcr = (1 << 0) | // uart enable
                    (1 << 8) | // rx enable
                    (1 << 9) | // tx enable
                    (0 << 7);  // no lpe

  uint32_t uartdmacr = (1 << 0) | // transmit dma enabled
                       (1 << 1);  // recieve dma enabled
  uint8_t dma_tx_ch = 0;
  uint8_t dma_rx_ch = 1;
};
} // namespace config
// #endif
