#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT (alignof(max_align_t))
#define ALIGNUP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct FreeBlock {
    size_t size;
    struct FreeBlock *next;
} FreeBlock;

static FreeBlock *free_list;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    n = ALIGNUP(n);
    
    FreeBlock *prev = NULL;
    FreeBlock *current = free_list;
    
    while (current) {
        if (current->size >= n) {
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }
            
            if (current->size > n + sizeof(FreeBlock)) {
                FreeBlock *new_free = (char*)current + n;
                new_free->size = current->size - n;
                new_free->next = NULL;
                free_list = new_free;
            }
            
            return (char*)current + sizeof(FreeBlock);
        }
        prev = current;
        current = current->next;
    }
    
    if (arena_used + n + sizeof(FreeBlock) > arena_size) {
        return NULL;
    }
    
    FreeBlock *block = (char*)arena_buf + arena_used;
    block->size = n;
    block->next = NULL;
    arena_used += n + sizeof(FreeBlock);
    
    return (char*)block + sizeof(FreeBlock);
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    FreeBlock *block = (char*)p - sizeof(FreeBlock);
    FreeBlock *prev = NULL;
    FreeBlock *current = free_list;
    
    while (current && current < block) {
        prev = current;
        current = current->next;
    }
    
    if (prev) {
        prev->next = block;
    } else {
        free_list = block;
    }
    
    block->next = current;
    
    FreeBlock *next = (char*)block + block->size + sizeof(FreeBlock);
    if ((char*)next < (char*)arena_buf + arena_used && 
        (char*)next + sizeof(FreeBlock) <= (char*)arena_buf + arena_size) {
        FreeBlock *next_block = (FreeBlock*)next;
        if (next_block->size != 0) {
            block->size += next_block->size + sizeof(FreeBlock);
            block->next = next_block->next;
        }
    }
    
    if (prev && (char*)prev + prev->size + sizeof(FreeBlock) == (char*)block) {
        prev->size += block->size + sizeof(FreeBlock);
        prev->next = block->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    FreeBlock *block = (char*)p - sizeof(FreeBlock);
    size_t old_size = block->size;
    
    if (n <= old_size) {
        return p;
    }
    
    FreeBlock *next = (char*)block + old_size + sizeof(FreeBlock);
    if ((char*)next < (char*)arena_buf + arena_used && 
        (char*)next + sizeof(FreeBlock) <= (char*)arena_buf + arena_size) {
        FreeBlock *next_block = (FreeBlock*)next;
        if (next_block->size != 0 && next_block->size >= n - old_size) {
            if (next_block->size > n - old_size + sizeof(FreeBlock)) {
                FreeBlock *new_free = (char*)next + n - old_size;
                new_free->size = next_block->size - (n - old_size);
                new_free->next = next_block->next;
                next_block->size = n - old_size;
            } else {
                next_block->size = 0;
            }
            return p;
        }
    }
    
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, old_size);
        arena_free(p);
    }
    return new_p;
}