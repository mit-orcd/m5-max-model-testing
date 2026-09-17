find_dupes() {
    local dir=$1
    local tmp=$(mktemp -t finddupes.XXXXXX) || return
    trap 'rm -f "$tmp"' RETURN

    # List files, compute hash, store as "hash<TAB>path"
    find "$dir" -type f -print0 |
    while IFS= read -r -d '' f; do
        hash=$(md5 -q "$f")
        printf '%s\t%s\n' "$hash" "$f"
    done > "$tmp"

    # Sort by hash then path, then collapse identical hashes into groups
    sort -k1,1 -k2,2 "$tmp" |
    awk -F'\t' '
        {
            if ($1 != prev_hash) {
                if (count > 1) print group
                group = $2
                count = 1
            } else {
                group = group " " $2
                count++
            }
            prev_hash = $1
        }
        END {
            if (count > 1) print group
        }
    ' |
    # Finally sort groups by their first path
    sort -k1,1
}