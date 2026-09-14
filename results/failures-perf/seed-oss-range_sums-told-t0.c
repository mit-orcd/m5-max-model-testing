<seed:think>
Got it, I need to implement the range_sums function in C11. Let's see what the problem says. The function takes an array a of size n, then q queries each with lo and hi, and we have to compute the sum from a[lo[i]] to a[hi[i]] inclusive for each query, storing the results in out. 

First, the constraints: n and q are in the hundreds of thousands. So we need an efficient solution. If we compute each query naively by iterating from lo to hi every time, that's O(q*n) in the worst case (like if all queries are the entire array), which would be way too slow for 1e5 each—1e10 operations. That's not going to work. So we need a prefix sum array.

Right, prefix sums are the way to go here. The prefix sum array (let's call it pre) where pre[0] = 0, pre[1] = a[0], pre[2] = a[0] + a[1], ..., pre[k] = sum of a[0] to a[k-1]. Then the sum from a[lo] to a[hi] inclusive is pre[hi+1] - pre[lo]. Oh right, because pre[hi+1] is sum up to a[hi], pre[lo] is sum up to a[lo-1], so subtracting gives sum from lo to hi.

So the steps are:

1. Compute the prefix sum array. Since n can be up to 1e5, the prefix array will have size n+1. The elements of a are ints, but the sum can be large. The problem says out is long long, so the prefix sum should also be long long to avoid overflow during calculation.

Wait, the input a is const int *, so each element is int. Summing many ints could overflow a 32-bit integer, so yes, the prefix sum must be long long.

So first, allocate a prefix sum array. But wait, in C, if n is up to 1e5, then n+1 is 1e5+1, which is manageable on the stack? Wait, no—stack size is usually limited (like a few MB). 1e5+1 long longs is 8*(1e5+1) = 800,008 bytes, which is about 800KB. That's probably okay, but maybe better to allocate on the heap? Or maybe use a static array? No, static might not be good if the function is reentrant. Alternatively, since in C11, we can use a VLA (variable length array) if the compiler supports it. But VLAs are allowed in C99 and C11, but some people don't like them because of stack issues. Alternatively, we can dynamically allocate with malloc.

Wait, but for n up to 1e5, even a VLA is manageable. Let's think: 1e5 elements of long long is 800KB, which is way under the typical stack size (which is like 8MB or more). So maybe a VLA is okay. Or maybe use malloc. Let's see.

Alternatively, compute the prefix sum on the fly and store it in a dynamically allocated array. Let's outline:

Compute pre[0] = 0;

for (size_t i = 0; i < n; i++) {
    pre[i+1] = pre[i] + a[i];
}

Then for each query i:

out[i] = pre[hi[i] + 1] - pre[lo[i]];

That's O(n + q) time, which is acceptable for n and q up to 1e5—total 2e5 operations, which is fine.

So the main task is to compute the prefix sum array correctly.

Now, possible edge cases: lo[i] = 0. Then pre[0] is 0, so sum is pre[hi[i]+1] - 0, which is correct (sum from 0 to hi). If lo[i] = hi[i], then sum is a[lo[i]] = pre[lo[i]+1] - pre[lo[i]], which is correct.

What about data types? The input a is int, so when we add to pre (long long), that's okay—int is promoted to long long.

Now, in C, how to handle the prefix array. Let's see: since n can be up to 1e5, we need an array of size n+1. So:

long long *pre = malloc((n + 1) * sizeof(long long));
if (pre == NULL) { /* handle error?