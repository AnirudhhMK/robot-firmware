#include "math.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
int test_arctan2();
int test_invsqrt();
int main() { test_arctan2(); }
int test_arctan2() {
  float max = 0;
  for (uint32_t i = 0; i < 360; i++) {
    float angle = i * M_PI / 180;
    double yd = sin(angle);
    double xd = cos(angle);
    q16_16_t y = yd * (1 << 16);
    q16_16_t x = xd * (1 << 16);
    double resd = (double)arctan2(y, x) / (double)(1 << 16);
    printf("angle:%f ,resd:%f, err:%f\n", angle, resd,
           (180 / M_PI) * atan2(y, x) - resd);
  }
}
int test_invsqrt() {
  float max = 0;
  for (uint32_t i = 1; i < (1 << 3); i++) {
    q16_16_t num = (uint32_t)(i << 1);
    q16_16_t res = inv_sqrt(num);
    float numf = (float)num / (float)(1 << 16);
    float resf = (float)res / (float)(1 << 16);
    if (max < fabsf(1 / sqrt(numf) - resf))
      max = fabsf(1 / sqrt(numf) - resf);
    printf("%f : %f , %f\n", numf, resf, (1 / sqrt(numf)) - (resf));
  }
  printf("max : %f\n", max);
}
