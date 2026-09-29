#include <stdalign.h2>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    int data[8];
    int head;
    int count;
} Ring;

void ring_init(Ring *r) {
    r->head = 0;
    r->count = 0;
}

int ring_push(Ring *r, int v) {
    if (r->count == 8) {
        return -1;
    }
    r->data[(r->head + r->count) % 8] = v;
    r->count++;
    return 0;
}

int ring_pop(Ring *r, int *out) {
    if (r->count == 0) {
        return -1;
    }
    *out = r->data[r->head];
    r->head = (r->head + 1) % 8;
    r->count--;
    return 0;
}