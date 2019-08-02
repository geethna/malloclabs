#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Geethna T K",
    /* First member's email address */
    "geethna.teekey@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 4

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

struct block_meta {
  size_t size;
  struct block_meta *next;
  int free;
};

int start_size = 0x1000;
void *global_base = NULL;
struct block_meta *current;

void *request_space(size_t size){
    void *p = sbrk(ALIGN(size));
    if(p == (void*)-1)
        return NULL;
    return p;
}
void *find_space(size_t size){
    current = global_base;
    while(current && !current->free && current->size>=size){
        current = current->next;
    }
    return current;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void *block;
    if(size <= 0)
        return NULL;
    if(!global_base){        //first call to malloc
        block = request_space(size);
        if(!block)
            return NULL;
        global_base = block;
        current = global_base;
        current->size = ALIGN(size);
        current->next = NULL;
        current->free = 0;      //0 indicated in-use
      }
    else{
        block = find_space(size);
        if(!block){
            block = request_space(size);
            if(!block)
                return NULL;
          }
        current->next = block;
        current = current->next;
        current->size = ALIGN(size);
        current->next = NULL;
        current->free = 0;
        }
      return current;
}

/*
 * mm_free - Freeing a block does nothing.
 */

void mm_free(void *ptr)
{
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
