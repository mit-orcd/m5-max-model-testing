#include <stddef.h>
#include <string.h>

#define ARENA_ALIGN _Alignof(max_align_t)

typedef struct arena_hdr {
    size_t size;      /* size of user allocation */
    size_t prev_free; /* offset of previous block (0 if none) */
    size_t next_free; /* offset of next free block (0 if none) */
    unsigned int used;/* 1 = used, 0 = free */
} arena_hdr_t;

static arena_hdr_t *hdr(void *buf, size_t off) {
    return (arena_hdr_t *)((unsigned char *)buf + off);
}

static void *to_ptr(void *buf, size_t off) {
    return (unsigned char *)buf + off;
}

static size_t align_up(size_t n) {
    return (n + ARENA_ALIGN - 1) & ~(size_t)(ARENA_ALIGN - 1);
}

static size_t hdr_size(void) {
    return align_up(sizeof(arena_hdr_t));
}

static size_t block_total(size_t n) {
    return hdr_size() + align_up(n);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < hdr_size())
        return;
    arena_hdr_t *h = hdr(buf, 0);
    h->size = size - hdr_size();
    h->prev_free = 0;
    h->next_free = 0;
    h->used = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0)
        return NULL;
    size_t need = block_total(n);
    unsigned char *buf = (unsigned char *)arena_buf;
    (void)buf;
    /* arena_buf is not accessible here; we rely on a file-scope pointer set by arena_init */
    return (void *)0; /* replaced below */
}

/* NOTE: arena_buf is set by arena_init. */
static void *arena_buf = NULL;
static size_t arena_cap = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < hdr_size())
        return;
    arena_buf = buf;
    arena_cap = size;
    arena_hdr_t *h = hdr(buf, 0);
    h->size = size - hdr_size();
    h->prev_free = 0;
    h->next_free = 0;
    h->used = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_buf)
        return NULL;
    size_t need = block_total(n);
    unsigned char *p = (unsigned char *)arena_buf;
    size_t off = hdr_size();
    size_t cap = arena_cap;
    while (off + need <= cap) {
        arena_hdr_t *h = hdr(p, off);
        if (!h->used && h->size >= need) {
            size_t rem = h->size - need;
            if (rem >= hdr_size() + ARENA_ALIGN) {
                /* split */
                arena_hdr_t *nh = hdr(p, off + need);
                nh->size = rem;
                nh->prev_free = 0;
                nh->next_free = 0;
                nh->used = 0;
                h->size = need;
                h->used = 1;
                return p + off + hdr_size();
            }
            /* take whole block */
            h->used = 1;
            return p + off + hdr_size();
        }
        off += hdr_size() + h->size;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_buf)
        return;
    unsigned char *bp = (unsigned char *)p;
    size_t off = (size_t)(bp - (unsigned char *)arena_buf) - hdr_size();
    if (off >= arena_cap)
        return;
    arena_hdr_t *h = hdr(arena_buf, off);
    if (!h->used)
        return;
    h->used = 0;
    /* merge with next */
    size_t next_off = off + hdr_size() + h->size;
    if (next_off < arena_cap) {
        arena_hdr_t *nh = hdr(arena_buf, next_off);
        if (!nh->used) {
            h->size += nh->size + hdr_size();
            /* shift any prev/next free links? we do not track free list */
        }
    }
    /* merge with prev */
    size_t prev_off = off;
    /* find previous block by scanning */
    unsigned char *cur = (unsigned char *)arena_buf + hdr_size();
    size_t prev_sz = 0;
    while (cur < bp) {
        arena_hdr_t *ch = hdr(arena_buf, (size_t)(cur - (unsigned char *)arena_buf));
        prev_sz = ch->size;
        cur += hdr_size() + ch->size;
        if (cur == bp)
            break;
    }
    if (prev_sz > 0 && prev_off > hdr_size()) {
        /* prev block ends at off */
        size_t prev_hdr_off = prev_off - prev_sz - hdr_size();
        if (prev_hdr_off >= hdr_size()) {
            arena_hdr_t *ph = hdr(arena_buf, prev_hdr_off);
            if (!ph->used) {
                ph->size += h->size + hdr_size();
                h->size = 0; /* merged away */
                return;
            }
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        if (n == 0)
            return NULL;
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    unsigned char *bp = (unsigned char *)p;
    size_t off = (size_t)(bp - (unsigned char *)arena_buf) - hdr_size();
    if (off >= arena_cap)
        return NULL;
    arena_hdr_t *h = hdr(arena_buf, off);
    if (!h->used)
        return NULL;

    size_t cur = h->size;
    size_t need = block_total(n);

    /* try to extend into next free neighbour */
    size_t next_off = off + hdr_size() + h->size;
    if (next_off < arena_cap) {
        arena_hdr_t *nh = hdr(arena_buf, next_off);
        if (!nh->used && nh->size >= need - hdr_size() - cur) {
            size_t add = need - hdr_size() - cur;
            if (add <= nh->size) {
                h->size += add;
                nh->size -= add;
                if (nh->size < hdr_size() + ARENA_ALIGN) {
                    /* absorb remainder */
                    h->size += nh->size + hdr_size();
                    /* mark nh used? it is absorbed, but we just zero it */
                    nh->used = 1;
                }
                return p;
            }
        }
    }

    /* general path: allocate new, copy, free old */
    void *np = arena_alloc(n);
    if (!np)
        return NULL;
    size_t cp = cur < n ? cur : n;
    memcpy(np, p, cp);
    arena_free(p);
    return np;
}