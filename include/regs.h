#pragma once

#define sio_base 0xd0000000 // SIO base 2.3.1.7

#define watchdog_base 0x40058000

#define timer_base 0x40054000

#define pio0_base 0x50200000

#define nvic_iser 0xe0000000 + 0xe100

#define ppb_offset_base 0xe0000000 + 0xe010

#define dma0_base 0x50000000

#define busctrl_base 0x40030000

#define clocks_base 0x40008000

#define io_bank0 0x40014000 // GPIO25_CTRL 2.19.6.1

#define rst_base 0x4000c000 // reset controller base 2.14.

#define xosc_base 0x40024000

#define spi0_base 0x4003C000
#define spi1_base 0x40040000

#define hw_xor 0x1000
#define hw_set 0x2000
#define hw_clr 0x3000
//------C only ---
#ifndef __ASSEMBLER__
#include <stdint.h>

typedef struct {
  volatile uint32_t SSPCR0;   // 0x000 Control Register 0
  volatile uint32_t SSPCR1;   // 0x004 Control Register 1
  volatile uint32_t SSPDR;    // 0x008 Data Register
  volatile uint32_t SSPSR;    // 0x00C Status Register
  volatile uint32_t SSPCPSR;  // 0x010 Clock Prescale
  volatile uint32_t SSPIMSC;  // 0x014 Interrupt Mask
  volatile uint32_t SSPRIS;   // 0x018 Raw Interrupt Status
  volatile uint32_t SSPMIS;   // 0x01C Masked Interrupt Status
  volatile uint32_t SSPICR;   // 0x020 Interrupt Clear
  volatile uint32_t SSPDMACR; // 0x024 DMA Control
} spi_hw_t;

#define SPI0 ((spi_hw_t *const)spi0_base)
#define SPI1 ((spi_hw_t *const)spi1_base)

typedef struct {
  volatile uint32_t RESET;
  volatile uint32_t WDSEL;
  volatile uint32_t RESET_DONE;
} resets_hw_t;

#define RESETS ((resets_hw_t *const)(rst_base))
#define RESETS_SET ((resets_hw_t *const)(rst_base + hw_set))
#define RESETS_CLR ((resets_hw_t *const)(rst_base + hw_clr))
#define RESETS_XOR ((resets_hw_t *const)(rst_base + hw_xor))

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t DIV;
  volatile uint32_t SELECTED;
} clk_hw_t;
typedef struct {
  clk_hw_t CLK_GPOUT[4];
  clk_hw_t CLK_REF;
  clk_hw_t CLK_SYS;
  clk_hw_t CLK_PERI;
  clk_hw_t CLK_USB;
  clk_hw_t CLK_ADC;
  clk_hw_t CLK_RTC;

  volatile uint32_t RESUS_CTRL;
  volatile uint32_t RESUS_STATUS;

  volatile uint32_t FC0_REF_KHZ;
  volatile uint32_t FC0_MIN_KHZ;
  volatile uint32_t FC0_MAX_KHZ;
  volatile uint32_t FC0_DELAY;
  volatile uint32_t FC0_INTERVAL;
  volatile uint32_t FC0_SRC;
  volatile uint32_t FC0_STATUS;
  volatile uint32_t FC0_RESULT;
} clocks_hw_t;
#define CLOCKS ((clocks_hw_t *const)clocks_base)

typedef struct {
  volatile uint32_t STATUS; // 0x00
  volatile uint32_t CTRL;   // 0x04
} io_gpio_hw_t;

typedef struct {
  volatile uint32_t INTR;       // raw interrupt
  volatile uint32_t PROC0_INTE; // enable
  volatile uint32_t PROC0_INTF; // force
  volatile uint32_t PROC0_INTS; // status

  volatile uint32_t PROC1_INTE;
  volatile uint32_t PROC1_INTF;
  volatile uint32_t PROC1_INTS;

  volatile uint32_t DORMANT_WAKE_INTE;
  volatile uint32_t DORMANT_WAKE_INTF;
  volatile uint32_t DORMANT_WAKE_INTS;
} io_irq_ctrl_hw_t;

