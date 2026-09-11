#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct block block_t;
struct block {
    block_t *next;
    size_t size;
};

static struct {
    block_t *free_list;
    char *buffer;
    size_t buffer_size;
} arena;

void arena_init(void *buf, size_t size) {
    arena.buffer = buf;
    arena.buffer_size = size;
    arena.free_list = (block_t *)buf;
    if (buf) {
        arena.free_list->next = NULL;
        arena.free_list->size = ALIGN(size - sizeof(block_t));
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t total_size = sizeof(block_t) + ALIGN(n);
    block_t *prev = NULL;
    block_t *curr = arena.free_list;
    
    while (curr) {
        if (curr->size >= total_size) break;
        prev = curr;
        curr = curr->next;
    }
    
    if (!curr) return NULL;
    
    if (curr->size >= total_size + ALIGN(sizeof(block_t))) {
        block_t *new_block = (block_t *)((char *)curr + sizeof(block_t) + ALIGN(curr->size - sizeof(block_t) - ALIGN(sizeof(block_t))));
        new_block->size = curr->size - sizeof(block_t) - ALIGN(sizeof(block_t));
        new_block->next = curr->next;
        curr->size = sizeof(block_t) + ALIGN(n);
        curr->next = new_block;
    }
    
    if (prev) prev->next = curr->next;
    else arena.free_list = curr->next;
    
    return (void *)((char *)curr + sizeof(block_t));
}

void arena_free(void *p) {
    if (!p) return;
    
    block_t *block = (block_t *)((char *)p - sizeof(block_t));
    block->next = arena.free_list;
    arena.free_list = block;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    if (!p) return arena_alloc(n);
    
    block_t *block = (block_t *)((char *)p - sizeof(block_t));
    size_t old_size = block->size - sizeof(block_t);
    size_t total_size = sizeof(block_t) + ALIGN(n);
    
    block_t *next = arena.free_list;
    while (next) {
        if ((char *)next == (char *)p + block->size) {
            if (block->size + next->size >= total_size) {
                block->size += next->size;
                if (block->size > total_size + ALIGN(sizeof(block_t))) {
                    block_t *new_block = (block_t *)((char *)block + sizeof(block_t) + ALIGN(block->size - sizeof(block_t) - ALIGN(sizeof(block_t))));
                    new_block->size = block->size - sizeof(block_t) - ALIGN(block->size - sizeof(block_t) - ALIGN(sizeof(block_t)));
                    new_block->next = next->next;
                    block->size = sizeof(block_t) + ALIGN(n);
                    block->next = new_block;
                } else {
                    block->next = next->next;
                }
                return p;
            }
            break;
        }
        next = next->next;
    }
    
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    
    char *src = (char *)p;
    size_t copy_size = old_size < n ? old_size : n;
    for (size_t i = 0; i < copy_size; i++) {
        ((char *)new_p)[i] = src[i];
    }
    
    arena_free(p);
    return new_p;
}