#include <stddef.h>
#include <stdalign.h>
#include <string.h>

typedef struct Block {
    size_t size;
    int free;
} Block;

static void *g_buf;
static size_t g_size;
static size_t g_total;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    if (size >= sizeof(Block)) {
        Block *b = (Block *)buf;
        b->size = size - sizeof(Block);
        b->free = 1;
        g_total = size - sizeof(Block);
    } else {
        g_total = 0;
    }
}

static Block *first_block(void) {
    return (Block *)g_buf;
}

static void *block_data(Block *b) {
    return (void *)((char *)b + sizeof(Block));
}

static Block *data_block(void *p) {
    return (Block *)((char *)p - sizeof(Block));
}

static int block_is_after(Block *b, void *p) {
    return (char *)b == (char *)g_buf + ((char *)p - (char *)block_data(b)) + b->size;
}

static Block *next_block(Block *b) {
    return (Block *)((char *)b + sizeof(Block) + b->size);
}

static int block_is_free(Block *b) {
    return b->free;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = _Alignof(max_align_t);
    size_t header_size = sizeof(Block);
    size_t effective_align = align > header_size ? align : header_size;
    size_t total_needed = n + effective_align;
    if (total_needed < n) return NULL;

    Block *best = NULL;
    Block *b = first_block();
    while ((char *)b < (char *)g_buf + g_size) {
        if (b->free && b->size >= total_needed) {
            if (!best || b->size < best->size) {
                best = b;
                if (b->size == total_needed) break;
            }
        }
        b = next_block(b);
    }
    if (!best) return NULL;

    if (best->size >= total_needed + sizeof(Block) + effective_align) {
        Block *new_block = (Block *)((char *)best + sizeof(Block) + total_needed);
        new_block->size = best->size - total_needed - sizeof(Block);
        new_block->free = 1;
        best->size = total_needed;
    } else {
        best->size = total_needed;
    }
    best->free = 0;
    g_total -= best->size;
    return block_data(best);
}

void arena_free(void *p) {
    if (!p) return;
    Block *b = data_block(p);
    if ((char *)b < (char *)g_buf || (char *)b >= (char *)g_buf + g_size) return;
    b->free = 1;
    g_total += b->size;

    Block *next = next_block(b);
    if ((char *)next < (char *)g_buf + g_size && next->free) {
        b->size += sizeof(Block) + next->size;
    }

    if (b != first_block()) {
        Block *prev = (Block *)((char *)b - sizeof(Block) - ((char *)b - (char *)block_data(prev)));
        /* find prev by scanning back */
        Block *scan = first_block();
        while (1) {
            Block *sn = next_block(scan);
            if (sn == b) break;
            scan = sn;
        }
        if (scan->free) {
            scan->size += sizeof(Block) + b->size;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    Block *b = data_block(p);
    size_t old_size = b->size;
    size_t align = _Alignof(max_align_t);
    size_t header_size = sizeof(Block);
    size_t effective_align = align > header_size ? align : header_size;
    size_t total_needed = n + effective_align;
    if (total_needed < n) return NULL;

    if (old_size >= total_needed) {
        if (old_size >= total_needed + sizeof(Block) + effective_align) {
            Block *new_block = (Block *)((char *)b + sizeof(Block) + total_needed);
            new_block->size = old_size - total_needed - sizeof(Block);
            new_block->free = 1;
            b->size = total_needed;
        } else {
            b->size = total_needed;
        }
        size_t copy = old_size < total_needed ? old_size : total_needed;
        if (copy > sizeof(Block)) {
            size_t data_copy = copy - sizeof(Block);
            memmove(block_data(b), p, data_copy);
        }
        return p;
    }

    Block *next = next_block(b);
    if ((char *)next < (char *)g_buf + g_size && next->free &&
        old_size + sizeof(Block) + next->size >= total_needed) {
        size_t extra = old_size + sizeof(Block) + next->size - total_needed;
        if (extra >= sizeof(Block) + effective_align) {
            Block *new_next = (Block *)((char *)next + total_needed - sizeof(Block));
            new_next->size = next->size + old_size + sizeof(Block) - total_needed - sizeof(Block);
            new_next->free = 1;
            b->size = total_needed;
        } else {
            b->size = old_size + sizeof(Block) + next->size;
        }
        size_t copy = old_size < total_needed ? old_size : total_needed;
        if (copy > sizeof(Block)) {
            size_t data_copy = copy - sizeof(Block);
            memmove(block_data(b), p, data_copy);
        }
        return p;
    }

    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy = old_size < total_needed ? old_size : total_needed;
        if (copy > sizeof(Block)) {
            size_t data_copy = copy - sizeof(Block);
            memcpy(new_p, p, data_copy);
        }
        arena_free(p);
    }
    return new_p;
}