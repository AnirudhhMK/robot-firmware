#include "ring_buffer.h"
#include <stdint.h>
// implementation of ring buffer with monotonically increasing reader and
// writer, wrap around at 2^32 doesn't affect anything unless buf size >2^31
// reader is the counter where next read occurs, writer is counter where next
// write occurs, occupancy is writer-reader

// return value if operation fails is always negative
// incase of write_overriding if some data is overwriten,-(1+data) is returned
int32_t Rbuf_write_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size) {
  if ((buf->writer + size) - buf->reader > buf->mask + 1)
    return -1; // maybe return the amount of excess space needed?
  uint32_t w = buf->writer;
  for (uint32_t i = 0; i < size; i++) {
    buf->data[w++ & buf->mask] = data[i];
  }
  buf->writer = w;
  return 0;
}

int32_t Rbuf_read_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size) {
  if ((buf->reader + size) > buf->writer)
    return -1;
  uint32_t r = buf->reader;
  for (uint32_t i = 0; i < size; i++) {
    data[i] = buf->data[r++ & buf->mask];
  }
  buf->reader = r;
  return 0;
}
