#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

typedef struct Header {
    size_t size_and_flag;          /* payload size, LSB = 1 if allocated */
    struct Header *next;           /* valid only when free */
    struct Header *prev;           /* valid only when free */
} Header;

static unsigned char *arena_base = NULL;
static size_t arena_total = 0;
static Header *free_list = NULL;

#define ALLOC_FLAG ((size_t)1)

static inline size_t hdr_size(Header *h) { return h->size_and_flag & ~ALLOC_FLAG; }
static inline int   hdr_allocated(Header *h) { return (h->size_and_flag & ALLOC_FLAG) != 0; }
static inline void  hdr_set_allocated(Header *h, size_t sz) { h->size_and_flag = sz | ALLOC_FLAG; }
static inline void  hdr_set_free(Header *h, size_t sz) { h->size_and_flag = sz; }

/* find the block that ends exactly where *block starts, or NULL */
static Header *find_prev(Header *block)
{
    Header *cur = (Header *)arena_base;
    Header *prev = NULL;
    while ((unsigned char *)cur < (unsigned char *)block) {
        size_t total = sizeof(Header) + hdr_size(cur);
        prev = cur;
        cur = (Header *)((unsigned char *)cur + total);
    }
    return ((unsigned char *)cur == (unsigned char *)block) ? prev : NULL;
}

/* insert a free block into the free list (at front) */
static void free_insert(Header *h)
{
    h->next = free_list;
    h->prev = NULL;
    if (free_list) free_list->prev = h;
    free_list = h;
}

/* remove a free block from the free list */
static void free_remove(Header *h)
{
    if (h->prev) h->prev->next = h->next;
    else free_list = h->next;
    if (h->next) h->next->prev = h->prev;
}

/* ---------------------------------------------------------------------- */
void arena_init(void *buf, size_t size)
{
    size_t align = alignof(max_align_t);
    uintptr_t start = (uintptr_t)buf;
    uintptr_t aligned = (start + align - 1) & ~(uintptr_t)(align - 1);
    size_t padding = aligned - start;

    if (size < padding + sizeof(Header)) {
        arena_base = NULL;
        arena_total = 0;
        free_list = NULL;
        return;
    }

    arena_base = (unsigned char *)aligned;
    arena_total = size - padding;

    Header *init = (Header *)arena_base;
    hdr_set_free(init, arena_total - sizeof(Header));
    init->next = init->prev = NULL;
    free_list = init;
}

/* ---------------------------------------------------------------------- */
void *arena_alloc(size_t n)
{
    if (n == 0 || arena_base == NULL) return NULL;

    size_t align = alignof(max_align_t);
    size_t req = (n + align - 1) & ~(align - 1);

    Header *cur = free_list;
    while (cur) {
        if (hdr_size(cur) >= req) break;
        cur = cur->next;
    }
    if (!cur) return NULL;               /* out of memory */

    size_t remaining = hdr_size(cur) - req;
    if (remaining >= sizeof(Header) + align) {
        /* split */
        Header *newfree = (Header *)((unsigned char *)cur + sizeof(Header) + req);
        hdr_set_free(newfree, remaining - sizeof(Header));
        free_remove(cur);
        free_insert(newfree);
        hdr_set_allocated(cur, req);
    } else {
        /* allocate whole block */
        free_remove(cur);
        hdr_set_allocated(cur, hdr_size(cur));
    }
    return (void *)((unsigned char *)cur + sizeof(Header));
}

/* ---------------------------------------------------------------------- */
void arena_free(void *p)
{
    if (!p || arena_base == NULL) return;

    Header *hdr = (Header *)((unsigned char *)p - sizeof(Header));
    size_t sz = hdr_size(hdr);
    hdr_set_free(hdr, sz);
    hdr->next = hdr->prev = NULL;

    /* coalesce with previous block if free */
    Header *prev = find_prev(hdr);
    if (prev && !hdr_allocated(prev)) {
        free_remove(prev);
        size_t newsz = hdr_size(prev) + sizeof(Header) + sz;
        hdr_set_free(prev, newsz);
        hdr = prev;
    }

    /* coalesce with next block if free */
    Header *next = (Header *)((unsigned char *)hdr + sizeof(Header) + hdr_size(hdr));
    if ((unsigned char *)next < arena_base + arena_total && !hdr_allocated(next)) {
        free_remove(next);
        size_t newsz = hdr_size(hdr) + sizeof(Header) + hdr_size(next);
        hdr_set_free(hdr, newsz);
    }

    free_insert(hdr);
}

/* ---------------------------------------------------------------------- */
void *arena_realloc(void *p, size_t n)
{
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (arena_base == NULL) return NULL;

    Header *hdr = (Header *)((unsigned char *)p - sizeof(Header));
    size_t oldsz = hdr_size(hdr);
    size_t align = alignof(max_align_t);
    size_t req = (n + align - 1) & ~(align - 1);

    if (req <= oldsz) {
        /* shrink, possibly split */
        size_t excess = oldsz - req;
        if (excess >= sizeof(Header) + align) {
            Header *newfree = (Header *)((unsigned char *)hdr + sizeof(Header) + req);
            hdr_set_free(newfree, excess - sizeof(Header));
            free_insert(newfree);
            hdr_set_allocated(hdr, req);
        }
        return p;
    }

    /* try to expand into following free block */
    Header *next = (Header *)((unsigned char *)hdr + sizeof(Header) + oldsz);
    if ((unsigned char *)next < arena_base + arena_total && !hdr_allocated(next)) {
        size_t combined = oldsz + sizeof(Header) + hdr_size(next);
        if (combined >= req) {
            free_remove(next);
            size_t remaining = combined - req;
            if (remaining >= sizeof(Header) + align) {
                Header *newfree = (Header *)((unsigned char *)hdr + sizeof(Header) + req);
                hdr_set_free(newfree, remaining - sizeof(Header));
                free_insert(newfree);
                hdr_set_allocated(hdr, req);
            } else {
                hdr_set_allocated(hdr, combined);
            }
            return p;
        }
    }

    /* allocate new block */
    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    memcpy(newp, p, oldsz < n ? oldsz : n);
    arena_free(p);
    return newp;
}