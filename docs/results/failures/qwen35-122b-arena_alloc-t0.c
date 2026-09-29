#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

static void *arena_buf = NULL;
static size_t arena_size = 0;
static size_t arena_offset = 0;

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

#define HEADER_SIZE (sizeof(block_header_t))
#define ALIGNMENT (alignof(max_align_t))
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_offset = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_buf == NULL) {
        return NULL;
    }

    size_t aligned_n = ALIGN_UP(n);
    size_t total_req = HEADER_SIZE + aligned_n;

    if (arena_offset + total_req > arena_size) {
        return NULL;
    }

    block_header_t *header = (block_header_t *)((char *)arena_buf + arena_offset);
    header->size = aligned_n;
    header->is_free = 0;
    header->next = NULL;
    header->prev = NULL;

    arena_offset += total_req;

    return (void *)((char *)header + HEADER_SIZE);
}

void arena_free(void *p) {
    if (p == NULL || arena_buf == NULL) {
        return;
    }

    block_header_t *header = (block_header_t *)((char *)p - HEADER_SIZE);
    if ((char *)header < (char *)arena_buf || (char *)header >= (char *)arena_buf + arena_size) {
        return;
    }

    header->is_free = 1;

    block_header_t *curr = (block_header_t *)arena_buf;
    block_header_t *prev = NULL;

    while (curr != NULL) {
        if (curr == header) {
            if (prev != NULL && prev->is_free) {
                prev->size += HEADER_SIZE + header->size;
                prev->next = header->next;
                if (header->next != NULL) {
                    header->next->prev = prev;
                }
                header = prev;
            }
            if (header->next != NULL && header->next->is_free) {
                header->size += HEADER_SIZE + header->next->size;
                header->next = header->next->next;
                if (header->next != NULL) {
                    header->next->prev = header;
                }
            }
            break;
        }
        prev = curr;
        curr = (block_header_t *)((char *)curr + HEADER_SIZE + curr->size);
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

    block_header_t *header = (block_header_t *)((char *)p - HEADER_SIZE);
    size_t old_size = header->size;
    size_t aligned_n = ALIGN_UP(n);

    if (old_size >= aligned_n) {
        return p;
    }

    size_t needed_extra = aligned_n - old_size;

    block_header_t *next = (block_header_t *)((char *)header + HEADER_SIZE + old_size);
    if (next != NULL && next->is_free) {
        size_t next_size = next->size;
        if (HEADER_SIZE + next_size >= needed_extra) {
            size_t new_total = old_size + HEADER_SIZE + next_size;
            size_t new_aligned = ALIGN_UP(n);
            size_t remaining = new_total - new_aligned;

            header->size = new_aligned;
            header->next = next->next;
            if (next->next != NULL) {
                next->next->prev = header;
            }

            if (remaining > 0) {
                block_header_t *new_free = (block_header_t *)((char *)header + HEADER_SIZE + new_aligned);
                new_free->size = remaining;
                new_free->is_free = 1;
                new_free->next = header->next;
                new_free->prev = header;
                if (header->next != NULL) {
                    header->next->prev = new_free;
                }
                header->next = new_free;
            }
            return p;
        }
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = (old_size < n) ? old_size : n;
    __builtin_memcpy(new_ptr, p, copy_size);

    arena_free(p);

    return new_ptr;
}