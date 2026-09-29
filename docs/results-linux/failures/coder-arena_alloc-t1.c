#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGNUP(n) (((n) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

struct block {
    size_t size;
    int free;
    struct block *next;
};

static struct block *head = NULL;
static char *arena_start = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(struct block))
        return;
    
    arena_start = (char *)buf;
    arena_size = size;
    
    head = (struct block *)arena_start;
    head->size = size - sizeof(struct block);
    head->free = 1;
    head->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0)
        return NULL;
    
    n = ALIGNUP(n);
    
    struct block *prev = NULL;
    struct block *current = head;
    
    while (current) {
        if (current->free && current->size >= n) {
            if (current->size >= n + sizeof(struct block)) {
                struct block *new_block = (char *)current + sizeof(struct block) + n;
                new_block->size = current->size - n - sizeof(struct block);
                new_block->free = 1;
                new_block->next = current->next;
                current->next = new_block;
                current->size = n;
            }
            current->free = 0;
            return (char *)current + sizeof(struct block);
        }
        prev = current;
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p)
        return;
    
    struct block *block = (char *)p - sizeof(struct block);
    block->free = 1;
    
    struct block *current = head;
    struct block *prev = NULL;
    
    while (current) {
        if (current->free && current->next && current->next->free) {
            current->size += sizeof(struct block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p)
        return arena_alloc(n);
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    n = ALIGNUP(n);
    struct block *block = (char *)p - sizeof(struct block);
    
    if (block->size >= n)
        return p;
    
    struct block *next_block = (char *)block + sizeof(struct block) + block->size;
    
    if (next_block->free && (next_block->size + sizeof(struct block) + block->size) >= n) {
        if (next_block->size >= n - block->size) {
            block->size = n;
            next_block->free = 0;
            return p;
        } else {
            block->size += sizeof(struct block) + next_block->size;
            next_block->free = 0;
            return p;
        }
    }
    
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, block->size);
        arena_free(p);
    }
    return new_p;
}