#include "my_malloc.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
struct free_region_node_tag {
  size_t size;
  struct free_region_node_tag * next;
};
typedef struct free_region_node_tag free_node_t;
static free_node_t head_node = {0, NULL};  //dummy head node

static void * heap_start = NULL;
static int program_start = 0;

//first fit
void * ff_malloc(size_t size) {
  //printf("ff_malloc called\n");
  free_node_t * cur = head_node.next;
  free_node_t ** prev_next = &head_node.next;
  while (cur != NULL) {
    if (cur->size >= size) {   //find region! first fit.
      *prev_next = cur->next;  //remove the node from list
      //printf("%s", "find free slot to alloc.\n");
      //printf("cur->size is %ld, and need to alloc is %ld.\n",
      //cur->size,
      //size + sizeof(free_node_t));
      if (cur->size > 2 * (sizeof(free_node_t) + size)) {  //enough free space, split
        //printf("%s", "the free slot is large, split.\n");
        free_node_t * toAdd = (free_node_t *)(((char *)cur) + sizeof(free_node_t) + size);
        toAdd->next = head_node.next;
        toAdd->size = cur->size - size - sizeof(free_node_t);
        head_node.next = toAdd;
        cur->size = size;
      }
      return ((char *)cur) + sizeof(free_node_t);
    }
    prev_next = &cur->next;
    cur = cur->next;
  }                                                     //no free block match
  void * new_block = sbrk(sizeof(free_node_t) + size);  //size including data and metadata
  if (new_block == (void *)(-1)) {
    perror("sbrk_failure.\n");
    exit(EXIT_FAILURE);
  }
  free_node_t * block = (free_node_t *)new_block;
  //printf("sbrk\n");
  if (program_start == 0) {
    heap_start = block;
    program_start = 1;
  }
  block->size = size;
  block->next = NULL;
  return ((char *)block) + sizeof(free_node_t);
}

void ff_free(void * ptr) {
  if (ptr == NULL) {
    return;
  }
  free_node_t * ptr_free_node = (free_node_t *)(((char *)ptr) - sizeof(free_node_t));
  free_node_t * back_adj = (free_node_t *)(((char *)ptr) + ptr_free_node->size);
  free_node_t * front_adj = NULL;
  free_node_t * cur = head_node.next;
  free_node_t ** prev_next = &head_node.next;
  while (cur != NULL) {
    if (cur == back_adj) {  //find back free block, merge
      //printf("%s", "find back free block, merge\n");
      if (front_adj == NULL) {
        //printf("%s", "not find front free slot yet.\n");
        ptr_free_node->size += back_adj->size + sizeof(free_node_t);
      }
      else {
        //printf("%s", "add to front free slot.\n");
        front_adj->size += back_adj->size + sizeof(free_node_t);
      }
      *prev_next = cur->next;  //remove back_adj from list
    }
    else if ((free_node_t *)(((char *)cur) + cur->size + sizeof(free_node_t)) ==
             ptr_free_node) {  //find front free slot, merge
      //printf("%s", "find front free slot,merge.\n");
      front_adj = cur;
      front_adj->size += ptr_free_node->size + sizeof(free_node_t);
    }
    prev_next = &cur->next;
    cur = cur->next;
  }
  if (front_adj == NULL) {  //add ptr to free list
    //printf("add to free list.\n");
    ptr_free_node->next = head_node.next;
    head_node.next = ptr_free_node;
  }
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
  return (char *)sbrk(0) - (char *)heap_start;
}  //in bytes
unsigned long get_data_segment_free_space_size() {
  free_node_t * cur = &head_node;
  size_t size = 0;
  while (cur->next != NULL) {
    cur = cur->next;
    size += sizeof(free_node_t) + cur->size;
  }
  return size;
}  //in byte
