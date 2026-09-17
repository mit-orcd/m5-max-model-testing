find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    trap 'rm -f "$tmp"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5=$(md5 -q "$f" 2>/dev/null) || continue
        printf '%s\t%s\n' "$md5" "$f"
    done > "$tmp"

    awk -F '\t' '
        { files[$1] = files[$1] " " $2 }
        END {
            for (h in files) {
                if (split(files[h], a, " ") > 1) {
                    n = split(files[h], a, " ")
                    for (i = 1; i <= n; i++) paths[i] = a[i]
                    for (i = 1; i <= n; i++)
                        for (j = i + 1; j <= n; j++)
                            if (paths[i] > paths[j]) { t = paths[i]; paths[i] = paths[j]; paths[j] = t }
                    line = paths[1]
                    for (i = 2; i <= n; i++) line = line " " paths[i]
                    print line
                }
            }
        }
    ' "$tmp" | sort
}