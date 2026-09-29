#include <stddef.h>

typedef struct header {
    size_t size;      /* size of user payload (not including header) */
    unsigned char used;
    unsigned char align_pad;
} header_t;

static header_t *hdr(void *p) { return (header_t *)p; }
static void *payload(header_t *h) { return (void *)((char *)h + sizeof(header_t)); }
static size_t hdr_size(void) { return sizeof(header_t); }

static void *buf;
static size_t buf_size;
static size_t used;   /* bytes of payload in use */

void arena_init(void *b, size_t s) {
    buf = b;
    buf_size = s;
    used = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = n + hdr_size();
    if (need > buf_size) return NULL;

    for (size_t off = 0; off + need <= buf_size; ) {
        header_t *h = (header_t *)((char *)buf + off);
        size_t cap = off + buf_size - off - need; /* not used; replaced below */
        /* compute free run length */
        size_t run = 0;
        for (size_t i = off; i + hdr_size() <= buf_size; ) {
            header_t *hh = (header_t *)((char *)buf + i);
            if (hh->used) break;
            run += hh->size + hdr_size();
            i += hh->size + hdr_size();
        }
        if (run >= need) {
            /* split if remainder fits a header */
            if (run - need >= hdr_size()) {
                size_t rem = run - need;
                h->size = n;
                h->used = 1;
                header_t *nh = (header_t *)((char *)h + n + hdr_size());
                nh->size = rem - hdr_size();
                nh->used = 0;
                used += n;
                return payload(h);
            } else {
                /* no split; use whole run */
                h->size = run - hdr_size();
                h->used = 1;
                used += h->size;
                return payload(h);
            }
        }
        off += (h->size + hdr_size());
    }
    return NULL;
}

static void coalesce(header_t *h) {
    /* merge with next */
    size_t total = h->size + hdr_size();
    header_t *n = (header_t *)((char *)h + total);
    if ((char *)n + hdr_size() <= (char *)buf + buf_size &&
        !n->used) {
        h->size = h->size + n->size + hdr_size();
    }
    /* merge with prev */
    header_t *p = (header_t *)((char *)h - (h->size + hdr_size()));
    size_t psize = h->size + hdr_size();
    if ((char *)p + sizeof(header_t) >= (char *)buf &&
        !p->used &&
        (char *)p + p->size + hdr_size() == (char *)h) {
        p->size = p->size + h->size + hdr_size();
        h = p;
    }
    (void)h;
}

void arena_free(void *p) {
    if (!p) return;
    header_t *h = hdr(p);
    h->used = 0;
    coalesce(h);
    used -= h->size;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) return NULL;
    if (!p) return arena_alloc(n);
    header_t *h = hdr(p);
    size_t need = n + hdr_size();
    size_t run = h->size + hdr_size();
    header_t *nxt = (header_t *)((char *)h + run);
    if ((char *)nxt + hdr_size() <= (char *)buf + buf_size &&
        !nxt->used &&
        h->size + nxt->size + hdr_size() >= need) {
        /* grow in place */
        size_t rem = h->size + nxt->size + hdr_size() - need;
        if (rem >= hdr_size()) {
            nxt->size = rem - hdr_size();
            h->size = n;
        } else {
            /* absorb whole neighbour */
            h->size = n;
        }
        used = used - h->size + n;
        return p;
    }
    /* try to shrink in place */
    if (h->size > n) {
        size_t rem = h->size - n;
        if (rem >= hdr_size()) {
            header_t *nh = (header_t *)((char *)h + n + hdr_size());
            nh->size = rem - hdr_size();
            nh->used = 0;
            used -= rem;
            return p;
        }
    }
    /* move */
    void *np = arena_alloc(n);
    if (!np) return NULL;
    size_t cp = h->size < n ? h->size : n;
    /* copy payload */
    __builtin_memcpy(np, p, cp);
    arena_free(p);
    return np;
}