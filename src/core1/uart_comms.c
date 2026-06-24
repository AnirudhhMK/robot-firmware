#include "uart_comms.h"
#include "regs.h"
#include "ring_buffer.h"
#include "scheduler.h"
#include <stdint.h>
#define TXBUF_LEN_LOG 10
#define RXBUF_LEN_LOG 8
static uint8_t tx_buf[1 << TXBUF_LEN_LOG]
    __attribute__((aligned(1 << TXBUF_LEN_LOG)));
static uint8_t rx_buf[1 << RXBUF_LEN_LOG]
    __attribute__((aligned(1 << RXBUF_LEN_LOG)));
ringbuf_handle_t uart_tx_queue =
    &(ringbuf_t){tx_buf, 0, 0, (1 << TXBUF_LEN_LOG) - 1};
ringbuf_handle_t uart_rx_queue =
    &(ringbuf_t){rx_buf, 0, 0, (1 << RXBUF_LEN_LOG) - 1};

void init_uart_comms(void) {
  // assumes DMA and UART is pulled out of reset
  // configure UART
  // configure baud rate to 115207.3732718894
  UART0->UARTIBRD = 54;
  UART0->UARTFBRD = 25;

  UART0->UARTLCR_H = (1 << 1) |   // enable parity check, default to odd parity
                     (1 << 4) |   // enable fifo
                     (0b11 << 5); // 8 bit data size

  UART0->UARTCR = (1 << 0) | // uart enable
                  (1 << 8) | // rx enable
                  (1 << 9) | // tx enable
                  (1 << 7);  // LPE

  UART0->UARTDMACR = (1 << 0) | // transmit dma enabled
                     (1 << 1);  // recieve dma enabled

  // configure DMA channel 0 for tx, channel 1 for rx
  DMA->CH[0].AL1_CTRL =
      (20 << 15) | // uart0_tx dreq
      (0 << 14) |  // ring wrapping on read address (tx buffer)
      (TXBUF_LEN_LOG
       << 6) | // ring size, txbuf_len_log number of bits are the only bits that
               // change, so buf must be aligned according to size
      (0 << 5) |    // do not advance write
      (1 << 4) |    // advance read
      (0b00 << 2) | // 1 byte per transfer
      (1 << 0);     // dma enable

  DMA->CH[1].AL1_CTRL =
      (21 << 15) | // uart0_rx dreq
      (1 << 14) |  // ring wrapping on write address (rx buffer)
      (RXBUF_LEN_LOG
       << 6) | // ring size, rxbuf_len_log number of bits are the only bits that
               // change, so buf must be aligned according to size
      (1 << 5) |    // do not advance read
      (0 << 4) |    // advance write
      (0b00 << 2) | // 1 byte per transfer
      (1 << 0);     // dma enable

  DMA->CH[0].AL1_READ_ADDR = (uint32_t)uart_tx_queue->data;
  DMA->CH[0].AL1_WRITE_ADDR = (uint32_t)&UART0->UARTDR;
  // transfer count is the trigger, once the tx buffer starts filling with
  // packets, the dma handler is called which starts the transfer sequence
  //
  DMA->CH[1].AL1_WRITE_ADDR = (uint32_t)uart_rx_queue->data;
  DMA->CH[1].AL1_READ_ADDR = (uint32_t)&UART0->UARTDR;
  DMA->CH[1].AL1_TRANS_COUNT_TRIG = ((1ull << 32) - 1);
  ACCESS_OFFSET(DMA->INTE0, hw_set) =
      (1 << 0) | (1 << 1); // enable interrupts for channel 0 and channel 1
}

void inline uart_tx_kick(void) {
  // function is called to calculate transfer count and initiate next transfer
  // its safe to call this function during a transfer sequence, as a trigger
  // doesn't do anything if a transfer sequence is in progressm and this updated
  // value just gets stored in reload register which we never use anyway
  DMA->CH[0].AL1_TRANS_COUNT_TRIG =
      (uart_tx_queue->writer - uart_tx_queue->reader);
}

void inline uart_tx_update_reader(void) {
  // the new updated reader is calculated from dma reader address
  // the reader is monotonically increasing, and reader is atmost equal to
  // writer so old_reader <= new_reader <= writer, we want to find new_reader,
  // such that this condition is satisfied and, &uart_tx_queue[new_reader &
  // mask] = dma_reader_addr so let new_reader_masked be new_reader & mask
  uint32_t new_reader_masked =
      (DMA->CH[0].AL1_READ_ADDR - (uint32_t)uart_tx_queue->data) /
      sizeof(uart_tx_queue->data[0]);
  // Note : rp2040:E14 causes the address read from READ_ADDR to not be wrapped
  // if its read while a transfer is underway, this however doesn't matter as we
  // take modulo size for the math anyways
  uint32_t old_reader = uart_tx_queue->reader;
  uint32_t mask = uart_tx_queue->mask;
  uart_tx_queue->reader =
      ((new_reader_masked - old_reader) & mask) + old_reader;
  // now new reader is the closes value above old reader satisfying the 2
  // conditions
}

void inline uart_rx_update_writer() {
  uint32_t new_writer_masked =
      (DMA->CH[1].AL1_WRITE_ADDR - (uint32_t)uart_rx_queue->data) /
      sizeof(uart_rx_queue->data[0]);
  uint32_t old_writer = uart_rx_queue->writer;
  uint32_t mask = uart_rx_queue->mask;
  uart_rx_queue->writer =
      ((new_writer_masked - old_writer) & mask) + old_writer;
}

int32_t uart_tx_send(uint8_t *data, uint32_t size) {
  uart_tx_update_reader();
  int32_t res = Rbuf_write_bulk(uart_tx_queue, data, size);
  uart_tx_kick();
  return res;
}
int32_t uart_rx_get(command_packet_t *packet) {
  // Note: we are depending on the assumption that overflows don't happen, if it
  // does we are not handling it, and just pretending that its all okay

  uart_rx_update_writer();
  int32_t n =
      uart_rx_queue->writer - uart_rx_queue->reader - sizeof(command_packet_t);
  uint32_t r = uart_rx_queue->reader;
  while (n >= 0) {
    if (uart_rx_queue->data[r & uart_rx_queue->mask] == 0xaa)
      if (uart_rx_queue->data[(r + 1) & uart_rx_queue->mask] == 0x55) {
        uart_rx_queue->reader = r;
        return Rbuf_read_bulk(uart_rx_queue, (uint8_t *)packet,
                              sizeof(command_packet_t));
      }
    r++;
    n--;
  }
  uart_rx_queue->reader = r;
  return -1;
}

void uart_rx_DMA_handler(void) {
  DMA->CH[1].AL1_TRANS_COUNT_TRIG = (1ull << 32) - 1;
}
void uart_tx_DMA_handler(void) {
  // assuming channel0 interrupt is already cleared by the IRQ that called this
  uart_tx_update_reader();
  uart_tx_kick(); // we don't need to check if the queue is empty because
                  // an empty queue would mean 0 trans_count which just causes a
                  // null trigger hence no transfer is initiated and no
                  // interrupts generated
}
