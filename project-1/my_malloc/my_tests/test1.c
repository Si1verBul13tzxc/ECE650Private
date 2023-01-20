#include <stdio.h>
#include <stdlib.h>

#include "my_malloc.h"
#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p) ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p) bf_free(p)
#endif

int main() {
  int * p = MALLOC(5 * sizeof(int));
  for (size_t i = 0; i < 6; i++) {
    p[i] = i * i;
  }
  int sum = 0;
  int exp_sum = 0;
  for (size_t i = 0; i < 5; i++) {
    sum += p[i];
    exp_sum += i * i;
  }
  printf("sum=%d, exp_sum=%d\n", sum, exp_sum);
  return EXIT_SUCCESS;
}
