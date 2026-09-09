
#include <stdio.h>
typedef struct { int data[8]; int head; int count; } Ring;
void ring_init(Ring *r); int ring_push(Ring *r, int v); int ring_pop(Ring *r, int *out);
static int fails = 0;
int main(void) {
    Ring r; int v; ring_init(&r);
    if (ring_pop(&r, &v) != -1) { printf("FAIL pop empty\n"); fails++; }
    for (int i = 0; i < 8; i++) if (ring_push(&r, i) != 0) { printf("FAIL push %d\n", i); fails++; }
    if (ring_push(&r, 99) != -1) { printf("FAIL push full\n"); fails++; }
    for (int i = 0; i < 4; i++) { if (ring_pop(&r, &v) != 0 || v != i) { printf("FAIL pop got %d want %d\n", v, i); fails++; } }
    for (int i = 8; i < 12; i++) if (ring_push(&r, i) != 0) { printf("FAIL wrap push %d\n", i); fails++; }
    for (int i = 4; i < 12; i++) { if (ring_pop(&r, &v) != 0 || v != i) { printf("FAIL wrap pop got %d want %d\n", v, i); fails++; } }
    if (ring_pop(&r, &v) != -1) { printf("FAIL pop empty2\n"); fails++; }
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

