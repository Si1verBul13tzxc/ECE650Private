//First Fit malloc/free
#include <stdlib.h>
typedef struct header_tag {
  size_t data_size;
  int allocated;
  struct header_tag * list_prev;
  struct header_tag * list_next;
} header_t;

typedef struct footer_tag {
  size_t data_size;
  int allocated;
  int has_back;
} footer_t;

void * ff_malloc(size_t size);
void ff_free(void * ptr);

//Best Fit
void * bf_malloc(size_t size);
void bf_free(void * ptr);

//test methods
unsigned long get_data_segment_size();             //in bytes
unsigned long get_data_segment_free_space_size();  //in byte

//helper
void init_heap();
void init_sentinel_head();
void init_sentinel_foot();
header_t * ff_find(size_t size);
header_t * init_header_footer(size_t size, void * new_block);
void add_to_front(header_t * header);
footer_t * find_self_footer(header_t * header, size_t data_size);
footer_t * find_front_footer(header_t * header);
header_t * find_self_header(footer_t * footer, size_t size);
header_t * find_back_header(footer_t * footer);
void delete_from_list(header_t * header);
void split(size_t allocated_data_size, header_t * header);
