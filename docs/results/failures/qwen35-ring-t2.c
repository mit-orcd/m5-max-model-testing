#include <stdint.h>

typedef struct {
    int data[8];
    int head;
    int count;
} Ring;

void ring_init(Ring *r) {
    if (r == NULL) return;
    r->head = 0;
    r->count = 0;
}

int ring_push(Ring *r, int v) {
    if (r == NULL) return -1;
    if (r->count == 8) return -1;

    int tail = (r->head + r->count) % 8;
    r->data[tail] = v;
    r->count++;
    return 0;
}

int ring_pop(Ring *r, int *out) {
    if (r == NULL || out == NULL) return -1;
    if (r->count == 0) return -1;

    *out = r->data[r->head];
    r->head = (r->head + 1) % 8;
    r->count--;
    return 0;
}