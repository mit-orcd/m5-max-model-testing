#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

struct block {
    size_t size;            /* least significant bit = free flag */
    struct block *next;     /* valid only when the block is free */
};

static void *arena_start;
static size_t arena_size;
static char *arena_end;
static struct block *free_list_head;

static const size_t ALIGNMENT   = _Alignof(max_align_t);
static const size_t HEADER_SIZE = sizeof(struct block);
static const size_t FOOTER_SIZE = sizeof(size_t);
static const size_t MIN_BLOCK_SIZE = HEADER_SIZE + FOOTER_SIZE + ALIGNMENT;
static const size_t FREE_FLAG = ((size_t)1);

/* Remove a block from the free list */
static void free_remove(struct block *b)
{
    struct block *prev = NULL;
    struct block *cur  = free_list_head;
    while (cur) {
        if (cur == b) {
            if (prev) prev->next = cur->next;
            else      free_list_head = cur->next;
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}

/* Initialise the arena with a user‑supplied buffer */
void arena_init(void *buf, size_t size)
{
    arena_start = buf;
    arena_size  = size;
    arena_end   = (char *)arena_start + arena_size;

    struct block *header = (struct block *)arena_start;
    header->size = arena_size | FREE_FLAG;
    header->next = NULL;

    size_t *footer = (size_t *)((char *)header + arena_size - FOOTER_SIZE);
    *footer = header->size;

    free_list_head = header;
}

/* Allocate a block of at least n bytes */
void *arena_alloc(size_t n)
{
    if (n == 0) return NULL;

    size_t aligned_payload = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t total_size = HEADER_SIZE + aligned_payload + FOOTER_SIZE;

    struct block *prev = NULL;
    struct block *cur  = free_list_head;

    while (cur) {
        size_t cur_size = cur->size & ~FREE_FLAG;
        if (cur_size >= total_size) {
            size_t remaining = cur_size - total_size;

            if (remaining >= MIN_BLOCK_SIZE) {
                /* split the block */
                struct block *next_free = (struct block *)((char *)cur + total_size);
                next_free->size = remaining | FREE_FLAG;
                next_free->next = cur->next;

                size_t *footer_next = (size_t *)((char *)next_free + remaining - FOOTER_SIZE);
                *footer_next = next_free->size;

                cur->size = total_size;
                size_t *footer_alloc = (size_t *)((char *)cur + total_size - FOOTER_SIZE);
                *footer_alloc = cur->size;

                if (prev) prev->next = next_free;
                else      free_list_head = next_free;
            } else {
                /* allocate the whole block */
                if (prev) prev->next = cur->next;
                else      free_list_head = cur->next;

                cur->size &= ~FREE_FLAG;
                size_t *footer = (size_t *)((char *)cur + cur->size - FOOTER_SIZE);
                *footer = cur->size;
            }

            return (void *)((char *)cur + HEADER_SIZE);
        }

        prev = cur;
        cur  = cur->next;
    }
    return NULL;
}

/* Free a previously allocated block */
void arena_free(void *p)
{
    if (!p) return;

    struct block *block = (struct block *)((char *)p - HEADER_SIZE);
    block->size |= FREE_FLAG;

    size_t *footer = (size_t *)((char *)block + (block->size & ~FREE_FLAG) - FOOTER_SIZE);
    *footer = block->size;

    /* Merge with previous free block if possible */
    if ((char *)block > arena_start) {
        size_t *prev_footer_ptr = (size_t *)((char *)block - FOOTER_SIZE);
        size_t prev_size = *prev_footer_ptr;
        struct block *prev_header =
            (struct block *)((char *)prev_footer_ptr - (prev_size & ~FREE_FLAG) + FOOTER_SIZE);

        if (prev_header->size & FREE_FLAG) {
            free_remove(prev_header);
            size_t new_size = (prev_header->size & ~FREE_FLAG) + (block->size & ~FREE_FLAG);
            prev_header->size = new_size | FREE_FLAG;

            size_t *new_footer = (size_t *)((char *)prev_header + new_size - FOOTER_SIZE);
            *new_footer = prev_header->size;

            block = prev_header;
        }
    }

    /* Merge with next free block if possible */
    struct block *next_header =
        (struct block *)((char *)block + (block->size & ~FREE_FLAG));

    if ((char *)next_header < arena_end && (next_header->size & FREE_FLAG)) {
        free_remove(next_header);
        size_t new_size = (block->size & ~FREE_FLAG) + (next_header->size & ~FREE_FLAG);
        block->size = new_size | FREE_FLAG;

        size_t *new_footer = (size_t *)((char *)block + new_size - FOOTER_SIZE);
        *new_footer = block->size;
    }

    /* Insert the (possibly merged) block into the free list */
    block->next = free_list_head;
    free_list_head = block;
}

/* Reallocate a block to a new size */
void *arena_realloc(void *p, size_t n)
{
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    struct block *block = (struct block *)((char *)p - HEADER_SIZE);
    size_t old_payload = (block->size & ~FREE_FLAG) - HEADER_SIZE - FOOTER_SIZE;
    size_t aligned_new_payload = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

    if (aligned_new_payload <= old_payload) {
        return p; /* shrink in place (no split for simplicity) */
    }

    /* Try to grow into the next free block */
    struct block *next_header =
        (struct block *)((char *)block + (block->size & ~FREE_FLAG));

    if ((char *)next_header < arena_end &&
        (next_header->size & FREE_FLAG) &&
        ((next_header->size & ~FREE_FLAG) >= (aligned_new_payload - old_payload))) {

        free_remove(next_header);
        size_t new_size = (block->size & ~FREE_FLAG) + (next_header->size & ~FREE_FLAG);
        block->size = new_size | FREE_FLAG;

        size_t *footer = (size_t *)((char *)block + new_size - FOOTER_SIZE);
        *footer = block->size;
        return p;
    }

    /* Allocate a new block and copy data */
    void *newp = arena_alloc(n);
    if (!newp) return NULL;

    size_t copy_size = old_payload < aligned_new_payload ? old_payload : aligned_new_payload;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}