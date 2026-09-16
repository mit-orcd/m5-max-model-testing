#include <stddef.h>
#include <stdalign.h>

typedef struct block {
    size_t size;      /* total block size including header */
    unsigned int free : 1;
    unsigned int align_pad : 31;
} block_t;

static block_t *buf_start;
static size_t buf_size;

void arena_init(void *buf, size_t size) {
    buf_start = (block_t *)buf;
    buf_size = size;
}

static size_t align_up(size_t n) {
    return (n + sizeof(block_t) - 1) & ~(sizeof(block_t) - 1);
}

static block_t *next_block(block_t *b) {
    return (block_t *)((char *)b + b->size);
}

static block_t *prev_block(block_t *b) {
    return (block_t *)((char *)b - b->size);
}

static int block_fits(block_t *b, size_t n) {
    return b->free && b->size >= align_up(n);
}

static void coalesce(block_t *b) {
    block_t *n = next_block(b);
    if ((char *)n < (char *)buf_start + buf_size && n->free) {
        b->size += n->size;
    }
    if ((char *)b > (char *)buf_start) {
        block_t *p = prev_block(b);
        if (p->free) {
            p->size += b->size;
            b = p;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = align_up(n);
    block_t *best = NULL;
    size_t best_size = (size_t)-1;

    for (block_t *b = buf_start; (char *)b < (char *)buf_start + buf_size; b = next_block(b)) {
        if (block_fits(b, need) && b->size < best_size) {
            best = b;
            best_size = b->size;
        }
    }

    if (best == NULL) return NULL;

    if (best->size >= need + sizeof(block_t)) {
        best->free = 0;
        best->size -= sizeof(block_t);
        return (void *)((char *)best + sizeof(block_t));
    }

    best->free = 0;
    return (void *)((char *)best + sizeof(block_t));
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_t *b = (block_t *)((char *)p - sizeof(block_t));
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) return NULL;
    if (p == NULL) return arena_alloc(n);

    block_t *b = (block_t *)((char *)p - sizeof(block_t));
    size_t need = align_up(n);

    /* Try to grow into next free neighbour */
    block_t *nxt = next_block(b);
    if ((char *)nxt < (char *)buf_start + buf_size && nxt->free &&
        b->size + nxt->size >= need) {
        b->size += nxt->size;
        return p;
    }

    /* Try to shrink in place */
    if (b->size >= need + sizeof(block_t)) {
        b->size -= sizeof(block_t);
        return p;
    }

    /* Allocate new and copy */
    void *np = arena_alloc(n);
    if (np == NULL) return NULL;
    size_t old_size = b->size - sizeof(block_t);
    size_t copy = old_size < n ? old_size : n;
    __builtin_memcpy(np, p, copy);
    arena_free(p);
    return np;
}