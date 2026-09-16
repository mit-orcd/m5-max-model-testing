#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE (ALIGN_UP(sizeof(Header)))

typedef struct Header {
    size_t size;      /* Total size of block (header + payload), including alignment */
    int free;         /* 1 if free, 0 if allocated */
    struct Header *next;
    struct Header *prev;
} Header;

static Header *free_list = NULL;
static uint8_t *arena_start = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < MIN_BLOCK_SIZE) {
        free_list = NULL;
        arena_start = NULL;
        arena_size = 0;
        return;
    }

    arena_start = (uint8_t *)buf;
    arena_size = size;

    Header *h = (Header *)arena_start;
    h->size = arena_size;
    h->free = 1;
    h->next = NULL;
    h->prev = NULL;

    free_list = h;
}

static Header *find_free_block(size_t n) {
    Header *curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= n) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

static void remove_from_free_list(Header *block) {
    if (block->prev) block->prev->next = block->next;
    else free_list = block->next;

    if (block->next) block->next->prev = block->prev;
}

static void add_to_free_list(Header *block) {
    block->next = free_list;
    block->prev = NULL;
    if (free_list) free_list->prev = block;
    free_list = block;
    block->free = 1;
}

static void coalesce_with_next(Header *block) {
    if (!block->next || !block->next->free) return;

    Header *next = block->next;
    block->size += next->size;

    if (next->next) next->next->prev = block;
    block->next = next->next;
}

static void coalesce_with_prev(Header *block) {
    if (!block->prev || !block->prev->free) return;

    Header *prev = block->prev;
    prev->size += block->size;

    if (block->next) block->next->prev = prev;
    prev->next = block->next;

    /* If block was head of free list, update */
    if (free_list == block) free_list = prev;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t total_size = ALIGN_UP(n) + sizeof(Header);
    if (total_size < MIN_BLOCK_SIZE) total_size = MIN_BLOCK_SIZE;

    Header *block = find_free_block(total_size);
    if (!block) return NULL;

    remove_from_free_list(block);

    if (block->size >= total_size + MIN_BLOCK_SIZE) {
        /* Split block */
        Header *new_block = (Header *)((uint8_t *)block + total_size);
        new_block->size = block->size - total_size;
        new_block->free = 1;
        new_block->next = block->next;
        new_block->prev = block;

        if (block->next) block->next->prev = new_block;
        block->next = new_block;

        block->size = total_size;
    }

    block->free = 0;
    return (void *)((uint8_t *)block + sizeof(Header));
}

void arena_free(void *p) {
    if (!p) return;

    Header *block = (Header *)((uint8_t *)p - sizeof(Header));
    if ((uint8_t *)block < arena_start || (uint8_t *)block >= arena_start + arena_size) {
        return; /* Invalid pointer, ignore as per spec */
    }

    block->free = 1;
    add_to_free_list(block);

    coalesce_with_prev(block);
    coalesce_with_next(block);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    Header *block = (Header *)((uint8_t *)p - sizeof(Header));
    if ((uint8_t *)block < arena_start || (uint8_t *)block >= arena_start + arena_size) {
        return NULL; /* Invalid pointer */
    }

    size_t needed = ALIGN_UP(n) + sizeof(Header);
    if (needed < MIN_BLOCK_SIZE) needed = MIN_BLOCK_SIZE;

    size_t old_payload = block->size - sizeof(Header);
    size_t copy_len = (n < old_payload) ? n : old_payload;

    /* Try to grow into next free block */
    Header *next = block->next;
    if (next && next->free && (block->size + next->size) >= needed) {
        /* Merge with next */
        remove_from_free_list(next);
        block->size += next->size;
        if (next->next) next->next->prev = block;
        block->next = next->next;

        /* If there's enough space after growth, split */
        if (block->size >= needed + MIN_BLOCK_SIZE) {
            Header *new_block = (Header *)((uint8_t *)block + needed);
            new_block->size = block->size - needed;
            new_block->free = 1;
            new_block->next = block->next;
            new_block->prev = block;

            if (block->next) block->next->prev = new_block;
            block->next = new_block;

            block->size = needed;
        }

        block->free = 0;
        memcpy(p, p, copy_len); /* No-op, but explicit for clarity */
        return p;
    }

    /* Otherwise, allocate new block and copy */
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, p, copy_len);
    arena_free(p);
    return new_ptr;
}