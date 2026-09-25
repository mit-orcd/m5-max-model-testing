/* glob_match.c - shell‑style pattern matching
 *
 * Returns 1 if the null‑terminated string `str` matches the pattern `pat`,
 * otherwise returns 0.  The pattern may contain:
 *   *  – matches any sequence of characters (including the empty sequence)
 *   ?  – matches exactly one character
 * All other characters match themselves literally.
 *
 * The whole string must be matched; partial matches are not accepted.
 *
 * This implementation follows the classic two‑pointer algorithm with back‑tracking
 * for '*', running in O(|pat|·|str|) worst‑case time and O(1) extra space.
 */

int glob_match(const char *pat, const char *str)
{
    const char *p = pat;   /* current position in pattern */
    const char *s = str;   /* current position in string  */
    const char *star = NULL;   /* last position of '*' in pattern */
    const char *ss   = NULL;   /* position in string when '*' was seen */

    while (*s) {
        if (*p == '*') {
            /* remember this '*', advance pattern, and remember string position */
            star = p++;
            ss = s;
        } else if (*p == '?' || *p == *s) {
            /* single‑character match */
            p++;
            s++;
        } else if (star) {
            /* mismatch after a previous '*': backtrack */
            p = star + 1;   /* pattern after '*' */
            s = ++ss;       /* try to let '*' consume one more character */
        } else {
            /* no '*' to backtrack to and characters differ */
            return 0;
        }
    }

    /* Consume any trailing '*' in the pattern */
    while (*p == '*')
        ++p;

    return *p == '\0';
}