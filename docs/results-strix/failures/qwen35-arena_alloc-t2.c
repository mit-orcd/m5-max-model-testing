#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#define HEADER_MAGIC 0xABCDEF0123456789ULL

typedef struct header {
    size_t size;
    int free;
    unsigned long long magic;
    struct header *next;
    struct header *prev;
} header_t;

static header_t *head;
static size_t total_size;

static inline size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static inline void* ptr_from_header(header_t *h) {
    return (void*)((char*)h + sizeof(header_t));
}

static inline header_t* header_from_ptr(void *p) {
    return (header_t*)((char*)p - sizeof(header_t));
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(header_t) + align_up(sizeof(header_t), _Alignof(max_align_t))) {
        head = NULL;
        total_size = 0;
        return;
    }
    
    size_t min_block = align_up(sizeof(header_t), _Alignof(max_align_t));
    if (size < min_block) {
        head = NULL;
        total_size = 0;
        return;
    }

    head = (header_t*)buf;
    head->size = size - sizeof(header_t);
    head->free = 1;
    head->magic = HEADER_MAGIC;
    head->next = NULL;
    head->prev = NULL;
    total_size = size - sizeof(header_t);
}

static header_t* find_free_block(size_t n) {
    header_t *curr = head;
    while (curr) {
        if (curr->free && curr->size >= n) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

static void split_block(header_t *h, size_t n) {
    size_t remaining = h->size - n;
    if (remaining < sizeof(header_t) + align_up(sizeof(header_t), _Alignof(max_align_t))) {
        return;
    }
    
    header_t *new_block = (header_t*)((char*)h + sizeof(header_t) + n);
    new_block->size = remaining - sizeof(header_t);
    new_block->free = 1;
    new_block->magic = HEADER_MAGIC;
    new_block->next = h->next;
    new_block->prev = h;
    
    h->size = n;
    if (h->next) {
        h->next->prev = new_block;
    }
    h->next = new_block;
}

static void merge_adjacent_free(header_t *h) {
    if (h->next && h->next->free) {
        header_t *next = h->next;
        h->size += sizeof(header_t) + next->size;
        h->next = next->next;
        if (next->next) {
            next->next->prev = h;
        }
    }
    if (h->prev && h->prev->free) {
        header_t *prev = h->prev;
        prev->size += sizeof(header_t) + h->size;
        prev->next = h->next;
        if (h->next) {
            h->next->prev = prev;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!head) return NULL;

    size_t align = _Alignof(max_align_t);
    size_t aligned_n = align_up(n, align);
    size_t block_size = sizeof(header_t) + aligned_n;

    header_t *block = find_free_block(block_size);
    if (!block) return NULL;

    split_block(block, aligned_n);
    block->free = 0;
    block->magic = HEADER_MAGIC;
    
    return ptr_from_header(block);
}

void arena_free(void *p) {
    if (!p) return;
    if (!head) return;

    header_t *h = header_from_ptr(p);
    if (h->magic != HEADER_MAGIC) return;
    
    h->free = 1;
    merge_adjacent_free(h);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);
    if (!head) return NULL;

    header_t *h = header_from_ptr(p);
    if (h->magic != HEADER_MAGIC) return NULL;

    size_t current_size = h->size - sizeof(header_t);
    size_t align = _Alignof(max_align_t);
    size_t aligned_n = align_up(n, align);
    size_t aligned_old = align_up(current_size, align);
    
    if (n <= current_size) {
        return p;
    }

    if (h->next && h->next->free) {
        size_t next_size = h->next->size;
        size_t combined = sizeof(header_t) + aligned_n + (next_size - sizeof(header_t));
        if (combined <= h->next->size + aligned_n) {
            size_t needed = sizeof(header_t) + aligned_n;
            if (h->next->size >= needed - sizeof(header_t) + (next_size - sizeof(header_t))) {
                size_t old_content = (n < current_size) ? n : current_size;
                void *data = ptr_from_header(h);
                
                header_t *next = h->next;
                header_t *new_h = h;
                new_h->size = aligned_n;
                new_h->next = next->next;
                if (next->next) {
                    next->next->prev = new_h;
                }
                
                memcpy(data, data, old_content);
                return data;
            }
        }
    }

    void *new_block = arena_alloc(n);
    if (!new_block) return NULL;
    
    size_t copy_size = (n < current_size) ? n : current_size;
    memcpy(new_block, p, copy_size);
    arena_free(p);
    return new_block;
}