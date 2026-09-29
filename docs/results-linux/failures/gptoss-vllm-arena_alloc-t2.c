#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

static void *arena_start = NULL;
static size_t arena_total_size = 0;

typedef struct alignas(max_align_t) block {
    size_t size;   /* size of the whole block including header and footer */
    int    free;   /* 1 = free, 0 = used */
} block;

static inline size_t align_up(size_t n, size_t a)
{
    return (n + a - 1) & ~(a - 1);
}

static inline void set_footer(block *b)
{
    size_t *footer = (size_t *)((char *)b + b->size - sizeof(size_t));
    *footer = b->size;
}

static inline size_t get_footer(block *b)
{
    size_t *footer = (size_t *)((char *)b + b->size - sizeof(size_t));
    return *footer;
}

/* Initialise the arena with a user supplied buffer */
void arena_init(void *buf, size_t size)
{
    if (!buf || size < sizeof(block) + sizeof(size_t))
        return;

    arena_start = buf;
    arena_total_size = size;

    block *b = (block *)arena_start;
    b->size = size;
    b->free = 1;
    set_footer(b);
}

/* Helper to find a free block that fits `need` bytes */
static block *find_fit(size_t need)
{
    block *b = (block *)arena_start;
    while ((char *)b < (char *)arena_start + arena_total_size) {
        if (b->free && b->size >= need)
            return b;
        b = (block *)((char *)b + b->size);
    }
    return NULL;
}

/* Allocate a block of `n` bytes */
void *arena_alloc(size_t n)
{
    if (n == 0 || !arena_start)
        return NULL;

    size_t header_size = sizeof(block);
    size_t total = header_size + n;
    total = align_up(total, alignof(max_align_t));

    block *b = find_fit(total);
    if (!b)
        return NULL;

    /* If the block is significantly larger, split it */
    size_t minimal_free = align_up(sizeof(block) + 1, alignof(max_align_t));
    if (b->size >= total + minimal_free) {
        block *next = (block *)((char *)b + total);
        next->size = b->size - total;
        next->free = 1;
        set_footer(next);

        b->size = total;
        set_footer(b);
    }

    b->free = 0;
    return (void *)((char *)b + header_size);
}

/* Free a previously allocated block */
void arena_free(void *p)
{
    if (!p || !arena_start)
        return;

    block *b = (block *)((char *)p - sizeof(block));
    b->free = 1;

    /* Coalesce with next block if free */
    block *next = (block *)((char *)b + b->size);
    if ((char *)next < (char *)arena_start + arena_total_size && next->free) {
        b->size += next->size;
        set_footer(b);
    }

    /* Coalesce with previous block if free */
    if ((char *)b > (char *)arena_start) {
        size_t prev_size = get_footer((block *)((char *)b - sizeof(size_t) - sizeof(block)));
        block *prev = (block *)((char *)b - prev_size);
        if (prev->free) {
            prev->size += b->size;
            set_footer(prev);
        }
    }
}

/* Reallocate a block to a new size */
void *arena_realloc(void *p, size_t n)
{
    if (!p)
        return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!arena_start)
        return NULL;

    block *b = (block *)((char *)p - sizeof(block));
    size_t old_payload = b->size - sizeof(block);
    size_t new_payload = n;
    size_t header_size = sizeof(block);
    size_t required = header_size + new_payload;
    required = align_up(required, alignof(max_align_t));

    /* Shrink in place if possible */
    if (required <= b->size) {
        size_t minimal_free = align_up(sizeof(block) + 1, alignof(max_align_t));
        if (b->size - required >= minimal_free) {
            block *next = (block *)((char *)b + required);
            next->size = b->size - required;
            next->free = 1;
            set_footer(next);

            b->size = required;
            set_footer(b);
        }
        return p;
    }

    /* Try to grow into next free block */
    block *next = (block *)((char *)b + b->size);
    if ((char *)next < (char *)arena_start + arena_total_size && next->free &&
        b->size + next->size >= required) {
        b->size += next->size;
        set_footer(b);

        /* Possibly split after growing */
        size_t minimal_free = align_up(sizeof(block) + 1, alignof(max_align_t));
        if (b->size - required >= minimal_free) {
            block *newfree = (block *)((char *)b + required);
            newfree->size = b->size - required;
            newfree->free = 1;
            set_footer(newfree);

            b->size = required;
            set_footer(b);
        }
        return p;
    }

    /* Allocate new block */
    void *newp = arena_alloc(n);
    if (!newp)
        return NULL;

    size_t copy_size = old_payload < new_payload ? old_payload : new_payload;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}