#include <stddef.h>

typedef struct block {
    struct block *next;
    size_t size;      /* payload size */
    unsigned long flags; /* bit 0: free */
} block;

static block *head;
static size_t buf_size;
static block *free_list;

void arena_init(void *buf, size_t size) {
    if (buf && size >= sizeof(block)) {
        head = (block *)buf;
        buf_size = size - sizeof(block);
        head->next = 0;
        head->size = buf_size;
        head->flags = 1;
        free_list = head;
    } else {
        head = 0;
        buf_size = 0;
        free_list = 0;
    }
}

static block *find_fit(size_t n) {
    block *b = free_list;
    while (b) {
        if (b->size >= n) return b;
        b = b->next;
    }
    return 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return 0;
    if (!head) return 0;
    if (n > buf_size) return 0;

    block *b = find_fit(n);
    if (!b) return 0;

    /* remove from free list */
    if (b == free_list) free_list = b->next;
    else {
        block *p = free_list;
        while (p && p->next != b) p = p->next;
        if (p) p->next = b->next;
    }

    if (b->size > n + sizeof(block)) {
        /* split off trailing free block */
        block *t = (block *)((char *)b + sizeof(block) + n);
        t->next = b->next;
        t->size = b->size - sizeof(block) - n;
        t->flags = 1;
        b->next = t;
        b->size = n;
        b->flags = 0;
        free_list = t;
    } else {
        b->next = 0;
        b->flags = 0;
    }

    return (void *)((char *)b + sizeof(block));
}

void arena_free(void *p) {
    if (!p) return;
    block *b = (block *)((char *)p - sizeof(block));

    /* coalesce with next */
    if (b->next && (b->next->flags & 1)) {
        block *nx = b->next;
        b->size += nx->size + sizeof(block);
        b->next = nx->next;
    }

    /* coalesce with previous */
    block *prev = 0;
    block *p2 = head;
    while (p2 && p2->next != b) {
        prev = p2;
        p2 = p2->next;
    }
    if (prev && (prev->flags & 1)) {
        prev->size += b->size + sizeof(block);
        prev->next = b->next;
        b = prev;
    }

    b->flags = 1;

    /* insert into free list at head */
    b->next = free_list;
    free_list = b;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return 0;
    }

    block *b = (block *)((char *)p - sizeof(block));

    /* try to grow into next free block */
    if (b->next && (b->next->flags & 1)) {
        size_t avail = b->size + b->next->size + sizeof(block);
        if (avail >= n) {
            /* merge */
            b->size += b->next->size + sizeof(block);
            b->next = b->next->next;
            if (b->next == free_list) free_list = b->next;
            /* remove b->next from free list if present */
            block *pp = free_list;
            while (pp && pp->next != b->next) pp = pp->next;
            if (pp) pp->next = b->next;
            return p;
        }
    }

    void *np = arena_alloc(n);
    if (!np) return 0;
    {
        size_t copy = n < b->size ? n : b->size;
        __builtin_memcpy(np, p, copy);
    }
    arena_free(p);
    return np;
}