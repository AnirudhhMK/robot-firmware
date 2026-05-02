#include <stdint.h>
#include <stdio.h>
#define ONE_q16_16 (1 << 16)
#define ROOTHALF 0.7071067811865475

typedef int32_t q16_16_t;
uint8_t LUT[64] = {128, 127, 126, 125, 124, 123, 122, 122, 121, 120, 119,
                   118, 117, 117, 116, 115, 114, 114, 113, 112, 112, 111,
                   110, 110, 109, 109, 108, 107, 107, 106, 106, 105, 105,
                   104, 103, 103, 102, 102, 101, 101, 100, 100, 99,  99,
                   99,  98,  98,  97,  97,  96,  96,  95,  95,  95,  94,
                   94,  93,  93,  93,  92,  92,  92,  91,  91};
q16_16_t inv_sqrt(q16_16_t num) {
  // range reduction
  uint8_t x = __builtin_clz(num); // num : bits[31..x] = 0, q16_16
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
