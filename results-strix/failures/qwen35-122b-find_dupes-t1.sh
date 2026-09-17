find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)

    trap 'rm -f "$tmpfile"' EXIT

    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    while IFS= read -r hash filepath; do
        printf '%s\t%s\n' "$hash" "$filepath"
    done | sort -k1,1 -k2,2 > "$tmpfile"

    awk '
    {
        hash = $1
        $1 = ""
        path = substr($0, 2)
        if (hash == prev_hash) {
            paths[hash] = paths[hash] " " path
            counts[hash]++
        } else {
            if (counts[prev_hash] >= 2) {
                print paths[prev_hash]
            }
            prev_hash = hash
            paths[hash] = path
            counts[hash] = 1
        }
    }
    END {
        if (counts[prev_hash] >= 2) {
            print paths[prev_hash]
        }
    }
    ' "$tmpfile"
}