typedef struct {
  io_gpio_hw_t GPIO[30]; // 0x000 - 0x0EF

  uint32_t _reserved0[2]; // alignment to 0x100

  io_irq_ctrl_hw_t IRQ[4]; // 0x100 - 0x13F
} io_bank0_hw_t;
#define IO_BANK0 ((io_bank0_hw_t *const)io_bank0)

typedef struct {
  volatile uint32_t CPUID; // 0x000

  volatile uint32_t GPIO_IN;    // 0x004
  volatile uint32_t GPIO_HI_IN; // 0x008

  volatile uint32_t _reserved0; // 0x00C

  volatile uint32_t GPIO_OUT;     // 0x010
  volatile uint32_t GPIO_OUT_SET; // 0x014
  volatile uint32_t GPIO_OUT_CLR; // 0x018
  volatile uint32_t GPIO_OUT_XOR; // 0x01C

  volatile uint32_t GPIO_OE;     // 0x020
  volatile uint32_t GPIO_OE_SET; // 0x024
  volatile uint32_t GPIO_OE_CLR; // 0x028
  volatile uint32_t GPIO_OE_XOR; // 0x02C

  volatile uint32_t GPIO_HI_OUT;     // 0x030
  volatile uint32_t GPIO_HI_OUT_SET; // 0x034
  volatile uint32_t GPIO_HI_OUT_CLR; // 0x038
  volatile uint32_t GPIO_HI_OUT_XOR; // 0x03C

  volatile uint32_t GPIO_HI_OE;     // 0x040
  volatile uint32_t GPIO_HI_OE_SET; // 0x044
  volatile uint32_t GPIO_HI_OE_CLR; // 0x048
  volatile uint32_t GPIO_HI_OE_XOR; // 0x04C

  volatile uint32_t FIFO_ST; // 0x050
  volatile uint32_t FIFO_WR; // 0x054
  volatile uint32_t FIFO_RD; // 0x058

  volatile uint32_t SPINLOCK_ST; // 0x05C

  volatile uint32_t DIV_UDIVIDEND; // 0x060
  volatile uint32_t DIV_UDIVISOR;  // 0x064
  volatile uint32_t DIV_SDIVIDEND; // 0x068
  volatile uint32_t DIV_SDIVISOR;  // 0x06C
  volatile uint32_t DIV_QUOTIENT;  // 0x070
  volatile uint32_t DIV_REMAINDER; // 0x074

  volatile uint32_t DIV_CSR; // 0x078

  volatile uint32_t _reserved1; // 0x07C

  volatile uint32_t SPINLOCK[32]; // 0x080 - 0x0FC

} sio_hw_t;
#define SIO ((sio_hw_t *const)sio_base)

typedef struct {
  volatile uint32_t TIMEHW; // 0x00: upper 32 bits of time
  volatile uint32_t TIMELW; // 0x04: lower 32 bits of time

  volatile uint32_t TIMEHR; // 0x08: upper 32 bits (latched read)
  volatile uint32_t TIMELR; // 0x0C: lower 32 bits (latched read)

  volatile uint32_t ALARM[4]; // 0x10–0x1C: alarms 0–3

  volatile uint32_t ARMED; // 0x20: which alarms are armed

  volatile uint32_t TIMERAWH; // 0x24: write high (set time)
  volatile uint32_t TIMERAWL; // 0x28: write low

  volatile uint32_t DBGPAUSE; // 0x2C: pause in debug

  volatile uint32_t PAUSE; // 0x30: pause timer

  volatile uint32_t INTR; // 0x34: raw interrupt status
  volatile uint32_t INTE; // 0x38: interrupt enable
  volatile uint32_t INTF; // 0x3C: interrupt force
  volatile uint32_t INTS; // 0x40: masked interrupt status
} timer_hw_t;
#define TIMER ((timer_hw_t *const)timer_base)

typedef struct {
  volatile uint32_t ISER[1];
  uint32_t RESERVED0[31];
  volatile uint32_t ICER[1];
  uint32_t RESERVED1[31];
  volatile uint32_t ISPR[1];
  uint32_t RESERVED2[31];
  volatile uint32_t ICPR[1];
  uint32_t RESERVED3[31];
  volatile uint32_t IP[8];
} nvic_type; // other MCUs with more interrupts may have multiple regs for
             // ISER,ICER etc, hence they're arrays

#define NVIC ((nvic_type *)0xE000E100UL)

#endif
