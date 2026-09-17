/* glob_match.c - shell‑style pattern matching
 *
 * Returns 1 if the string `str` matches the pattern `pat`, otherwise 0.
 * The pattern may contain:
 *   *  – matches any sequence of characters (including the empty sequence)
 *   ?  – matches exactly one character
 * All other characters match themselves.
 *
 * The whole string must be matched.
 *
 * This implementation follows the classic two‑pointer algorithm with
 * backtracking, and conforms to the C11 standard.
 */

int glob_match(const char *pat, const char *str)
{
    const char *p = pat;   /* pointer into pattern */
    const char *s = str;   /* pointer into string */
    const char *star = NULL;   /* last position of '*' in pattern */
    const char *ss   = NULL;   /* position in string corresponding to that '*' */

    while (*s) {
        if (*p == '*') {
            /* remember this '*' and the position in the string,
               then advance pattern pointer */
            star = p++;
            ss = s;
        } else if (*p == '?' || *p == *s) {
            /* '?' matches any single character; otherwise characters must be equal */
            ++p;
            ++s;
        } else if (star) {
            /* mismatch after a previous '*': backtrack,
               let the '*' consume one more character */
            p = star + 1;
            s = ++ss;
        } else {
            /* mismatch and no previous '*' to backtrack to */
            return 0;
        }
    }

    /* Skip any trailing '*' in the pattern */
    while (*p == '*')
        ++p;

    return *p == '\0';
}