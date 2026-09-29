#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT (alignof(max_align_t))
#define ALIGNUP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

typedef struct block {
    size_t size;
    int free;
} block;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    if (size >= sizeof(block)) {
        block *b = (block *)buf;
        b->size = size - sizeof(block);
        b->free = 1;
        arena_used = sizeof(block);
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t req_size = ALIGNUP(n + sizeof(block));
    block *best = NULL;
    block *current = (block *)arena_buf;
    char *end = (char *)arena_buf + arena_size;
    
    while ((char *)current < end) {
        if (current->free && current->size >= req_size) {
            if (!best || current->size < best->size) {
                best = current;
            }
        }
        if ((char *)current + current->size >= end) break;
        current = (block *)((char *)current + current->size);
    }
    
    if (!best) return NULL;
    
    best->free = 0;
    if (best->size >= req_size + sizeof(block)) {
        block *next = (block *)((char *)best + req_size);
        next->size = best->size - req_size;
        next->free = 1;
        best->size = req_size;
    }
    
    return (char *)best + sizeof(block);
}

void arena_free(void *p) {
    if (!p) return;
    
    block *b = (block *)((char *)p - sizeof(block));
    b->free = 1;
    
    char *end = (char *)arena_buf + arena_size;
    block *current = (block *)arena_buf;
    
    while ((char *)current < end) {
        if (current->free && (char *)current + current->size == (char *)current + sizeof(block)) {
            block *next = (block *)((char *)current + current->size);
            if ((char *)next < end && next->free) {
                current->size += next->size;
                current = next;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block *b = (block *)((char *)p - sizeof(block));
    size_t old_size = b->size - sizeof(block);
    size_t new_size = ALIGNUP(n + sizeof(block));
    
    if (new_size <= b->size) {
        if (new_size < old_size) {
            block *next = (block *)((char *)b + b->size);
            if ((char *)next < (char *)arena_buf + arena_size && next->free) {
                if (next->size >= new_size - b->size + sizeof(block)) {
                    next->size -= new_size - b->size;
                    b->size = new_size;
                    return p;
                }
            }
        }
        return p;
    }
    
    block *next = (block *)((char *)b + b->size);
    if ((char *)next < (char *)arena_buf + arena_size && next->free && 
        next->size >= new_size - b->size) {
        b->size = new_size;
        return p;
    }
    
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, old_size < n ? old_size : n);
        arena_free(p);
    }
    return new_p;
}