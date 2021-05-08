#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "implicit.h"

/*
 * Determine whether or not a block is in use.
 */
static int block_is_in_use(void *block_start) {

  return 1 & *((int *) block_start);
}

/*
 * Return the size of a block.
 */
static int get_block_size(void *block_start) {

  int *header = block_start;
  // This is equivalent to the value of the header, but setting the
  // last two bits to 0 (i.e., the value of the header without the
  // "in-use" bit).
  return -HEADER_SIZE & *header;
}

/*
 * Return the size of the payload of a block.
 */
static int get_payload_size(void *block_start) {
  
  return get_block_size(block_start) - HEADER_SIZE * 2;
}

/*
 * Find the start of the block, given a pointer to the payload.
 */
static void *get_block_start(void *payload) {
  
  return payload - HEADER_SIZE;
}

/*
 * Find the payload, given a pointer to the start of the block.
 */
static void *get_payload(void *block_start) {
  
  return block_start + HEADER_SIZE;
}

/*
 * Set the size of a block, and whether or not it is in use. Remember
 * each block has two copies of the header (one at each end).
 */
static void set_block_header(void *block_start, int block_size, int in_use) {
  
  int header_value = block_size | in_use;
  int *header_position = block_start;
  int *trailer_position = block_start + block_size - HEADER_SIZE;
  *header_position  = header_value;
  *trailer_position = header_value;
}


/*
 * Find the start of the next block.
 */
static void *get_next_block(void *block_start) {
  
  return block_start + get_block_size(block_start);
}

/*
 * Find the start of the previous block.
 */
static void *get_previous_block(void *block_start) {
  
  return block_start - get_block_size(block_start - HEADER_SIZE);
}

/*
 * Determine whether or not the given block is at the front of the heap.
 */
static int is_first_block(struct heap *h, void *block_start) {
  
  return block_start == h->start;
}

/*
 * Determine whether or not the given block is at the end of the heap.
 */
static int is_last_block(struct heap *h, void *block_start) {
  
  return get_next_block(block_start) == h->start + h->size;
}

/*
 * Determine whether or not the given address is inside the heap
 * region. Can be used to loop through all blocks:
 *
 * for (blk = h->start; is_within_heap_range(h, blk); blk = get_next_block(blk)) ...
 */
static int is_within_heap_range(struct heap *h, void *addr) {
  
  return addr >= h->start && addr < h->start + h->size;
}

/*
 * Coalesce (joins) a block with its consecutive (next) block, but
 * only if both blocks are free. Return a pointer to the beginning of
 * the coalesced block.
 */
static void *coalesce(struct heap *h, void *first_block_start) {
  
  if (!is_last_block(h, first_block_start) && 
      !(block_is_in_use(first_block_start)) &&
      !(block_is_in_use(get_next_block(first_block_start)))) {
        set_block_header(first_block_start, get_block_size(first_block_start) + 
        get_block_size(get_next_block(first_block_start)), 0);
      }

  return first_block_start;
}

/*
 * Determine the size of the block we need to allocate given the size
 * the user requested. Don't forget we need space for the header and
 * footer, and that the block's actual payload size must be a multiple
 * of HEADER_SIZE.
 */
static int get_size_to_allocate(int user_size) {
  // round user_size up to nearest multiple of HEADER_SIZE
  // first add (HEADER_SIZE - 1) to user_size, then round down to nearest multiple
  // to round down to nearest multiple of HEADER_SIZE (4), first shift right by 2 bits,
  // then shift left by 2 bits
  int payload_size = ((user_size + (HEADER_SIZE - 1)) >> 2) << 2;

  return payload_size + 2*HEADER_SIZE;
}

/*
 * Checks if the block can be split. It can split if the left over
 * bytes after the split (current block size minus needed size) are
 * enough for a new block, i.e., there is enough space left over for a
 * header, trailer and some payload (i.e., at least 3 times
 * HEADER_SIZE). If it can be split, splits the block in two and marks
 * the first as in use, the second as free. Otherwise just marks the
 * block as in use. Returns the payload of the block marked as in use.
 */
static void *split_and_mark_used(struct heap *h, void *block_start, int needed_size) {
  if ((get_block_size(block_start) - needed_size) >= 3*HEADER_SIZE) {
    set_block_header(block_start + needed_size, get_block_size(block_start) - needed_size, 0);
    set_block_header(block_start, needed_size, 1); 
  }
  else set_block_header(block_start, get_block_size(block_start), 1);  

  return block_start + HEADER_SIZE;
}

/*
 * Create a heap that is "size" bytes large.
 */
struct heap *heap_create(unsigned int size)
{
  /* Allocate space in the process' actual heap */
  void *heap_start = sbrk(size);
  if (heap_start == (void *) -1) return NULL;
  
  /* Use the first part of the allocated space for the heap header */
  struct heap *h = heap_start;
  h->size = size - sizeof(struct heap);
  h->start = heap_start + sizeof(struct heap);

  /* Initializes one big block with the full space in the heap. Will
     be split in the first malloc. */
  set_block_header(h->start, h->size, 0);
  return h;
}

/*
 * Free a block on the heap h. Also attempts to join (coalesce) the
 * block with the previous and the next block, if they are also free.
 */
void myfree(struct heap *h, void *payload) {
  void *block_start = get_block_start(payload);
  set_block_header(block_start, get_block_size(block_start), 0);

  if (!is_last_block(h, block_start) && is_within_heap_range(h, get_next_block(block_start))
  && (!block_is_in_use(get_next_block(block_start)))) {
      block_start = coalesce(h, block_start);
  }
  
  if (!is_first_block(h, block_start) && is_within_heap_range(h, get_previous_block(block_start)) 
  && (!block_is_in_use(get_previous_block(block_start)))) {
      block_start = coalesce(h, get_previous_block(block_start));
  }
}

/*
 * Malloc a block on the heap h, using implicit free list code. Return
 * a pointer to the block's payload in case of success, or NULL if no
 * block large enough to satisfy the request exists.
 */
void *mymalloc(struct heap *h, unsigned int user_size) {
  void *mallocBlock;

  for (void *blk = h->start; is_within_heap_range(h, blk); blk = get_next_block(blk)) {
    if (!block_is_in_use(blk) && get_block_size(blk) >= get_size_to_allocate(user_size)) {
      mallocBlock = split_and_mark_used(h, blk, get_size_to_allocate(user_size));
      return mallocBlock; 
    }
  }
  return NULL;
}
