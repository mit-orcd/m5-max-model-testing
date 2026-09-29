#include <stddef.h>

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

static char *g_buf;
static size_t g_size;
static Block *g_head;

static size_t align_up(size_t n, size_t a) {
    return (n + a - 1) & ~(a - 1);
}

void arena_init(void *buf, size_t size) {
    g_buf = (char *)buf;
    g_size = size;
    g_head = (Block *)g_buf;
    g_head->size = (size >= sizeof(Block)) ? (size - sizeof(Block)) : 0;
    g_head->next = 0;
}

void *arena_alloc(size_t n) {
    size_t a;
    Block *b;
    if (n == 0) {
        return 0;
    }
    a = align_up(n, sizeof(Block));
    b = g_head;
    while (b) {
        if (b->size >= a) {
            if (b->size >= a + sizeof(Block)) {
                Block *nb = (Block *)((char *)b + a);
                nb->size = b->size - a;
                nb->next = b->next;
                b->size = a;
                b->next = nb;
            }
            return (void *)(b + 1);
        }
        b = b->next;
    }
    return 0;
}

void arena_free(void *p) {
    Block *b, *prev;
    Block *fb;
    if (!p) {
        return;
    }
    fb = (Block *)p - 1;
    b = g_head;
    prev = 0;
    while (b && b < fb) {
        prev = b;
        b = b->next;
    }
    if (b == fb) {
        if (prev) {
            prev->next = fb->next;
        } else {
            g_head = fb->next;
        }
        fb->next = 0;
        if (prev && (char *)prev + prev->size + sizeof(Block) == (char *)fb) {
            prev->size += sizeof(Block) + fb->size;
            prev->next = fb->next;
        }
        if (fb->next && (char *)fb + sizeof(Block) + fb->size == (char *)fb->next) {
            fb->size += sizeof(Block) + fb->next->size;
            fb->next = fb->next->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    Block *b;
    size_t a;
    void *np;
    size_t copy;
    char *dst;
    const char *src;
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return 0;
    }
    b = (Block *)p - 1;
    a = align_up(n, sizeof(Block));
    if (b->size >= a) {
        return p;
    }
    if (b->next && (char *)b + sizeof(Block) + b->size == (char *)b->next) {
        if (b->size + b->next->size >= a) {
            if (b->size + b->next->size >= a + sizeof(Block)) {
                Block *nb = (Block *)((char *)b + a);
                nb->size = b->size + b->next->size - a;
                nb->next = b->next->next;
                b->size = a;
                b->next = nb;
            } else {
                b->size += b->next->size;
                b->next = b->next->next;
            }
            if (b->next && (char *)b + sizeof(Block) + b->size == (char *)b->next) {
                b->size += sizeof(Block) + b->next->size;
                b->next = b->next->next;
            }
            return p;
        }
    }
    np = arena_alloc(n);
    if (!np) {
        return 0;
    }
    copy = (b->size < n) ? b->size : n;
    dst = (char *)np;
    src = (const char *)p;
    while (copy--) {
        *dst++ = *src++;
    }
    arena_free(p);
    return np;
}