/* glob_match.c
 * Shell‑style pattern matching: '*' = any sequence (including empty),
 * '?' = any single character, other characters match literally.
 * Returns 1 on match, 0 otherwise.
 * Conforms to C11.
 */

int glob_match(const char *pat, const char *str)
{
    const char *p = pat;   /* pattern pointer */
    const char *s = str;   /* string pointer */
    const char *star = NULL;   /* last '*' in pattern */
    const char *ss   = NULL;   /* position in string after last '*'-matched chars */

    while (*s) {
        if (*p == '*') {
            /* remember position of '*', advance pattern */
            star = p++;
            ss = s;
        } else if (*p == '?' || *p == *s) {
            /* single‑character match */
            p++;
            s++;
        } else if (star) {
            /* mismatch after a previous '*': backtrack */
            p = star + 1;   /* pattern resumes after '*' */
            s = ++ss;       /* consume one more character from string */
        } else {
            return 0;       /* no '*' to backtrack to and characters differ */
        }
    }

    /* Consume trailing '*' in pattern */
    while (*p == '*')
        ++p;

    return (*p == '\0');
}