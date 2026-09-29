#include <stddef.h>
#include <stdint.h>

typedef struct blk {
    size_t size;        /* payload bytes, low bit = used flag */
    struct blk *next;
} blk;

static blk *head;

#define ALIGN_UP(x) (((x) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1))

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(blk)) { head = NULL; return; }
    head = (blk *)buf;
    head->size = size - sizeof(blk);
    head->next = NULL;
}

static blk *next_phys(blk *b, size_t n) {
    (void)n;
    return b->next;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    n = ALIGN_UP(n);
    for (blk *b = head; b; b = b->next) {
        if ((b->size & 1) || (b->size & ~(size_t)1) < n) continue;
        size_t avail = b->size & ~(size_t)1;
        if (avail >= n + sizeof(blk) + _Alignof(max_align_t)) {
            blk *nb = (blk *)((char *)(b + 1) + n);
            nb->size = (avail - n - sizeof(blk));
            nb->next = b->next;
            b->next = nb;
            b->size = n | 1;
        } else {
            b->size = avail | 1;
        }
        return (void *)(b + 1);
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    blk *b = (blk *)p - 1;
    b->size &= ~(size_t)1;
    /* coalesce: single pass merging adjacent free blocks via the list */
    for (blk *c = head; c && c->next; ) {
        blk *nx = c->next;
        int cfree = !(c->size & 1), nfree = !(nx->size & 1);
        if (cfree && nfree && (blk *)((char *)(c + 1) + c->size) == nx) {
            c->size += sizeof(blk) + nx->size;
            c->next = nx->next;
        } else {
            c = nx;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    blk *b = (blk *)p - 1;
    size_t old = b->size & ~(size_t)1;
    n = ALIGN_UP(n);
    if (n <= old) return p;
    /* grow into the next block if it is free and adjacent and large enough */
    blk *nx = b->next;
    if (nx && !(nx->size & 1) && (blk *)((char *)(b + 1) + old) == nx) {
        size_t nsize = nx->size & ~(size_t)1;
        if (old + sizeof(blk) + nsize >= n) {
            size_t total = old + sizeof(blk) + nsize;
            if (total >= n + sizeof(blk) + _Alignof(max_align_t)) {
                blk *nb = (blk *)((char *)(b + 1) + n);
                nb->size = total - n - sizeof(blk);
                nb->next = nx->next;
                b->next = nb;
                b->size = n | 1;
            } else {
                b->size = total | 1;
                b->next = nx->next;
            }
            return p;
        }
    }
    void *np = arena_alloc(n);
    if (!np) return NULL;
    char *d = np, *s = p;
    for (size_t i = 0; i < old; i++) d[i] = s[i];
    arena_free(p);
    return np;
}
