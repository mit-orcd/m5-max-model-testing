#include <stddef.h>
#include <stdint.h>
#include <string.h>

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct free_block {
    size_t size;
    struct free_block *next;
} free_block;

static free_block *free_list;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    free_list = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t aligned_n = (n + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);
    size_t total_size = aligned_n + sizeof(size_t);
    
    free_block *prev = NULL;
    free_block *current = free_list;
    
    while (current) {
        if (current->size >= n) {
            if (prev) {
                prev->next = current->next;
            } else {
                free_list = current->next;
            }
            *(size_t*)((char*)current + sizeof(free_block)) = n;
            return (char*)current + sizeof(free_block);
        }
        prev = current;
        current = current->next;
    }
    
    if (arena_used + total_size > arena_size) {
        return NULL;
    }
    
    void *ptr = (char*)arena_buf + arena_used;
    *(size_t*)ptr = n;
    arena_used += total_size;
    return (char*)ptr + sizeof(size_t);
}

void arena_free(void *p) {
    if (!p) return;
    
    char *ptr = (char*)p - sizeof(size_t);
    size_t size = *(size_t*)ptr;
    
    free_block *new_block = (free_block*)ptr;
    new_block->size = size;
    
    free_block *prev = NULL;
    free_block *current = free_list;
    
    while (current && current < new_block) {
        prev = current;
        current = current->next;
    }
    
    free_block *next = new_block->next;
    
    if (prev) {
        prev->next = new_block;
        new_block->next = current;
    } else {
        free_list = new_block;
        new_block->next = current;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    
    char *ptr = (char*)p - sizeof(size_t);
    size_t old_size = *(size_t*)ptr;
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    size_t aligned_n = (n + sizeof(size_t) - 1) & ~(sizeof(size_t) -1);
    size_t total_size = aligned_n + sizeof(size_t);
    
    if (n <= old_size) {
        *(size_t*)ptr = n;
        return p;
    }
    
    char *next_block = (char*)ptr + sizeof(size_t) + old_size;
    free_block *next_free = (free_block*)next_block;
    
    if (next_free->size > 0 && next_free->size >= n - old_size) {
        free_list = next_free->next;
        *(size_t*)ptr = n;
        return p;
    }
    
    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        memcpy(new_ptr, p, old_size);
    }
    arena_free(p);
    return new_ptr;
}