find_dupes() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo "find_dupes: $dir: No such directory" >&2
        return 1
    fi

    find "$dir" -type f -print0 | \
    while IFS= read -r -d '' file; do
        local hash
        hash=$(md5 -q "$file")
        printf '%s\t%s\n' "$hash" "$file"
    done | \
    sort -t$'\t' -k1,1 -k2,2 | \
    awk -F'\t' '
    {
        if ($1 == prev) {
            if (count == 1) {
                line = $2
            } else {
                line = line " " $2
            }
            count++
        } else {
            if (count > 1) print line
            prev = $1
            count = 1
            line = $2
        }
    }
    END {
        if (count > 1) print line
    }
    ' | sort -k1,1
}