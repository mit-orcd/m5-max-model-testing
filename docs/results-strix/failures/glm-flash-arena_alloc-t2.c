#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct {
    size_t size;
    struct block_header *next;
} block_header;

typedef struct {
    block_header *head;
    block_header *tail;
    block_header *free_list;
} arena_meta;

static inline uint8_t *align_up(uint8_t *p, size_t alignment) {
    return (uint8_t *)(((uintptr_t)(p) + (alignment - 1)) & ~(alignment - 1));
}

static inline uint8_t *align_down(uint8_t *p, size_t alignment) {
    return (uint8_t *)(((uintptr_t)(p)) & ~(alignment - 1));
}

void arena_init(void *buf, size_t size) {
    arena_meta *meta = (arena_meta *)buf;
    uint8_t *data = (uint8_t *)buf + sizeof(arena_meta);
    uint8_t *end = (uint8_t *)buf + size;

    meta->head = NULL;
    meta->tail = NULL;
    meta->free_list = NULL;

    if (size <= sizeof(arena_meta)) {
        return;
    }

    block_header *head = (block_header *)data;
    size_t usable_size = end - data;
    
    head->size = usable_size;
    head->next = NULL;
    
    meta->head = head;
    meta->tail = head;
    meta->free_list = head;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    arena_meta *meta = (arena_meta *)_arena_base;
    block_header *prev = NULL;
    block_header *curr = meta->free_list;

    size_t aligned_n = align_up(n, _Alignof(max_align_t));

    while (curr != NULL) {
        size_t block_size = curr->size;
        
        if (block_size >= aligned_n) {
            size_t remaining = block_size - aligned_n;
            block_header *new_block = NULL;

            if (remaining >= sizeof(block_header)) {
                new_block = (block_header *)((uint8_t *)curr + aligned_n);
                new_block->size = remaining;
                new_block->next = curr->next;
                
                curr->size = aligned_n;
                curr->next = new_block;
                
                if (curr == meta->free_list) {
                    meta->free_list = new_block;
                }
            } else {
                if (curr == meta->free_list) {
                    meta->free_list = curr->next;
                }
            }

            return (void *)((uint8_t *)curr + sizeof(block_header));
        }

        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }

    arena_meta *meta = (arena_meta *)_arena_base;
    block_header *free_block = (block_header *)((uint8_t *)p - sizeof(block_header));
    
    free_block->next = meta->free_list;
    meta->free_list = free_block;

    arena_coalesce(meta);
}

void arena_coalesce(arena_meta *meta) {
    block_header *curr = meta->free_list;
    block_header *prev = NULL;

    while (curr != NULL) {
        block_header *next = curr->next;
        
        if (next != NULL) {
            uint8_t *curr_end = (uint8_t *)curr + curr->size + sizeof(block_header);
            uint8_t *next_start = (uint8_t *)next + sizeof(block_header);

            if (curr_end == next_start) {
                curr->size += next->size + sizeof(block_header);
                curr->next = next->next;
                
                if (next == meta->free_list) {
                    meta->free_list = curr;
                }
                
                if (prev != NULL) {
                    prev->next = curr->next;
                }
                
                next = curr;
            }
        }

        prev = curr;
        curr = curr->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }

    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    arena_meta *meta = (arena_meta *)_arena_base;
    block_header *block = (block_header *)((uint8_t *)p - sizeof(block_header));
    size_t block_size = block->size;
    size_t aligned_n = align_up(n, _Alignof(max_align_t));

    if (block_size >= aligned_n) {
        return p;
    }

    block_header *next_block = (block_header *)((uint8_t *)block + block_size + sizeof(block_header));
    
    if (next_block != NULL && next_block->size > 0 && next_block->next == NULL) { 
        uint8_t *next_end = (uint8_t *)next_block + next_block->size + sizeof(block_header);
        uint8_t *curr_end = (uint8_t *)block + block_size + sizeof(block_header);

        if (next_end == curr_end) {
            block->size += next_block->size + sizeof(block_header);
            next_block->size = 0;
            arena_coalesce(meta);
            return p;
        }
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr != NULL) {
        size_t copy_size = block_size < aligned_n ? block_size : aligned_n;
        __builtin_memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }

    return new_ptr;
}