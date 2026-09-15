#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(n) (((n) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define BLOCK_HEADER_SIZE ALIGN(sizeof(block_header_t))

typedef struct block_header {
    size_t size;
    int free;
    struct block_header *next;
} block_header_t;

static void *g_buf;
static size_t g_size;
static block_header_t *g_head;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_head = (block_header_t *)buf;
    g_head->size = size - BLOCK_HEADER_SIZE;
    g_head->free = 0;
    g_head->next = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned = ALIGN(n);
    block_header_t *curr = g_head;
    while (curr) {
        if (curr->free && curr->size >= aligned) {
            curr->free = 0;
            if (curr->size >= aligned + BLOCK_HEADER_SIZE + ALIGNMENT) {
                size_t remaining = curr->size - aligned - BLOCK_HEADER_SIZE;
                block_header_t *next = (block_header_t *)((char *)curr + BLOCK_HEADER_SIZE + aligned);
                next->size = remaining;
                next->free = 1;
                next->next = curr->next;
                curr->size = aligned;
                curr->next = next;
            }
            return (char *)curr + BLOCK_HEADER_SIZE;
        }
        curr = curr->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    block_header_t *block = (block_header_t *)((char *)p - BLOCK_HEADER_SIZE);
    block->free = 1;
    block_header_t *next = block->next;
    if (next && next->free) {
        block->size += BLOCK_HEADER_SIZE + next->size;
        block->next = next->next;
    }
    if (block != g_head) {
        block_header_t *prev = g_head;
        while (prev && prev->next != block) prev = prev->next;
        if (prev && prev->free) {
            prev->size += BLOCK_HEADER_SIZE + block->size;
            prev->next = block->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    size_t aligned = ALIGN(n);
    block_header_t *block = (block_header_t *)((char *)p - BLOCK_HEADER_SIZE);
    if (block->size >= aligned) return p;
    block_header_t *next = block->next;
    if (next && next->free && block->size + BLOCK_HEADER_SIZE + next->size >= aligned) {
        size_t old_size = block->size;
        block->size = aligned;
        block->free = 0;
        block->next = next->next;
        if (block->size < old_size + BLOCK_HEADER_SIZE + next->size - BLOCK_HEADER_SIZE) {
            size_t remaining = old_size + BLOCK_HEADER_SIZE + next->size - BLOCK_HEADER_SIZE - aligned;
            if (remaining >= BLOCK_HEADER_SIZE + ALIGNMENT) {
                block_header_t *new_next = (block_header_t *)((char *)block + BLOCK_HEADER_SIZE + aligned);
                new_next->size = remaining;
                new_next->free = 1;
                new_next->next = block->next;
                block->next = new_next;
            }
        }
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy = block->size < aligned ? block->size : aligned;
        __builtin_memcpy(new_p, p, copy);
        arena_free(p);
    }
    return new_p;
}