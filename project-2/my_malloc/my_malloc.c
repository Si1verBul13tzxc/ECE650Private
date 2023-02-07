#include "my_malloc.h"

#include <assert.h>
#include <pthread.h>
#include <unistd.h>
static void * heap_start = NULL;
static int program_start = 0;
static header_t * sentinel_head = NULL;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//thread-local-storage
__thread void * th_heap_start = NULL;
__thread int th_program_start = 0;
__thread header_t * th_sentinel_head = NULL;
//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void * ans = _bf_malloc(size, 0);
  pthread_mutex_unlock(&lock);
  return ans;
}

void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&lock);
  _bf_free(ptr, 0);
  pthread_mutex_unlock(&lock);
}

void * ts_malloc_nolock(size_t size) {
  return _bf_malloc(size, 1);
}

void ts_free_nolock(void * ptr) {
  _bf_free(ptr, 1);
}

//***********helper***********************//
void * _bf_malloc(size_t size, int mode) {
  if (mode == 0) {
    if (program_start == 0) {
      _init_heap(0);
    }
  }
  else if (mode == 1) {
    if (th_program_start == 0) {
      _init_heap(1);
    }
  }
  header_t * header = _bf_find(size, mode);
  if (header != NULL) {
    if (header->data_size >=
        2 * size + sizeof(header_t)) {  //large size: the size is large
      _split(size, header, mode);
    }
    else {                    // no need to split
      header->allocated = 1;  //set to allocated
    }
    return ((char *)header) + sizeof(header_t);
  }
  //no free block find, sbrk
  if (mode == 1) {  //unlock version, but need to lock sbrk
    pthread_mutex_lock(&lock);
  }
  void * new_block = sbrk(sizeof(header_t) + size);
  if (mode == 1) {
    pthread_mutex_unlock(&lock);
  }
  header_t * new_block_header = _init_header(size, new_block, mode);
  return ((char *)new_block_header) + sizeof(header_t);
}

void _init_heap(int mode) {
  header_t * sentinel = NULL;
  if (mode == 0) {  //lock version
    program_start = 1;
    heap_start = sbrk(sizeof(header_t));
    sentinel_head = (header_t *)heap_start;
    sentinel = sentinel_head;
  }
  else if (mode == 1) {  //nolock version
    th_program_start = 1;
    pthread_mutex_lock(&lock);
    th_heap_start = sbrk(sizeof(header_t));  //sbrk need lock
    pthread_mutex_unlock(&lock);
    sentinel_head = th_sentinel_head = (header_t *)th_heap_start;
    sentinel = th_sentinel_head;
  }
  sentinel->allocated = 1;
  sentinel->data_size = 0;
  sentinel->list_next = sentinel;
  sentinel->list_prev = sentinel;
  sentinel->front = sentinel;
  sentinel->back = sentinel;
}

//enough to allocate 2 of required size, split
void _split(size_t allocated_data_size, header_t * header, int mode) {
  size_t new_block_data_size = header->data_size - allocated_data_size - sizeof(header_t);
  header->allocated = 1;
  header->data_size = allocated_data_size;
  header_t * new_header = _find_back_adjcent_header(header, allocated_data_size);
  new_header->allocated = 0;
  new_header->back = header->back;
  if (new_header->back) {
    new_header->back->front = new_header;
  }
  new_header->front = header;
  new_header->data_size = new_block_data_size;
  header->back = new_header;
  _add_to_front(new_header, mode);
}

header_t * _find_back_adjcent_header(header_t * front_header, size_t data_size) {
  return (header_t *)((char *)front_header + sizeof(header_t) + data_size);
}

header_t * _init_header(size_t size, void * new_block, int mode) {
  header_t * new_block_header = (header_t *)new_block;
  new_block_header->allocated = 1;
  new_block_header->data_size = size;
  new_block_header->list_next = NULL;
  new_block_header->list_next = NULL;
  if (mode == 0) {  //lock version
    _add_to_back(sentinel_head, new_block_header);
  }
  else if (mode == 1) {
    _add_to_back(th_sentinel_head, new_block_header);
  }
  return new_block_header;
}

