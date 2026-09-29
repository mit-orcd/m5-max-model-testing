```c
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <string.h>

/* Block header – stored inside the arena buffer */
typedef struct block {
    size_t size;          /* payload size */
    int free;             /* 1 = free, 0 = allocated */
    struct block *next;   /* next block in the list */
} alignas(max_align_t) block_t;

/* Global arena state */
static void *arena_buf   = NULL;
static size_t arena_size = 0;
static block_t *arena_start = NULL;

/* Align up to the next multiple of ALIGN */
static size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

/* Initialise the arena with a buffer */
void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_t))
        return;

    arena_buf   = buf;
    arena_size  = size;
    arena_start = (block_t *)buf;

    arena_start->size = size - sizeof(block_t);
    arena_start->free = 1;
    arena_start->next = NULL;
}

/* Allocate n bytes from the arena */
void *arena_alloc(size_t n) {
    if (n == 0 || !arena_start)
        return NULL;

    size_t req = align_up(n, alignof(max_align_t));
    block_t *prev = NULL;
    block_t *curr = arena_start;

    while (curr) {
        if (curr->free && curr->size >= req) {
            /* If the block is large enough to split */
            size_t remaining = curr->size - req;
            if (remaining >= sizeof(block_t) + 1) {
                block_t *next = (block_t *)((char *)curr + sizeof(block_t) + req);
                next->size  = remaining - sizeof(block_t);
                next->free  = 1;
                next->next  = curr->next;
                curr->size  = req;
                curr->next  = next;
            }
            curr->free = 0;
            return (void *)((char *)curr + sizeof(block