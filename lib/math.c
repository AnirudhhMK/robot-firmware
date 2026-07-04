#include "asm_func.h"
#include "regs.h"
#include <stdint.h>
#define ONE_q16_16 (1 << 16)
typedef int32_t q16_16_t;

q16_16_t mulq16_16(q16_16_t x, q16_16_t y) { // untested
  int32_t y1 = y >> 16;
  int32_t x1 = x >> 16;
  int32_t x0 = (int16_t)x;
  int32_t y0 = (int16_t)y;
  return x * y1 + (x0 * y0 >> 16) + x1 * y0;
}

q16_16_t thetaTable[16] = {2949120, 1740967, 919879, 466945, 234378, 117303,
                           58666,   29334,   14667,  7333,   3666,   1833,
                           916,     458,     229,    114};
__attribute__((section(".ramcode"))) q16_16_t
arctan(q16_16_t y,
       q16_16_t x) { // gives result in degrees, using standard CORDIC algorithm
  q16_16_t theta = 0;
  uint8_t m = 0;
  if (x < 0) {
    x = -x;
    y = -y;
    theta += y < 0 ? 180 * (1 << 16) : -180 * (1 << 16);
  }
  if (x >= (1 << 29)) { // ensure that even in worse case there can be no
                        // overflow in x, as x will increase
    x >>= 2;
    y >>= 2;
  }
  for (uint8_t p = 0; p < 16; p++) {
    if (y > 0) {
      q16_16_t temp = y - (x >> p);
      x = x + (y >> p);
      y = temp;
      theta += thetaTable[p];
    } else {
      q16_16_t temp = y + (x >> p);
      x = x - (y >> p);
      y = temp;
      theta -= thetaTable[p];
    }
  }
  return theta;
}

uint8_t LUT[64] = {128, 127, 126, 125, 124, 123, 122, 122, 121, 120, 119,
                   118, 117, 117, 116, 115, 114, 114, 113, 112, 112, 111,
                   110, 110, 109, 109, 108, 107, 107, 106, 106, 105, 105,
                   104, 103, 103, 102, 102, 101, 101, 100, 100, 99,  99,
                   99,  98,  98,  97,  97,  96,  96,  95,  95,  95,  94,
                   94,  93,  93,  93,  92,  92,  92,  91,  91};

__attribute__((section(".ramcode"))) q16_16_t
inv_sqrt(q16_16_t num) { // 86 cycles
  // pls add a guard to check if num == 0
  //  range reduction
  uint8_t x = _clz(num); // num : bits[31..x] = 0, q16_16
  uint32_t ind = num << x;
  uint32_t rrnum = ind >> (31 - 15); //[15] has leading 1, easy multiply, q1_15
                                     // guarenteed since rrnum ∈ [1,2)
  //  rrnum : bits[31..16] = 0, q1_15
  //  hence rrnum = num/2^(15-x)
  //  rrRes = 1/sqrt(rrnum)
  //  Res = rrRes/ 2^((15-x)/2)
  ind = ind << 1;
  ind = ind >> (32 - 6);
  uint32_t y = LUT[ind] << (15 - 7); // again q1_15 form guarenteed by
                                     // LUT[ind]/128 ∈ (0.707.., 1]
  //  y = y << 1;
  q16_16_t threehalves = 3 << (15 - 1);
  q16_16_t num2 = rrnum >> 1;
  y = (y * (threehalves - ((((num2 * y) >> 15) * y) >> 15))) >>
      15; // shift by 15 to renormalize to q2_15 from q2_30
          // since num2*y*y converges to 0.5 (and will stay approximately around
          // 0.5)
          //(((q1_15 * q1_15)>>15) * q1_15)>>15 = (q2_15 * q1_15)>>15, although
          // under normal cicumstances, this can overflow, since this result
          // should be around 0.5, it cannot overflow
  // y = (y * (threehalves - ((((num2 * y) >> 15) * y) >> 15))) >> 15;
  y = y << 1; // to bring it back to q1_16, i.e q16_16_t form
  // now to reverse range reduction
  // if 15-x is even, Res = rrRes>>((15-x)>>1)
  // else: Res = (1/root2)*rrRes>>((15-x-1)>>1)
  int8_t z = 15 - x;
  if (z & 1) {
    z--;
    y = y * ((uint32_t)46340); // y = y*1/root2 in q16_16
    y = y >> 16;               // renormalize
  }
  q16_16_t Res;
  if (z < 0) {
    Res = y << ((-z) >> 1);
  } else {
    Res = y >> (z >> 1);
  }
  return Res;
}
