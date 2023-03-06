//project 2 header file: thread safe malloc and free.
#include <stdlib.h>
typedef struct header_tag {
  size_t data_size;
  int allocated;
  struct header_tag * list_prev;
  struct header_tag * list_next;
  struct header_tag * front;
  struct header_tag * back;
} header_t;

//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size);
void ts_free_lock(void * ptr);

//Thread Safe malloc/free: non-locking version
void * ts_malloc_nolock(size_t size);
void ts_free_nolock(void * ptr);

//to be compatible with project1
void * bf_malloc(size_t size);
void * ff_malloc(size_t size);
void ff_free(void * ptr);
void bf_free(void * ptr);
//helper
void * _bf_malloc(size_t size, int mode);
void _init_heap(int mode);
header_t * _init_header(size_t size, void * new_block, int mode);
void _add_to_front(header_t * header, int mode);
void _delete_from_list(header_t * header);
void _split(size_t allocated_data_size, header_t * header, int mode);
header_t * _bf_find(size_t size, int mode);
void _bf_free(void * ptr, int mode);
header_t * _find_back_adjcent_header(header_t * front_header, size_t data_size);
void _add_to_back(header_t * sentinel, header_t * new_header);
void _delete_from_allocation(header_t * header);
