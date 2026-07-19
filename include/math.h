
#include <stdint.h>
typedef int32_t q16_16_t;
q16_16_t
arctan(q16_16_t y,
       q16_16_t x); // gives result in degrees, using standard CORDIC algorithm
q16_16_t inv_sqrt(q16_16_t num);

q16_16_t mulq16_16(q16_16_t x, q16_16_t y);
