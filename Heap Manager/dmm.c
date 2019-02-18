#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev;
  bool unalloc;
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency
 */
static metadata_t* freelist = NULL;

void* dmalloc(size_t numbytes) {
  /* initialize through mmap call first time */
  if(freelist == NULL) {
    if(!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);

  numbytes = ALIGN(numbytes);
  metadata_t* header = freelist;
  int alloc_memory = numbytes + METADATA_T_ALIGNED;
  void* ptr = NULL;

  while(alloc_memory > header->size || header->unalloc == false){
    header = header-> next;
    if (header == NULL){
      return NULL;
    }
    // First-fit
  }

  ptr = (void*) header;
  ptr += alloc_memory;

  metadata_t* after_split = (metadata_t*) ptr;
  after_split->size = header->size - alloc_memory;
  after_split->next = header->next;
  after_split->prev = header;
  after_split->unalloc = true;

  if(header->next != NULL){
    after_split->next->prev = after_split;
  }
  // Split remaining blocks

  header->size = numbytes;
  header->next = after_split;
  header->unalloc = false;
  // New header

  void* retptr = NULL;

  retptr = (void*) header;
  retptr += METADATA_T_ALIGNED;

  return retptr;

}

void dfree(void* ptr) {
  ptr -= METADATA_T_ALIGNED;
  metadata_t* header = (metadata_t*) ptr;
  header->unalloc = true;

  if(header->next != NULL && header->next->unalloc == true){
    header->size += header->next->size + METADATA_T_ALIGNED;
    if(header->next->next != NULL){
      header->next->next->prev = header;
    }
    header->next = header->next->next;
    // Coallesce if next block is free
  }
  if(header->prev != NULL && header->prev->unalloc == true){
    header->prev->size += header->size + METADATA_T_ALIGNED;
    if(header->next != NULL){
      header->next->prev = header->prev;
    }
    header->prev->next = header->next;
  }
  // Coallesce if prev block is free
}

bool dmalloc_init() {
  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  // freelist = (metadata_t*) sbrk(max_bytes); // returns heap_region, which is initialized to freelist

  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  freelist->unalloc = true;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
