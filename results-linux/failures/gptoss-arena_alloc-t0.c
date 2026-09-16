#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;                     // LSB 0 = free, 1 = allocated
    struct block_header *prev_free;  // valid only if free
    struct block_header *next_free;  // valid only if free
} _Alignas(max_align_t) block_header;

static void *arena_start = NULL;
static size_t arena_size = 0;
static char *arena_end = NULL;
static block_header *free_list_head = NULL;

static void remove_free(block_header *b) {
    if (b->prev_free) b->prev_free->next_free = b->next_free;
    else free_list_head = b->next_free;
    if (b->next_free) b->next_free->prev_free = b->prev_free;
    b->prev_free = b->next_free = NULL;
}

static void insert_free(block_header *b) {
    b->prev_free = NULL;
    b->next_free = free_list_head;
    if (free_list_head) free_list_head->prev_free = b;
    free_list_head = b;
}

void arena_init(void *buf, size_t size) {
    uintptr_t ptr = (uintptr_t)buf;
    size_t align = alignof(max_align_t);
    size_t offset = (align - (ptr & (align - 1))) & (align - 1);
    if (offset >= size) {
        arena_start = NULL;
        arena_size = 0;
        arena_end = NULL;
        free_list_head = NULL;
        return;
    }
    arena_start = (void *)(ptr + offset);
    arena_size = size - offset;
    arena_end = (char *)arena_start + arena_size;
    if (arena_size < sizeof(block_header) + sizeof(size_t)) {
        arena_start = NULL;
        arena_size = 0;
        arena_end = NULL;
        free_list_head = NULL;
        return;
    }
    block_header *initial = (block_header *)arena_start;
    initial->size = arena_size;          // free
    initial->prev_free = NULL;
    initial->next_free = NULL;
    size_t *footer = (size_t *)((char *)initial + arena_size - sizeof(size_t));
    *footer = arena_size;
    free_list_head = initial;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !free_list_head) return NULL;
    size_t align = alignof(max_align_t);
    size_t n_aligned = (n + align - 1) & ~(align - 1);
    size_t total_needed = n_aligned + sizeof(block_header) + sizeof(size_t);

    block_header *b = free_list_head;
    while (b && b->size < total_needed) b = b->next_free;
    if (!b) return NULL;

    remove_free(b);
    size_t bsize = b->size;
    if (bsize - total_needed >= sizeof(block_header) + sizeof(size_t) + 1) {
        // split
        block_header *new_free = (block_header *)((char *)b + total_needed);
        size_t new_size = bsize - total_needed;
        new_free->size = new_size;
        new_free->prev_free = NULL;
        new_free->next_free = NULL;
        size_t *new_footer = (size_t *)((char *)new_free + new_size - sizeof(size_t));
        *new_footer = new_size;
        insert_free(new_free);

        b->size = total_needed | 1;
        size_t *footer = (size_t *)((char *)b + total_needed - sizeof(size_t));
        *footer = total_needed;
    } else {
        // allocate whole block
        b->size = bsize | 1;
        size_t *footer = (size_t *)((char *)b + bsize - sizeof(size_t));
        *footer = bsize;
    }
    return (void *)((char *)b + sizeof(block_header));
}

void arena_free(void *p) {
    if (!p) return;
    block_header *header = (block_header *)((char *)p - sizeof(block_header));
    size_t block_size = header->size & ~((size_t)1);
    header->size = block_size;
    size_t *footer = (size_t *)((char *)header + block_size - sizeof(size_t));
    *footer = block_size;

    // coalesce with previous block
    if ((char *)header > arena_start) {
        size_t *prev_footer = (size_t *)((char *)header - sizeof(size_t));
        size_t prev_size = *prev_footer;
        block_header *prev_header = (block_header *)((char *)header - prev_size);
        if ((prev_header->size & 1) == 0) {
            remove_free(prev_header);
            prev_header->size += block_size;
            size_t *new_footer = (size_t *)((char *)prev_header + prev_header->size - sizeof(size_t));
            *new_footer = prev_header->size;
            header = prev_header;
            block_size = prev_header->size;
        }
    }

    // coalesce with next block
    if ((char *)header + block_size < arena_end) {
        block_header *next_header = (block_header *)((char *)header + block_size);
        if ((next_header->size & 1) == 0) {
            remove_free(next_header);
            header->size += next_header->size;
            size_t *new_footer = (size_t *)((char *)header + header->size - sizeof(size_t));
            *new_footer = header->size;
            block_size = header->size;
        }
    }

    insert_free(header);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header *header = (block_header *)((char *)p - sizeof(block_header));
    size_t old_block_size = header->size & ~((size_t)1);
    size_t old_payload_size = old_block_size - sizeof(block_header) - sizeof(size_t);

    size_t align = alignof(max_align_t);
    size_t n_aligned = (n + align - 1) & ~(align - 1);
    size_t new_total_needed = n_aligned + sizeof(block_header) + sizeof(size_t);

    if (new_total_needed <= old_block_size) {
        // shrink in place
        if (old_block_size - new_total_needed >= sizeof(block_header) + sizeof(size_t) + 1) {
            block_header *new_free = (block_header *)((char *)header + new_total_needed);
            size_t new_size = old_block_size - new_total_needed;
            new_free->size = new_size;
            new_free->prev_free = NULL;
            new_free->next_free = NULL;
            size_t *new_footer = (size_t *)((char *)new_free + new_size - sizeof(size_t));
            *new_footer = new_size;
            header->size = new_total_needed | 1;
            size_t *footer = (size_t *)((char *)header + new_total_needed - sizeof(size_t));
            *footer = new_total_needed;
            insert_free(new_free);
        } else {
            header->size = old_block_size | 1;
            size_t *footer = (size_t *)((char *)header + old_block_size - sizeof(size_t));
            *footer = old_block_size;
        }
        return p;
    }

    // try to grow into next free block
    block_header *next_header = (block_header *)((char *)header + old_block_size);
    if ((char *)next_header < arena_end && (next_header->size & 1) == 0) {
        size_t combined_size = old_block_size + next_header->size;
        if (combined_size >= new_total_needed) {
            remove_free(next_header);
            header->size = combined_size | 1;
            size_t *footer = (size_t *)((char *)header + combined_size - sizeof(size_t));
            *footer = combined_size;
            return p;
        }
    }

    // allocate new block
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    size_t copy_size = old_payload_size < n ? old_payload_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}