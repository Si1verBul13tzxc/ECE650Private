//First Fit malloc/free
#include <stdlib.h>
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best Fit
void * bf_malloc(size_t size);
void bf_free(void * ptr);
