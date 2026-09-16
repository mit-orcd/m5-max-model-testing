#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT (alignof(max_align_t))
#define ALIGN_UP(n, a) (((n) + (a) - 1) & ~((a) - 1))

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block_t;

static free_block_t *free_list;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    free_list = NULL;
    
    if (size >= sizeof(free_block_t)) {
        free_block_t *first_block = (free_block_t *)buf;
        first_block->size = size - offsetof(free_block_t, size);
        first_block->next = NULL;
        free_list = first_block;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    n = ALIGN_UP(n, ALIGNMENT);
    
    free_block_t **prev = &free_list;
    free_block_t *current = free_list;
    
    while (current) {
        if (current->size >= n) {
            *prev = current->next;
            
            if (current->size >= n + sizeof(free_block_t)) {
                free_block_t *new_free = (free_block_t *)((char *)current + n);
                new_free->size = current->size - n - offsetof(free_block_t, size);
                new_free->next = NULL;
                free_list = new_free;
            }
            
            return (char *)current + offsetof(free_block_t, size);
        }
        prev = &current->next;
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    free_block_t *block = (free_block_t *)((char *)p - offsetof(free_block_t, size));
    free_block_t **prev = &free_list;
    free_block_t *current = free_list;
    
    while (current && current < block) {
        prev = &current->next;
        current = current->next;
    }
    
    block->next = current;
    *prev = block;
    
    free_block_t *next_block = (free_block_t *)((char *)block + block->size + offsetof(free_block_t, size));
    
    if (current && (char *)current == (char *)next_block) {
        block->size += current->size + offsetof(free_block_t, size);
        block->next = current->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    n = ALIGN_UP(n, ALIGNMENT);
    
    free_block_t *block = (free_block_t *)((char *)p - offsetof(free_block_t, size));
    size_t old_size = block->size;
    
    if (n <= old_size) {
        return p;
    }
    
    free_block_t *next_block = (free_block_t *)((char *)block + old_size + offsetof(free_block_t, size));
    
    if ((char *)next_block == (char *)block + old_size + offsetof(free_block_t, size) &&
        next_block->size >= n - old_size) {
        if (next_block->size >= n - old_size + sizeof(free_block_t)) {
            free_block_t *new_free = (free_block_t *)((char *)block + n);
            new_free->size = next_block->size - (n - old_size) - offsetof(free_block_t, size);
            new_free->next = next_block->next;
            free_list = new_free;
        } else {
            free_list = next_block->next;
        }
        block->size = n;
        return p;
    }
    
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, old_size);
        arena_free(p);
    }
    return new_p;
}