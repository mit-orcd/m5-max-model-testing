int glob_match(const char *pat, const char *str) {
    int plen = 0, slen = 0;
    for (const char *p = pat; *p; p++, plen++);
    for (const char *s = str; *s; s++, slen++);

    int **dp = malloc((plen + 1) * sizeof(int *));
    for (int i = 0; i <= plen; i++)
        dp[i] = malloc((slen + 1) * sizeof(int));

    dp[0][0] = 1;
    for (int i = 1; i <= plen; i++) {
        if (pat[i - 1] == '*')
            dp[i][0] = dp[i - 1][0];
        else
            dp[i][0] = 0;
    }

    for (int i = 1; i <= plen; i++) {
        for (int j = 1; j <= slen; j++) {
            if (pat[i - 1] == '*')
                dp[i][j] = dp[i - 1][j] || dp[i][j - 1];
            else if (pat[i - 1] == '?' || pat[i - 1] == str[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = 0;
        }
    }

    int result = dp[plen][slen];
    for (int i = 0; i <= plen; i++) free(dp[i]);
    free(dp);
    return result;
}