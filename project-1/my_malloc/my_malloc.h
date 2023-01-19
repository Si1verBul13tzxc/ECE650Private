//First Fit malloc/free
#include <stdlib.h>
void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best Fit
void * bf_malloc(size_t size);
void bf_free(void * ptr);

//test methods
unsigned long get_data_segment_size();             //in bytes
unsigned long get_data_segment_free_space_size();  //in byte
