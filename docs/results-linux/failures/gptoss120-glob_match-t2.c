/* glob_match.c - shell‑style wildcard matching
 * Returns 1 if `str` matches the pattern `pat`, otherwise 0.
 * '*'  matches any sequence of characters (including the empty sequence)
 * '?'  matches exactly one arbitrary character
 * all other characters match themselves.
 * The whole string must be consumed.
 * Conforms to C11.
 */

int glob_match(const char *pat, const char *str)
{
    const char *p = pat;      /* current position in pattern */
    const char *s = str;      /* current position in string  */
    const char *star = NULL;  /* last '*' encountered in pattern */
    const char *ss   = NULL;  /* position in string at the time of that '*' */

    while (*s) {
        if (*p == '*') {
            star = p++;       /* remember position of '*' */
            ss = s;           /* remember where we were in the string */
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else if (star) {
            /* mismatch after a previous '*': backtrack */
            p = star + 1;
            s = ++ss;
        } else {
            return 0;         /* no '*' to backtrack to and characters differ */
        }
    }

    /* Consume trailing '*' in the pattern */
    while (*p == '*')
        ++p;

    return *p == '\0';
}