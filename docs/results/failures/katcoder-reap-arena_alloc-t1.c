#include <stdint.h>
#include <stddef.h>

#define ALLOC_MAGIC 0xDEADBEEF

typedef struct Node {
    uint32_t magic;
    size_t   size;
    int      free;
    struct  Node *next;
} Node;

static void *g_arena_buf;
static size_t g_arena_size;
static Node   g_freelist;
static size_t g_free_count;

void arena_init(void *buf, size_t size) {
    g_arena_buf = buf;
    g_arena_size = size;
    g_freelist.next = 0;
    g_freelist.size = 0;
    g_freelist.free = 1;
    g_free_count = 0;
}

static size_t buf_offset(void *p) {
    return (size_t)((uintptr_t)p - (uintptr_t)g_arena_buf);
}

static int  find_free(size_t n, size_t *out_off) {
    uint8_t *b = (uint8_t*)g_arena_buf;
    size_t    end  = g_arena_size - sizeof(Node);
    size_t    off  = 0;
    size_t    best = (size_t)(-1);

    while (off < end) {
        Node *nd = (Node*)(b + off);
        if (nd->magic != ALLOC_MAGIC) return -1;
        if (nd->free) {
            if (nd->size >= n) {
                if (best == (size_t)(-1) || nd->size < ((Node*)(b + best))->size)
                    best = off;
                if (nd->size == n) break;
            }
            off += sizeof(Node) + nd->size;
        } else {
            off += sizeof(Node) + nd->size;
            Node *nx = (Node*)(b + off);
            if (off >= end || nx->magic != ALLOC_MAGIC) return -1;
        }
    }
    *out_off = best;
    return 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t off;
    if (find_free(n, &off)) return NULL;

    uint8_t *b = (uint8_t*)g_arena_buf;
    Node    *nd = (Node*)(b + off);
    size_t   rest = nd->size - n;
    size_t   next = off + sizeof(Node) + n;
    size_t   end  = g_arena_size - sizeof(Node);

    nd->size = n;
    nd->free = 0;
    g_free_count++;

    if (rest == 0 || next >= end) return (void*)(b + off + sizeof(Node));

    Node *nx = (Node*)(b + next);
    nx->magic = ALLOC_MAGIC;
    nx->size  = rest;
    nx->free  = 1;
    nx->next  = nd->next;

    nd->next = nx;
    g_freelist.next = nd;
    return (void*)(b + off + sizeof(Node));
}

void arena_free(void *p) {
    if (!p) return;
    uint8_t *b = (uint8_t*)g_arena_buf;
    Node    *nd = (Node*)((uintptr_t)p - sizeof(Node) - (uintptr_t)g_arena_buf);
    if (nd->magic != ALLOC_MAGIC || nd->free) return;

    nd->free = 1;
    g_free_count--;

    /* coalesce with next */
    if (nd->next && nd->next->free) {
        Node *nx = nd->next;
        nd->size += sizeof(Node) + nx->size;
        nd->next  = nx->next;
        g_free_count--;
        if (g_freelist.next == nx) g_freelist.next = nd;
    }

    /* coalesce with prev */
    Node *prev = 0;
    Node *cur  = &g_freelist;
    size_t off = 0;
    size_t end = g_arena_size - sizeof(Node);
    while (cur) {
        if (cur == nd) {
            if (prev && prev->free) {
                prev->size += sizeof(Node) + nd->size;
                prev->next  = nd->next;
                if (g_freelist.next == nd) g_freelist.next = prev;
            }
            break;
        }
        off  += sizeof(Node) + cur->size;
        prev  = cur;
        cur   = cur->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return NULL;
    uint8_t *b = (uint8_t*)g_arena_buf;
    Node    *nd = (Node*)((uintptr_t)p - sizeof(Node) - (uintptr_t)g_arena_buf);
    if (nd->magic != ALLOC_MAGIC || nd->free) return NULL;

    size_t old = nd->size;
    if (n <= old) {
        if (n == 0) { arena_free(p); return NULL; }
        /* shrink trailing free space */
        if (nd->next && nd->next->free) {
            size_t new_size = n;
            size_t rest     = old - new_size;
            size_t next_off = (size_t)((uintptr_t)p - (uintptr_t)g_arena_buf) + sizeof(Node) + new_size;
            size_t end      = g_arena_size - sizeof(Node);
            if (next_off <= end) {
                Node *nx = (Node*)(b + next_off);
                nx->magic = ALLOC_MAGIC;
                nx->size  = rest;
                nx->free  = 1;
                nx->next  = nd->next;
                nd->size  = new_size;
                nd->next  = nx;
                g_freelist.next = nd;
            }
        }
        return p;
    }

    size_t diff = n - old;
    if (nd->next && nd->next->free && nd->next->size >= diff) {
        size_t new_size = n;
        size_t next_off = (size_t)((uintptr_t)p - (uintptr_t)g_arena_buf) + sizeof(Node) + old;
        size_t end      = g_arena_size - sizeof(Node);
        if (next_off <= end) {
            Node *nx = (Node*)(b + next_off);
            size_t surplus = nx->size - diff;
            if (surplus == 0) {
                nd->size  = new_size;
                nd->next  = nx->next;
                g_free_count--;
                if (g_freelist.next == nx) g_freelist.next = nd;
            } else {
                nx->size  = surplus;
                nd->size  = new_size;
            }
            return p;
        }
    }

    void *np = arena_alloc(n);
    if (!np) return NULL;
    size_t copy = old < n ? old : n;
    size_t off_old = (size_t)((uintptr_t)p - (uintptr_t)g_arena_buf) + sizeof(Node);
    size_t off_new = (size_t)((uintptr_t)np - (uintptr_t)g_arena_buf);
    if (g_arena_buf) {
        uint8_t *src = (uint8_t*)g_arena_buf;
        uint8_t *dst = (uint8_t*)g_arena_buf;
        size_t   src_off = (size_t)((uintptr_t)p - (uintptr_t)g_arena_buf) + sizeof(Node);
        size_t   dst_off = (size_t)((uintptr_t)np - (uintptr_t)g_arena_buf);
        if (src_off < dst_off) {
            for (size_t i = 0; i < copy; i++)
                dst[dst_off + i] = src[src_off + i];
        } else {
            for (size_t i = copy; i > 0; i--)
                dst[dst_off + i - 1] = src[src_off + i - 1];
        }
    }
    arena_free(p);
    return np;
}