header_t * _bf_find(size_t size, int mode) {
  header_t * sentinel = NULL;
  if (mode == 0) {  //lock version
    sentinel = sentinel_head;
  }
  else if (mode == 1) {
    sentinel = th_sentinel_head;
  }
  header_t * cur = sentinel->list_next;
  header_t * bf_header = NULL;
  size_t min_size_diff = (size_t)(-1);  //the largest size_t
  size_t size_diff = 0;
  while (cur != sentinel) {
    if (cur->data_size >= size) {
      if (cur->data_size == size) {  //perfectly fit
        _delete_from_list(cur);
        return cur;
      }
      else {
        size_diff = cur->data_size - size;
        if (size_diff < min_size_diff) {
          bf_header = cur;
          min_size_diff = size_diff;
        }
      }
    }
    cur = cur->list_next;
  }
  if (bf_header) {
    _delete_from_list(bf_header);
    return bf_header;
  }
  return NULL;
}

void _bf_free(void * ptr, int mode) {
  header_t * sentinel = NULL;
  if (mode == 0) {
    sentinel = sentinel_head;
  }
  else if (mode == 1) {
    sentinel = th_sentinel_head;
  }
  header_t * header = (header_t *)(((char *)ptr) - sizeof(header_t));
  header_t * front_adjcent_free_header = NULL;
  header_t * back_adjcent_free_header = NULL;
  //merge if possible
  if (header->back != sentinel &&
      header->back == _find_back_adjcent_header(header, header->data_size) &&
      header->back->allocated == 0) {  //merge back
    back_adjcent_free_header = header->back;
    header->data_size += back_adjcent_free_header->data_size + sizeof(header_t);
    _delete_from_allocation(back_adjcent_free_header);
    _delete_from_list(back_adjcent_free_header);
  }
  if (header->front != sentinel &&
      _find_back_adjcent_header(header->front, header->front->data_size) == header &&
      header->front->allocated == 0) {  //merge front
    front_adjcent_free_header = header->front;
    front_adjcent_free_header->data_size += header->data_size + sizeof(header_t);
    _delete_from_allocation(header);
  }
  if (front_adjcent_free_header == NULL) {
    header->allocated = 0;
    _add_to_front(header, mode);
  }
}

void _add_to_front(header_t * header, int mode) {
  header_t * sentinel = NULL;
  if (mode == 0) {
    sentinel = sentinel_head;
  }
  else if (mode == 1) {
    sentinel = th_sentinel_head ? th_sentinel_head : sentinel_head;
  }
  header->list_next = sentinel->list_next;
  header->list_prev = sentinel;
  sentinel->list_next->list_prev = header;
  sentinel->list_next = header;  //add to free list
}

void _delete_from_list(header_t * header) {
  header->list_prev->list_next = header->list_next;
  header->list_next->list_prev = header->list_prev;  //remove the node from list.
  header->list_next = NULL;
  header->list_prev = NULL;
}

void _add_to_back(header_t * sentinel, header_t * new_header) {
  new_header->back = sentinel;
  new_header->front = sentinel->front;
  sentinel->front->back = new_header;
  sentinel->front = new_header;
}

void _delete_from_allocation(header_t * header) {
  header->front->back = header->back;
  header->back->front = header->front;
  header->back = NULL;
  header->front = NULL;
}

//***************to test on project 1***********//
void * bf_malloc(size_t size) {
  return _bf_malloc(size, 0);
}
void * ff_malloc(size_t size) {
  return _bf_malloc(size, 0);
}
void ff_free(void * ptr) {
  _bf_free(ptr, 0);
}
void bf_free(void * ptr) {
  _bf_free(ptr, 0);
}

//test methods
unsigned long get_data_segment_size() {
  return sbrk(0) - heap_start;
}
//in bytes
unsigned long get_data_segment_free_space_size() {
  header_t * cur = sentinel_head->list_next;
  size_t ans = 0;
  while (cur != sentinel_head) {
    ans += sizeof(header_t) + cur->data_size;
    cur = cur->list_next;
  }
  return ans;
}  //in byte
