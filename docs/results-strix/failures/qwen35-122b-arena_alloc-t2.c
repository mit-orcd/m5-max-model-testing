#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

static uint8_t *g_buf = NULL;
static size_t g_size = 0;

typedef struct block {
    size_t size;
    int is_free;
    struct block *next;
} block_t;

static inline size_t align_size(size_t n) {
    return (n + sizeof(max_align_t) - 1) & ~(sizeof(max_align_t) - 1);
}

void arena_init(void *buf, size_t size) {
    g_buf = (uint8_t *)buf;
    g_size = size;
    if (g_size < sizeof(block_t)) {
        g_size = 0;
        return;
    }
    
    block_t *header = (block_t *)g_buf;
    header->size = g_size - sizeof(block_t);
    header->is_free = 1;
    header->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_size == 0) return NULL;
    
    size_t req = align_size(n);
    size_t header_size = align_size(sizeof(block_t));
    if (req + header_size > g_size) return NULL;

    block_t *curr = (block_t *)g_buf;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= req) {
            size_t rem = curr->size - req - header_size;
            
            if (rem >= sizeof(block_t)) {
                block_t *new_block = (block_t *)((uint8_t *)curr + header_size + req);
                new_block->size = rem - sizeof(block_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                
                curr->size = req;
                curr->is_free = 0;
                curr->next = new_block;
            } else {
                curr->is_free = 0;
            }
            return (void *)((uint8_t *)curr + header_size);
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    if (g_size == 0) return;

    size_t header_size = align_size(sizeof(block_t));
    if ((uint8_t *)p < g_buf + header_size || (uint8_t *)p >= g_buf + g_size) return;

    block_t *curr = (block_t *)((uint8_t *)p - header_size);
    curr->is_free = 1;

    // Merge with next if free
    if (curr->next != NULL && curr->next->is_free) {
        curr->size += sizeof(block_t) + curr->next->size;
        curr->next = curr->next->next;
    }

    // Merge with previous if free
    block_t *prev = (block_t *)g_buf;
    while (prev->next != NULL && prev->next != curr) {
        prev = prev->next;
    }
    if (prev != curr && prev->is_free) {
        prev->size += sizeof(block_t) + curr->size;
        prev->next = curr->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) {
        return arena_alloc(n);
    }

    size_t header_size = align_size(sizeof(block_t));
    block_t *curr = (block_t *)((uint8_t *)p - header_size);
    
    size_t old_usable = curr->size;
    size_t req = align_size(n);
    
    // Check if we can extend into the next block
    if (curr->next != NULL && curr->next->is_free) {
        size_t total_avail = old_usable + sizeof(block_t) + curr->next->size;
        if (total_avail >= req) {
            // Merge next into current
            size_t new_size = total_avail - req;
            if (new_size >= sizeof(block_t)) {
                block_t *new_next = (block_t *)((uint8_t *)curr + header_size + req);
                new_next->size = new_size - sizeof(block_t);
                new_next->is_free = 1;
                new_next->next = curr->next->next;
                curr->size = req;
                curr->next = new_next;
            } else {
                curr->size = total_avail;
                curr->next = NULL;
            }
            return p;
        }
    }

    // Need to allocate new block
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    size_t copy_size = (n < old_usable) ? n : old_usable;
    memcpy(new_p, p, copy_size);
    
    arena_free(p);
    return new_p;
}