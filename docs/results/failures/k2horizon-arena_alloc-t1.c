#include <stddef.h>

#define ARENA_MAGIC 0x4142454E  /* "AREN" */

typedef struct {
    struct Block *next;
    size_t size;   /* payload size in bytes */
    unsigned used; /* 0 = free, 1 = used */
} Block;

static Block *header_of(void *p) {
    return (Block *)((char *)p - sizeof(Block));
}

static size_t align_up(size_t n) {
    return (n + sizeof(Block) - 1) & ~(sizeof(Block) - 1);
}

static void coalesce(Block *b) {
    Block *next = b->next;
    if (b->next != NULL && !b->next->used) {
        b->size += next->size + sizeof(Block);
        b->next = next->next;
    }
}

static void *find_block(Block *start, size_t need) {
    for (Block *b = start; b != NULL; b = b->next) {
        if (!b->used && b->size >= need) {
            return b;
        }
    }
    return NULL;
}

static void *split(Block *b, size_t need) {
    size_t total = b->size + sizeof(Block);
    if (total >= need + sizeof(Block) + sizeof(Block)) {
        b->size = need;
        Block *nb = (Block *)((char *)b + sizeof(Block) + need);
        nb->size = total - sizeof(Block) - need;
        nb->used = 0;
        nb->next = b->next;
        b->next = nb;
    } else {
        b->size = total - sizeof(Block);
        b->next = NULL;
    }
    return (char *)b + sizeof(Block);
}

static Block *head;
static size_t capacity;

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(Block)) {
        head = NULL;
        capacity = 0;
        return;
    }
    Block *b = (Block *)buf;
    b->next = NULL;
    b->size = size - sizeof(Block);
    b->used = 0;
    head = b;
    capacity = size;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = align_up(n);
    Block *b = find_block(head, need);
    if (b == NULL) return NULL;
    b->used = 1;
    void *p = split(b, need);
    return p;
}

void arena_free(void *p) {
    if (p == NULL) return;
    Block *b = header_of(p);
    if (b->used) {
        b->used = 0;
        coalesce(b);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        if (p != NULL) arena_free(p);
        return NULL;
    }
    size_t need = align_up(n);
    if (p == NULL) return arena_alloc(need);

    Block *b = header_of(p);
    if (b->used == 0) return arena_alloc(need);

    /* Try to grow into free next block */
    if (b->next != NULL && !b->next->used &&
        b->size + b->next->size + sizeof(Block) >= need) {
        b->size += b->next->size + sizeof(Block);
        b->next = b->next->next;
        if (b->size < need) {
            /* not enough; fall through to copy path */
            b->used = 1;
        } else {
            void *np = split(b, need);
            if (np == p) return p;
            /* split moved start; copy data */
            if (need <= b->size) {
                __builtin_memcpy(np, p, need);
            }
            return np;
        }
    }

    /* Shrink in place if possible */
    if (b->size >= need) {
        b->size = need;
        return p;
    }

    /* Allocate new and copy */
    void *np = arena_alloc(need);
    if (np == NULL) return NULL;
    size_t copy = (b->size < need) ? b->size : need;
    __builtin_memcpy(np, p, copy);
    arena_free(p);
    return np;
}