#include "my_malloc.h"

#include <unistd.h>
struct free_region_node_tag {
  size_t size;
  struct free_region_node_tag * next;
};
typedef struct free_region_node_tag free_node_t;
static free_node_t head_node = {0, NULL};  //dummy head node
//first fit
void * ff_malloc(size_t size) {
  free_node_t * cur = &head_node;
  free_node_t * prev = NULL;
  while (cur->next != NULL) {
    prev = cur;
    cur = cur->next;
    if (cur->size >= size) {   //find region! first fit.
      prev->next = cur->next;  //remove the node from the list.
      return ((char *)cur) + sizeof(free_node_t);
    }
  }                                                 //no free block match
  size_t total_alloc = sizeof(free_node_t) + size;  //size including data and metadata
  free_node_t * new_block = sbrk(total_alloc);
  new_block->size = size;
  new_block->next = NULL;
  return ((char *)new_block) + sizeof(free_node_t);
}

void ff_free(void * ptr) {
  if (ptr == NULL) {
    return;
  }
  free_node_t * ptr_free_node = (free_node_t *)(((char *)ptr) - sizeof(free_node_t));
  free_node_t * back_adj = (free_node_t *)(((char *)ptr) + ptr_free_node->size);
  free_node_t * front_adj = NULL;
  free_node_t * cur = &head_node;
  free_node_t * prev = NULL;
  while (cur->next != NULL) {
    prev = cur;
    cur = cur->next;
    if (cur == back_adj) {  //find back free block, merge
      if (front_adj == NULL) {
        ptr_free_node->size += back_adj->size + sizeof(free_node_t);
      }
      else {
        front_adj->size += back_adj->size + sizeof(free_node_t);
      }
      prev->next = cur->next;  //remove back_adj from list
    }
    else if (((char *)cur) + cur->size + sizeof(free_node_t) ==
             (char *)ptr_free_node) {  //find front free slot, merge
      front_adj = cur;
      front_adj->size += ptr_free_node->size + sizeof(free_node_t);
    }
  }
  if (front_adj == NULL) {  //add ptr to free list
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
