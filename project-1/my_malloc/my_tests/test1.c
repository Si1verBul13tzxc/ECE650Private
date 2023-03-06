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
  int * int1 = MALLOC(10 * sizeof(int));
  printf("---------------------------------\n");
  int * int2 = MALLOC(sizeof(int));
  printf("---------------------------------\n");
  int * int3 = MALLOC(sizeof(int));
  printf("---------------------------------\n");
  *int1 = 1;
  *int2 = 2;
  *int3 = 3;
  FREE(int1);
  printf("---------------------------------\n");
  char * char1 = MALLOC(sizeof(char));
  *char1 = 'a';
  printf("---------------------------------\n");
  char * char2 = MALLOC(sizeof(char));
  *char2 = 'b';
  printf("---------------------------------\n");
  char * char3 = MALLOC(sizeof(char));
  *char3 = 'c';
  printf("---------------------------------\n");
  FREE(char1);
  printf("---------------------------------\n");
  FREE(char2);
  printf("---------------------------------\n");
  FREE(char3);
  printf("---------------------------------\n");
  FREE(int3);
  printf("---------------------------------\n");
  FREE(int2);
  printf("---------------------------------\n");
  return EXIT_SUCCESS;
}
