find_dupes() {
    local dir="$1"
    [ -d "$dir" ] || return 1
    local tmp
    tmp=$(mktemp -t find_dupes.XXXXXX) || return 1
    trap 'rm -f "$tmp"' RETURN

    find "$dir" -type f -print0 2>/dev/null | while IFS= read -r -d '' f; do
        md5=$(md5 -r "$f" 2>/dev/null | awk '{print $1}')
        [ -n "$md5" ] && printf '%s\t%s\n' "$md5" "$f"
    done | sort -k1,1 -k2,2 > "$tmp"

    awk -F '\t' '
        {
            if ($1 != prev) {
                if (count > 1) {
                    line = paths[prev]
                    for (i = 2; i <= count; i++) line = line " " paths[prev][i]
                    print line
                }
                delete paths[prev]
                count = 0
            }
            prev = $1
            count++
            paths[prev][count] = $2
        }
        END {
            if (count > 1) {
                line = paths[prev]
                for (i = 2; i <= count; i++) line = line " " paths[prev][i]
                print line
            }
        }
    ' "$tmp"
}