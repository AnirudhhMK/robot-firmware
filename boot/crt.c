#include <stdint.h>

typedef struct {
  uint32_t *src; // flash (LMA)
  uint32_t *dst; // RAM (VMA)
  uint32_t len;  // in words/bytes
} copy_entry_t;

typedef struct {
  uint32_t *dst;
  uint32_t len;
} zero_entry_t;

extern copy_entry_t __copy_table_start__;
extern copy_entry_t __copy_table_end__;

extern zero_entry_t __zero_table_start__;
extern zero_entry_t __zero_table_end__;

void crt0_init(void) {
  for (copy_entry_t *copy_table = &__copy_table_start__;
       copy_table < &__copy_table_end__; copy_table++) {
    for (uint32_t i = 0; i < copy_table->len; i++) {
      copy_table->dst[i] = copy_table->src[i];
    }
  }
  for (zero_entry_t *zero_table = &__zero_table_start__;
       zero_table < &__zero_table_end__; zero_table++) {
    for (uint32_t i = 0; i < zero_table->len; i++) {
      zero_table->dst[i] = 0;
    }
  }
}
