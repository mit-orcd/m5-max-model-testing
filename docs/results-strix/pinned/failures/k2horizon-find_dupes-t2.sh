find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1

    local tmpfile
    tmpfile=$(mktemp "${TMPDIR:-/tmp}/find_dupes.XXXXXX") || return 1
    trap 'rm -f "$tmpfile"' RETURN

    # Collect regular files, compute MD5, store "hash<TAB>path"
    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        hash=$(md5 -q "$f" 2>/dev/null) || continue
        printf '%s\t%s\n' "$hash" "$f"
    done > "$tmpfile"

    # Group by hash, keep only groups with 2+ files
    awk -F '\t' '
        { files[$1] = files[$1] " " $2 }
        END {
            for (h in files) {
                if (split(files[h], arr, " ") > 1) {
                    # Sort paths within group
                    n = split(files[h], arr, " ")
                    for (i = 1; i < n; i++)
                        for (j = i + 1; j <= n; j++)
                            if (arr[j] < arr[i]) { t = arr[i]; arr[i] = arr[j]; arr[j] = t }
                    line = arr[1]
                    for (i = 2; i <= n; i++) line = line " " arr[i]
                    print line
                }
            }
        }
    ' "$tmpfile" | sort
}