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

void arena_free(void *p) {
    if (!p) return;
    
    size_t *p_ptr = (size_t*)p - 1;
    size_t size = *p_ptr;
    
    free_block *new_free = (free_block*)p;
    new_free->size = size;
    
    free_block **current = &free_list;
    free_block **prev = NULL;
    
    while (*current && (char*)*current > (char*)p) {
        prev = current;
        current = &(*current)->next;
    }
    
    free_block *new_block = new_free;
    free_block *next_block = NULL;
    
    if (current && (char*)*current == (char*)p + size) {
        next_block = (free_block*)((char*)p + size);
        new_block->size += next_block->size;
        new_block->next = next_block->next;
    } else {
        new_block->next = *current;
    }
    
    if (prev) {
        (*prev)->next = new_block;
    } else {
        free_list = new_block;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t aligned_n = (n + sizeof(size_t) - 1) & ~(sizeof(size_t)-1);
    size_t total_size = aligned_n + sizeof(size_t);
    
    free_block **current = &free_list;
    free_block **prev = NULL;
    
    while (*current) {
        if ((*current)->size >= total_size) {
            free_block *found = *current;
            *next = found->next;
            
            size_t *ptr = (size_t*)found + 1;
            *ptr = total_size;
            
            return (void*)(ptr + 1);
        }
        current = &(*current)->next;
    }
    
    if (arena_used + total_size > arena_size) {
        return NULL;
    }
    
    size_t *ptr = (size_t*)((char*)arena_buf + arena_used);
    *ptr = total_size;
    arena_used += total_size;
    
    return (void*)(ptr + 1);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    
    size_t *p_ptr = (size_t*)p - 1;
    size_t old_size = *p_ptr;
    size_t aligned_n = (n + sizeof(size_t) - 1) & ~(sizeof(size_t)-1);
    
    if (aligned_n <= old_size) {
        return p;
    }
    
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }
    
    memcpy(new_p, p, old_size - sizeof(size_t));
    arena_free(p);
    
    return new_p;
}