#include <stdint.h>
#include <string.h>

typedef struct block {
    size_t n;
    struct block *next;
} block;

void arena_init(void *buf, size_t size) {
    block *b = buf;
    b->n = size;
    b->next = NULL;
    memset(buf, 0, size);
}

void *arena_alloc(size_t n) {
    block *b, *prev;
    void *p;
    size_t s;

    for (b = prev = (block*)buf; b; prev = b, b = b->next) {
        if (b->n >= n) {
            s = n;
            if (b->n > s && b->next && b->next->n >= s)
                s = b->n + b->next->n;
            p = (void*)((char*)b + s);
            if (s < b->n)
                b->n -= s;
            else {
                block *nb = b->next;
                b->next = nb->next;
                nb->next = NULL;
                if (nb->n >= s) {
                    nb->n -= s;
                    return p;
                }
            }
            return p;
        }
    }
    return NULL;
}

void arena_free(void *p) {
    block *b, *prev;

    for (b = prev = (block*)buf; b; prev = b, b = b->next) {
        if ((char*)p >= (char*)b && (char*)p < (char*)b + b->n) {
            if (prev->next != b)
                prev->next = b->next;
            else
                prev = b;
            b->n = (size_t)((char*)p - (char*)b);
            return;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    size_t s;
    block *b;

    for (b = (block*)buf; b; b = b->next) {
        if ((char*)p >= (char*)b && (char*)p < (char*)b + b->n) {
            s = n;
            if (b->n > s && b->next && b->next->n >= s)
                s = b->n + b->next->n;
            if (s <= b->n) {
                b->n -= s;
                return p;
            }
            return arena_alloc(n);
        }
    }
    return arena_alloc(n);
}