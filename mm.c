#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "bi0s",
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
#define ALIGNMENT 8
#define IN_USE 1
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(block_meta)))
#define is_free(size) (size&0x00000000)
#define BLOCK_MEM(ptr) ((void *)((unsigned long)ptr + sizeof(block_meta)))
#define BLOCK_HEADER(ptr) ((void *)((unsigned long)ptr - sizeof(block_t)))
#define get_chunk(p) (p -SIZE_T_SIZE)

typedef struct block_meta {
  int size;
  struct block_meta *next;
}block_meta;

struct block_meta *free_base;
struct block_meta *current;

void *request_space(size_t size){
    void *p = mem_sbrk(ALIGN(size));
    if (p == (void *)-1)
        return NULL;
    return p;
}

void *del(block_meta *ptr){
    if(ptr == free_base){         //if its head
        if(ptr->next){      //if its not the only chunk in the list
            free_base = ptr->next;
        }
        else{               //if its the only chunk in the list
            free_base = NULL;
        }
    }
    else{                   //if its the last chunk
    block_meta *curr = free_base;           //if its in the middle
    while(curr->next != ptr){
        curr = curr->next;
    }
    curr->next = ptr->next;
    }
    return NULL;
}

void *add(block_meta *ptr){
    ptr->next = NULL;
    if(!free_base){
        free_base = ptr;
    }
    else{
        block_meta *curr = free_base;
        while(curr->next && (unsigned long)curr > (unsigned long) ptr){
            curr = curr->next;
        }
        if(ptr != curr->next){
        ptr->next = curr->next;
        curr->next = ptr;
      }
    }
        return NULL;
}

void *split(block_meta *block,int size){
    block_meta *newptr = (block_meta *) ((unsigned long)block + size);
    newptr->size = block->size-size;
    block->size = size;
    return newptr;
}

void *merge(block_meta *ptr){
    unsigned long curr_p = (unsigned long)ptr;
    unsigned long curr_n = (unsigned long)ptr->next;
    if(curr_p + ptr->size == curr_n){
          ptr->size = ptr->size + ptr->next->size;
          del(ptr->next);
      }
    if(ptr == free_base){         //if its head
        return NULL;
    }
    else{
    block_meta *curr = free_base;
    while(curr->next != ptr){
        curr = curr->next;
    }
    unsigned long curr_pr = (unsigned long)curr;
    if(curr_pr + curr->size == curr_p){
          curr->size = curr->size + ptr->size;
          del(curr->next);
      }
    }
        return NULL;
  }

void *find_space(size_t size){
    block_meta *block;
    block = free_base;
    while(block){
        if(block->size >= size){
            del(block);
            if(block->size == size)
                return block;
            block_meta *newptr = split(block,size);
            add(newptr);        //adding the split chunk to the free_list
            return block;     //returning the chunk we got
        }
        else{           //if we did not get the relatable size chunk
          block = block->next;
        }
    }
    return NULL;
}

int mm_init(void)
{
    free_base = NULL;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void *block = NULL;
    if(size<=0)
        return NULL;
    block = find_space(ALIGN(size)+SIZE_T_SIZE);
    if(!block){
        block = request_space(size+SIZE_T_SIZE);
        if(!block)
            return NULL;
    }
    current = (block_meta *)block;
    current->size = ALIGN(size)+SIZE_T_SIZE;
    current->next = NULL;
    return (void *)((char *)current+SIZE_T_SIZE);
}

void mm_free(void *ptr)
{
    ptr = (void *)get_chunk(ptr);
    add(ptr);
    merge(ptr);
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

void *mm_realloc(void *ptr, size_t size)
{
    block_meta *free_ptr;
    if(!ptr)      //realloc should act like malloc
        return mm_malloc(size);
    if(size == 0)
        mm_free(ptr);
    block_meta *b = get_chunk(ptr);
    if(b->size >= size + SIZE_T_SIZE){
      free_ptr = split(b,size+SIZE_T_SIZE);
      add(free_ptr);
      return ptr;
    }
    void *new_ptr;
    new_ptr = mm_malloc(size);
    if(!new_ptr)
        return NULL;
    memcpy(new_ptr,ptr,b->size);
    mm_free(ptr);
    return new_ptr;
}
