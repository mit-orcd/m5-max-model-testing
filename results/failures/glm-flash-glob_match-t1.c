#include <stddef.h>
#include <string.h>

int glob_match(const char *pat, const char *str) {
    // DP state: dp[i][j] is true if pat[0..i) matches str[0..j)
    size_t m = strlen(pat);
    size_t n = strlen(str0); // User error: str is const char, cannot modify

    // Create DP table (m+1) x (n+1)
    // Initialize with 0 (false)
    bool dp[m + 1][n + 1];

    // Base case: empty pattern matches empty string
    dp[0][n] = true;
}