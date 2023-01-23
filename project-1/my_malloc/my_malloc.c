#include "my_malloc.h"

#include <assert.h>
#include <unistd.h>
static void * heap_start = NULL;
static int program_start = 0;
static header_t * sentinel_head = NULL;
static footer_t * sentinel_foot = NULL;

void init_heap() {
  assert(program_start == 0);
  program_start = 1;
  heap_start = sbrk(sizeof(header_t) + sizeof(footer_t));
  init_sentinel_head();
  init_sentinel_foot();
}

void init_sentinel_head() {
  sentinel_head = (header_t *)heap_start;
  sentinel_head->allocated = 1;
  sentinel_head->data_size = 0;
  sentinel_head->list_next = sentinel_head;
  sentinel_head->list_prev = sentinel_head;
}

void init_sentinel_foot() {
  sentinel_foot = find_self_footer((header_t *)heap_start, 0);
  sentinel_foot->allocated = 1;
  sentinel_foot->data_size = 0;
  sentinel_foot->has_back = 0;
}

void * ff_malloc(size_t size) {
  if (program_start == 0) {
    init_heap();
  }
  header_t * header = ff_find(size);
  if (header != NULL) {
    header->allocated = 1;  //set to allocated
    find_self_footer(header, header->data_size)->allocated = 1;
    //TODO: large size try to split.
    return ((char *)header) + sizeof(header_t);
  }
  //no free block find, sbrk
  void * new_block = sbrk(sizeof(header_t) + size + sizeof(footer_t));
  header_t * new_block_header = init_header_footer(size, new_block);
  return ((char *)new_block_header) + sizeof(header_t);
}

header_t * init_header_footer(size_t size, void * new_block) {
  header_t * new_block_header = (header_t *)new_block;
  new_block_header->allocated = 1;
  new_block_header->data_size = size;
  new_block_header->list_next = NULL;
  new_block_header->list_next = NULL;
  footer_t * new_block_footer = find_self_footer(new_block_header, size);
  new_block_footer->allocated = 1;
  new_block_footer->data_size = size;
  new_block_footer->has_back = 0;
  find_front_footer(new_block_header)->has_back = 1;
  return new_block_header;
}

footer_t * find_self_footer(header_t * header, size_t data_size) {
  return (footer_t *)(((char *)header) + sizeof(header_t) + data_size);
}
footer_t * find_front_footer(header_t * header) {
  return (footer_t *)(((char *)header) - sizeof(footer_t));
}
header_t * find_self_header(footer_t * footer, size_t data_size) {
  return (header_t *)(((char *)footer) - data_size - sizeof(header_t));
}
header_t * find_back_header(footer_t * footer) {
  return (header_t *)(((char *)footer) + sizeof(footer_t));
}

header_t * ff_find(size_t size) {
  header_t * cur = sentinel_head->list_next;
  while (cur != sentinel_head) {
    assert(cur->allocated == 0);
    if (cur->data_size >= size) {
      delete_from_list(cur);
      return cur;
    }
    cur = cur->list_next;
  }
  return NULL;
}

void ff_free(void * ptr) {
  header_t * header = (header_t *)(((char *)ptr) - sizeof(header_t));
  footer_t * footer = find_self_footer(header, header->data_size);
  //merge if possible
  if (footer->has_back && find_back_header(footer)->allocated == 0) {
    if (find_front_footer(header)->allocated == 0) {  //case 1: merge front and back;
      footer_t * front_footer = find_front_footer(header);
      header_t * front_header = find_self_header(front_footer, front_footer->data_size);
      header_t * back_header = find_back_header(footer);
      footer_t * back_footer = find_self_footer(back_header, back_header->data_size);
      front_header->data_size += 2 * sizeof(header_t) + 2 * sizeof(footer_t) +
                                 header->data_size + back_header->data_size;
      delete_from_list(back_header);
      back_footer->allocated = 0;
      back_footer->data_size = front_header->data_size;
    }
    else {  //case2: merge back
      header_t * back_header = find_back_header(footer);
      delete_from_list(back_header);
      header->data_size += sizeof(footer_t) + sizeof(header_t) + back_header->data_size;
      header->allocated = 0;
      footer = find_self_footer(header, header->data_size);
      footer->allocated = 0;
      footer->data_size = header->data_size;
      add_to_front(header);
    }
  }
  else {
    if (find_front_footer(header)->allocated == 0) {  //case 3: merge front
      footer_t * front_footer = find_front_footer(header);
      header_t * front_header = find_self_header(front_footer, front_footer->data_size);
      assert(front_header->allocated == 0);
      front_header->data_size += sizeof(footer_t) + sizeof(header_t) + header->data_size;
      footer->allocated = 0;
      footer->data_size = front_header->data_size;
    }
    else {  //case 4: no merge
      add_to_front(header);
      header->allocated = 0;
      find_self_footer(header, header->data_size)->allocated = 0;
    }
  }
}

void add_to_front(header_t * header) {
  header->list_next = sentinel_head->list_next;
  header->list_prev = sentinel_head;
  sentinel_head->list_next->list_prev = header;
  sentinel_head->list_next = header;  //add to free list
}

void delete_from_list(header_t * header) {
  header->list_prev->list_next = header->list_next;
  header->list_next->list_prev = header->list_prev;  //remove the node from list.
  header->list_next = NULL;
  header->list_prev = NULL;
}

//Best Fit
void * bf_malloc(size_t size) {
  return NULL;
}
void bf_free(void * ptr) {
  return;
}

//test methods
unsigned long get_data_segment_size() {
  return sbrk(0) - heap_start;
}  //in bytes
unsigned long get_data_segment_free_space_size() {
  header_t * cur = sentinel_head->list_next;
  size_t ans = 0;
  while (cur != sentinel_head) {
    ans += sizeof(header_t) + cur->data_size + sizeof(footer_t);
    cur = cur->list_next;
  }
  return ans;
}  //in byte
