#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

/* Fixed-buffer allocator with coalescing free-list.
 * All state lives inside the provided buffer.
 */

typedef struct block_header {
    size_t size; /* including header */
    int is_free;
    struct block_header *next; /* next physical block */
    struct block_header *prev; /* previous physical block */
} block_header_t;

#define HEADER_SIZE ((sizeof(block_header_t) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define ALIGN_SIZE(s) (((s) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

typedef struct {
    void *buf_start;
    void *buf_end;
    block_header_t *head; /* first block */
    size_t total_size;
} arena_state_t;

#define STATE_OFFSET (alignof(max_align_t) - 1)
#define STATE_SIZE ((sizeof(arena_state_t) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

static inline arena_state_t *get_state(void *buf)
{
    return (arena_state_t *)buf;
}

static inline void *state_to_buf(arena_state_t *state)
{
    return (void *)state;
}

/* Get pointer to the data area of a block */
static inline void *block_data(block_header_t *blk)
{
    return (void *)((char *)blk + HEADER_SIZE);
}

/* Get block header from data pointer */
static inline block_header_t *data_to_block(void *p)
{
    return (block_header_t *)((char *)p - HEADER_SIZE);
}

/* Check if pointer is within buffer */
static inline int in_buffer(arena_state_t *state, void *p)
{
    return (unsigned char *)p >= (unsigned char *)state->buf_start &&
           (unsigned char *)p < (unsigned char *)state->buf_end;
}

static void arena_init(void *buf, size_t size)
{
    if (!buf || size == 0) return;

    /* Align the buffer start */
    void *aligned_buf = (void *)(((uintptr_t)buf + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1));
    size_t aligned_size = size - ((unsigned char *)aligned_buf - (unsigned char *)buf);

    if (aligned_size < STATE_SIZE + HEADER_SIZE) return;

    arena_state_t *state = get_state(aligned_buf);
    state->buf_start = aligned_buf;
    state->buf_end = (void *)((unsigned char *)aligned_buf + aligned_size);
    state->total_size = aligned_size;

    /* First block starts after state */
    void *first_block_addr = (void *)((unsigned char *)aligned_buf + STATE_SIZE);
    block_header_t *first = (block_header_t *)first_block_addr;

    first->size = aligned_size - STATE_SIZE;
    first->is_free = 1;
    first->next = NULL;
    first->prev = NULL;

    state->head = first;
}

static void *arena_alloc(size_t n)
{
    if (n == 0) return NULL;

    /* Need to find the state. But we don't have a global pointer.
     * This is a problem: arena_alloc has no argument to find state.
     * 
     * Looking at the API again: arena_init takes buf and size.
     * arena_alloc takes only n.
     * 
     * This means we need a global or thread-local variable to track the active arena.
     * Since the problem doesn't specify, and we can't use malloc, we'll use a static variable.
     * This is a common pattern for simple allocators.
     */
    static arena_state_t *g_state = NULL;

    if (!g_state) return NULL;

    /* Align the requested size */
    size_t aligned_n = ALIGN_SIZE(n);
    size_t total_needed = HEADER_SIZE + aligned_n;

    /* First-fit search */
    block_header_t *blk = g_state->head;
    while (blk) {
        if (blk->is_free && blk->size >= total_needed) {
            /* Check if we should split */
            size_t remainder = blk->size - total_needed;
            if (remainder >= HEADER_SIZE) {
                /* Split the block */
                block_header_t *new_blk = (block_header_t *)((char *)blk + total_needed);
                new_blk->size = remainder;
                new_blk->is_free = 1;
                new_blk->next = blk->next;
                new_blk->prev = blk;
                if (blk->next) {
                    blk->next->prev = new_blk;
                }
                blk->next = new_blk;
                blk->size = total_needed;
            }
            blk->is_free = 0;
            return block_data(blk);
        }
        blk = blk->next;
    }

    return NULL;
}

static void arena_free(void *p)
{
    if (!p) return;

    static arena_state_t *g_state = NULL;
    if (!g_state) return;

    /* Check if p is valid */
    if (!in_buffer(g_state, p)) return;

    block_header_t *blk = data_to_block(p);

    /* Validate the block */
    if (!in_buffer(g_state, blk)) return;
    if (!blk->is_free) {
        blk->is_free = 1;

        /* Coalesce with next block */
        if (blk->next && blk->next->is_free) {
            blk->size += blk->next->size;
            blk->next = blk->next->next;
            if (blk->next) {
                blk->next->prev = blk;
            }
        }

        /* Coalesce with previous block */
        if (blk->prev && blk->prev->is_free) {
            blk->prev->size += blk->size;
            blk->prev->next = blk->next;
            if (blk->next) {
                blk->next->prev = blk->prev;
            }
        }
    }
}

static void *arena_realloc(void *p, size_t n)
{
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    if (!p) {
        return arena_alloc(n);
    }

    static arena_state_t *g_state = NULL;
    if (!g_state) return NULL;

    if (!in_buffer(g_state, p)) return NULL;

    block_header_t *blk = data_to_block(p);
    if (!in_buffer(g_state, blk)) return NULL;

    size_t aligned_n = ALIGN_SIZE(n);
    size_t total_needed = HEADER_SIZE + aligned_n;
    size_t old_size = blk->size;

    /* Try to grow in place */
    if (blk->size >= total_needed) {
        /* Same block, possibly shrink */
        size_t remainder = blk->size - total_needed;
        if (remainder >= HEADER_SIZE) {
            /* Split */
            block_header_t *new_blk = (block_header_t *)((char *)blk + total_needed);
            new_blk->size = remainder;
            new_blk->is_free = 1;
            new_blk->next = blk->next;
            new_blk->prev = blk;
            if (blk->next) {
                blk->next->prev = new_blk;
            }
            blk->next = new_blk;
            blk->size = total_needed;
        }
        return p;
    }

    /* Try to grow into next free block */
    if (blk->next && blk->next->is_free) {
        size_t combined_size = blk->size + blk->next->size;
        if (combined_size >= total_needed) {
            /* Grow into next block */
            blk->size = combined_size;
            blk->next = blk->next->next;
            if (blk->next) {
                blk->next->prev = blk;
            }

            /* Possibly split */
            size_t remainder = blk->size - total_needed;
            if (remainder >= HEADER_SIZE) {
                block_header_t *new_blk = (block_header_t *)((char *)blk + total_needed);
                new_blk->size = remainder;
                new_blk->is_free = 1;
                new_blk->next = blk->next;
                new_blk->prev = blk;
                if (blk->next) {
                    blk->next->prev = new_blk;
                }
                blk->next = new_blk;
                blk->size = total_needed;
            }
            return p;
        }
    }

    /* Need to allocate a new block */
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;

    /* Copy min(old_size, new_size) bytes */
    size_t copy_size = old_size - HEADER_SIZE;
    size_t new_copy_size = n;
    if (copy_size > new_copy_size) {
        copy_size = new_copy_size;
    }
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}

/* Helper to set global state - must be called after init */
void arena_set_state(void *buf, size_t size)
{
    void *aligned_buf = (void *)(((uintptr_t)buf + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1));
    size_t aligned_size = size - ((unsigned char *)aligned_buf - (unsigned char *)buf);
    if (aligned_size < STATE_SIZE + HEADER_SIZE) return;

    static arena_state_t *g_state = NULL;
    g_state = get_state(aligned_buf);
}