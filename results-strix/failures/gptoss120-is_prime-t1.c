/* is_prime.c - C11 implementation */
#include <limits.h>
#include <math.h>

/* Returns 1 if n is a prime number, 0 otherwise.
   n <= 1 (including negatives) are not prime. */
int is_prime(int n)
{
    if (n <= 1) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0) return 0;

    int limit = (int)sqrt((double)n);
    for (int i = 3; i <= limit; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}