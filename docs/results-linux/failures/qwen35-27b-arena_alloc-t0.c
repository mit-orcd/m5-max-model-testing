#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static void *arena_ptr = NULL;
static size_t arena_size = 0;
static block_header_t *arena_first = NULL;
static block_header_t *arena_last = NULL;

static block_header_t *get_header(void *p) {
    return (block_header_t *)((char *)p - sizeof(block_header_t));
}

static void *get_payload(block_header_t *h) {
    return (void *)((char *)h + sizeof(block_header_t));
}

static void merge_with_next(block_header_t *h) {
    if (!h->next || !h->next->is_free) return;
    
    block_header_t *next = h->next;
    block_header_t *next_next = next->next;
    
    h->size += sizeof(block_header_t) + next->size;
    h->next = next_next;
    
    if (next_next) {
        next_next->prev = h;
    } else {
        arena_last = h;
    }
}

static void split_block(block_header_t *h, size_t req_size) {
    size_t total_size = h->size;
    size_t min_split = sizeof(block_header_t) + req_size;
    
    if (total_size - min_split < sizeof(block_header_t)) {
        return;
    }
    
    size_t new_size = total_size - min_split;
    block_header_t *new_block = (block_header_t *)((char *)h + min_size);
    
    new_block->size = new_size;
    new_block->is_free = 1;
    new_block->next = h->next;
    new_block->prev = h;
    
    if (h->next) {
        h->next->prev = new_block;
    } else {
        arena_last = new_block;
    }
    
    h->next = new_block;
    h->size = min_size - sizeof(block_header_t);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_header_t)) {
        arena_ptr = NULL;
        arena_size = 0;
        arena_first = NULL;
        arena_last = NULL;
        return;
    }
    
    arena_ptr = buf;
    arena_size = size;
    
    block_header_t *first = (block_header_t *)buf;
    first->size = size - sizeof(block_header_t);
    first->is_free = 1;
    first->next = NULL;
    first->prev = NULL;
    
    arena_first = first;
    arena_last = first;
}

void *arena_alloc(size_t n) {
    if (!arena_ptr || n == 0) {
        return NULL;
    }
    
    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    size_t total_needed = sizeof(block_header_t) + aligned_n;
    
    block_header_t *curr = arena_first;
    
    while (curr) {
        if (curr->is_free && curr->size >= aligned_n) {
            curr->is_free = 0;
            
            if (curr->size > aligned_n + sizeof(block_header_t)) {
                split_block(curr, aligned_n);
            }
            
            return get_payload(curr);
        }
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_ptr) {
        return;
    }
    
    block_header_t *h = get_header(p);
    
    if ((char *)h < (char *)arena_ptr || (char *)h >= (char *)arena_ptr + arena_size) {
        return;
    }
    
    h->is_free = 1;
    
    merge_with_next(h);
    
    if (h->prev && h->prev->is_free) {
        merge_with_next(h->prev);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!arena_ptr) {
        return NULL;
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    if (!p) {
        return arena_alloc(n);
    }
    
    block_header_t *h = get_header(p);
    size_t old_size = h->size;
    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    
    if (aligned_n <= old_size) {
        return p;
    }
    
    if (h->next && h->next->is_free) {
        size_t combined_size = old_size + sizeof(block_header_t) + h->next->size;
        if (combined_size >= aligned_n) {
            h->size = combined_size - sizeof(block_header_t);
            h->next = h->next->next;
            if (h->next) {
                h->next->prev = h;
            } else {
                arena_last = h;
            }
            return p;
        }
    }
    
    void *new_ptr = arena_alloc(aligned_n);
    if (!new_ptr) {
        return NULL;
    }
    
    size_t copy_size = (n < old_size) ? n : old_size;
    memcpy(new_ptr, p, copy_size);
    
    arena_free(p);
    
    return new_ptr;
}