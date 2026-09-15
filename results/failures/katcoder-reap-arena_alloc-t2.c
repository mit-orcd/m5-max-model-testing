#include <stdint.h>
#include <stddef.h>

typedef struct Node {
    size_t offset;
    size_t size;
    int    free;
    struct Node *next;
} Node;

void *arena_buf;
size_t arena_size;
size_t arena_used;
struct Node *arena_head;

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    arena_head = (struct Node *)buf;
    arena_head->next = NULL;
    arena_head->offset = 0;
    arena_head->size = size;
    arena_head->free = 1;
}

void *arena_alloc(size_t n) {
    if (!arena_buf || !n) return NULL;
    for (struct Node *cur = arena_head; cur; cur = cur->next) {
        if (cur->free && cur->size >= n) {
            size_t remain = cur->size - n;
            if (remain == 0) {
                cur->free = 0;
                arena_used += n;
                return (char *)arena_buf + cur->offset;
            }
            struct Node *new_node = (struct Node *)((char *)arena_buf + cur->offset + n);
            new_node->offset = cur->offset + n;
            new_node->size = remain;
            new_node->free = 1;
            new_node->next = cur->next;
            cur->size = n;
            cur->free = 0;
            cur->next = new_node;
            arena_used += n;
            return (char *)arena_buf + cur->offset;
        }
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p || !arena_buf) return;
    size_t off = (size_t)((char *)p - (char *)arena_buf);
    for (struct Node *cur = arena_head; cur; cur = cur->next) {
        if (cur->offset == off) {
            cur->free = 1;
            arena_used -= cur->size;
            if (cur->next && cur->next->free) {
                cur->size += cur->next->size;
                cur->next = cur->next->next;
            }
            break;
        }
        if (cur->next && cur->next->offset == cur->offset + cur->size && cur->next->free) {
            cur->size += cur->next->size;
            cur->next = cur->next->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p || !arena_buf) return NULL;
    if (!n) { arena_free(p); return NULL; }
    size_t off = (size_t)((char *)p - (char *)arena_buf);
    for (struct Node *cur = arena_head; cur; cur = cur->next) {
        if (cur->offset == off && !cur->free) {
            size_t old_size = cur->size;
            if (cur->size >= n) {
                if (cur->size > n) {
                    size_t remain = cur->size - n;
                    cur->size = n;
                    struct Node *new_node = (struct Node *)((char *)arena_buf + cur->offset + n);
                    new_node->offset = cur->offset + n;
                    new_node->size = remain;
                    new_node->free = 1;
                    new_node->next = cur->next;
                    cur->next = new_node;
                }
                memmove(p, p, n < old_size ? n : old_size);
                return p;
            }
            size_t needed = n - cur->size;
            if (cur->next && cur->next->free && cur->next->size >= needed) {
                size_t old_next_size = cur->next->size;
                cur->size = n;
                cur->next = cur->next->next;
                memmove((char *)p + old_size, (char *)p + old_size, n - old_size);
                return p;
            }
            void *new_p = arena_alloc(n);
            if (!new_p) return NULL;
            memmove(new_p, p, n < old_size ? n : old_size);
            arena_free(p);
            return new_p;
        }
    }
    return NULL;
}