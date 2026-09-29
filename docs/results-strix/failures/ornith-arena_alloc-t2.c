#include <stddef.h>

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

static char *g_heap = NULL;
static size_t g_heap_size = 0;
static Block *g_head = NULL;

void arena_init(void *buf, size_t size) {
    g_heap = (char *)buf;
    g_heap_size = size;
    g_head = (Block *)g_heap;
    g_head->size = (size >= sizeof(Block)) ? (size - sizeof(Block)) : 0;
    g_head->next = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }
    size_t need = n + sizeof(Block);
    if (need < n) {
        return NULL;
    }
    Block *cur = g_head;
    while (cur != NULL) {
        if (cur->size >= need) {
            size_t remaining = cur->size - need;
            if (remaining > sizeof(Block)) {
                Block *split = (Block *)((char *)cur + need);
                split->size = remaining - sizeof(Block);
                split->next = cur->next;
                cur->size = need;
                cur->next = split;
            } else {
                cur->size = need;
            }
            void *p = (void *)((char *)cur + sizeof(Block));
            g_head = cur;
            return p;
        }
        cur = cur->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }
    Block *b = (Block *)((char *)p - sizeof(Block));
    if ((char *)b < g_heap || (char *)b + g_heap_size <= (char *)b ||
        (size_t)((char *)b + g_heap_size - (char *)b) < sizeof(Block)) {
        return;
    }
    b->size += sizeof(Block);
    if (b->next != NULL) {
        b->size += b->next->size;
        b->next = b->next->next;
    }
    Block *cur = g_head;
    Block *prev = NULL;
    while (cur != NULL && cur < b) {
        prev = cur;
        cur = cur->next;
    }
    if (prev != NULL) {
        prev->size += b->size;
        prev->next = b->next;
    } else {
        *b = *g_head;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    Block *b = (Block *)((char *)p - sizeof(Block));
    size_t old_size = b->size - sizeof(Block);
    if (old_size >= n) {
        return p;
    }
    size_t need = n + sizeof(Block);
    Block *next = b->next;
    if (next != NULL && next->size >= need - b->size) {
        next->size -= need - b->size;
        b->size = need;
        if (next->size <= sizeof(Block)) {
            b->next = next->next;
        } else {
            b->next = next;
        }
        return p;
    }
    void *np = arena_alloc(n);
    if (np != NULL) {
        memcpy(np, p, old_size);
        arena_free(p);
    }
    return np;
}