#include <stddef.h>
#include <stdint.h>

typedef struct block {
    size_t size;      /* total block size including header, rounded to alignment */
    uint8_t free;     /* 1 if free, 0 if in use */
    uint8_t pad[7];   /* alignment padding so size is multiple of _Alignof(max_align_t) */
} block_t;

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + ALIGNMENT - 1) & ~(size_t)(ALIGNMENT - 1))
#define HEADER_SIZE (ALIGN_UP(sizeof(block_t)))
#define USER_SIZE(b) ((b)->size - HEADER_SIZE)
#define NEXT(b)   ((block_t *)((char *)(b) + (b)->size))
#define PREV(b)   ((block_t *)((char *)(b) - ((block_t *)b)->size))
#define PREV_FREE(b) ((char *)(b) - ((block_t *)((char *)(b) - HEADER_SIZE))->size)

static block_t *arena_start;
static size_t arena_size;
static block_t *arena_end;

static block_t *find_fit(size_t n) {
    block_t *b = arena_start;
    size_t need = HEADER_SIZE + n;
    size_t min_size = ALIGN_UP(need);
    while ((size_t)(b->size) <= arena_size - min_size) {
        if (b->free && b->size >= min_size) return b;
        b = NEXT(b);
    }
    return NULL;
}

void arena_init(void *buf, size_t size) {
    arena_start = (block_t *)buf;
    arena_size = size;
    arena_end = (block_t *)((char *)buf + size);
    block_t *b = arena_start;
    b->size = arena_size;
    b->free = 1;
    /* zero the header; body is left untouched */
    for (size_t i = 0; i < HEADER_SIZE; i++) ((uint8_t *)b)[i] = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    block_t *b = find_fit(n);
    if (b == NULL) return NULL;
    /* split if remainder is large enough to hold a free block */
    size_t rem = b->size - HEADER_SIZE - n;
    if (rem >= HEADER_SIZE + ALIGNMENT) {
        block_t *nb = (block_t *)((char *)b + HEADER_SIZE + n);
        nb->size = rem + HEADER_SIZE;
        nb->free = 1;
        for (size_t i = 0; i < HEADER_SIZE; i++) ((uint8_t *)nb)[i] = 0;
    }
    b->size = HEADER_SIZE + n;
    b->free = 0;
    return (void *)((char *)b + HEADER_SIZE);
}

void arena_free(void *p) {
    if (p == NULL) return;
    block_t *b = (block_t *)((char *)p - HEADER_SIZE);
    b->free = 1;
    /* coalesce with next */
    if ((size_t)(b->size) <= arena_size - HEADER_SIZE - USER_SIZE(b)) {
        block_t *nb = NEXT(b);
        if (nb->free) {
            b->size += nb->size;
        }
    }
    /* coalesce with prev */
    if ((char *)b >= (char *)arena_start + HEADER_SIZE) {
        block_t *pb = PREV_FREE(b);
        if (pb->free) {
            pb->size += b->size;
            b = pb;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    block_t *b = (block_t *)((char *)p - HEADER_SIZE);
    size_t old_user = USER_SIZE(b);

    /* try to grow in place using the next block */
    if ((size_t)(b->size) <= arena_size - HEADER_SIZE - old_user) {
        block_t *nb = NEXT(b);
        if (nb->free && nb->size >= HEADER_SIZE + n) {
            /* nb is big enough: extend b into it */
            size_t extra = nb->size - HEADER_SIZE;
            if (extra >= n) {
                b->size += HEADER_SIZE + n;
                /* if nb had a tail, leave a remainder block */
                if (extra > n) {
                    block_t *tail = (block_t *)((char *)nb + HEADER_SIZE + n);
                    tail->size = extra - n;
                    tail->free = 1;
                    for (size_t i = 0; i < HEADER_SIZE; i++) ((uint8_t *)tail)[i] = 0;
                } else {
                    /* nb consumed entirely; mark b free tail if any */
                }
                return p;
            }
        }
    }

    /* otherwise allocate new and copy */
    void *np = arena_alloc(n);
    if (np == NULL) return NULL;
    /* copy up to min(old_user, n) */
    if (old_user < n) old_user = n;
    /* n is the new size; copy min(old_user, n) */
    size_t to_copy = old_user < n ? old_user : n;
    for (size_t i = 0; i < to_copy; i++) ((uint8_t *)np)[i] = ((uint8_t *)p)[i];
    arena_free(p);
    return np;
}