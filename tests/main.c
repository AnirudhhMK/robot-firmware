#include "math.h"
#include <math.h>
#include <stdio.h>
int main() {
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
