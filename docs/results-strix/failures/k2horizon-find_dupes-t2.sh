find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -d) || return 1
    trap 'rm -rf "$tmp"' RETURN

    # Collect regular files under dir (recursive), skipping symlinks by default
    find "$dir" -type f -print0 | while IFS= read -r -d '' f; do
        # Compute MD5 of file content using macOS md5 -r
        hash=$(md5 -r "$f" 2>/dev/null | awk '{print $1}')
        if [ -n "$hash" ]; then
            printf '%s\t%s\n' "$hash" "$f"
        fi
    done | sort -k1,1 -k2,2 | awk -F'\t' '
        {
            hash = $1
            path = $2
            files[hash] = files[hash] " " path
            count[hash]++
        }
        END {
            for (h in files) {
                if (count[h] >= 2) {
                    paths = files[h]
                    n = split(paths, arr, " ")
                    # arr[1] is empty due to leading space
                    out = arr[2]
                    for (i = 3; i <= n; i++) out = out " " arr[i]
                    # Split into array to sort
                    split(out, p, " ")
                    for (i = 1; i <= length(p); i++)
                        for (j = i + 1; j <= length(p); j++)
                            if (p[j] < p[i]) { t = p[i]; p[i] = p[j]; p[j] = t }
                    line = p[1]
                    for (i = 2; i <= length(p); i++) line = line " " p[i]
                    first = p[1]
                    print first "\t" line
                }
            }
        }
    ' | sort -k1,1 | cut -f2-
}