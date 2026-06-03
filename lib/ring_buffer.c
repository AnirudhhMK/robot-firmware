#include "ring_buffer.h"
#include <stdint.h>
// implementation of ring buffer with monotonically increasing reader and
// writer, wrap around at 2^32 doesn't affect anything unless buf size >2^31
// reader is the counter where next read occurs, writer is counter where next
// write occurs, occupancy is writer-reader

// return value if operation fails is always negative
// incase of write_overriding if some data is overwriten,-(1+data) is returned
inline int32_t Rbuf_write_blocking(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader >
      buf->mask) // occupancy >= capacity, occupancy > capacity - 1
    return -1;
  buf->data[buf->writer++ & buf->mask] = *data;
  return 0;
}

inline int32_t Rbuf_write_overriding(ringbuf_handle_t buf, uint8_t *data) {
  int32_t ret = 0;
  if (buf->writer - buf->reader > buf->mask)
    ret = ~(uint32_t)(buf->data[buf->reader++ & buf->mask]);
  buf->data[buf->writer++ & buf->mask] = *data;
  return ret;
}

inline int32_t Rbuf_read(ringbuf_handle_t buf, uint8_t *data) {
  if (buf->writer - buf->reader <= 0)
    return -1;
  *data = buf->data[buf->reader++ & buf->mask];
  return 0;
}

int32_t Rbuf_write_bulk(ringbuf_handle_t buf, uint8_t *data, uint32_t size) {
  if ((buf->writer + size) - buf->reader > buf->mask + 1)
    return -1;
  uint32_t w = buf->writer;
  for (uint32_t i = 0; i < size; i++) {
    buf->data[w++ & buf->mask] = data[i];
  }
  buf->writer = w;
  return 0;
}
