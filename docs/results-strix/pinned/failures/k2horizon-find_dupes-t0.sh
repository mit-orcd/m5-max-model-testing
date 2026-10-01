find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1

    local tmpfile
    tmpfile=$(mktemp "${TMPDIR:-/tmp}/find_dupes.XXXXXX") || return 1
    trap 'rm -f "$tmpfile"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5 -r "$f" 2>/dev/null | awk -v path="$f" '{print $1 " " path}' >> "$tmpfile"
    done

    awk '{ if (length($1) == 32) { hash[$1] = hash[$1] " " $2 } } END { for (h in hash) { if (split(hash[h], a, " ") > 1) { n = split(hash[h], a, " "); asort(a); line = a[1]; for (i = 2; i <= n; i++) line = line " " a[i]; print line } } }' "$tmpfile" | sort
}