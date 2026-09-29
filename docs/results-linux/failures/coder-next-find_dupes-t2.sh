find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, sort by hash then path
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done | sort -k1,1 -k2,2 > "$tmpfile"
    
    # Group by hash and print groups with 2+ files
    awk -F'\t' '
    {
        if ($1 != prev_hash && NR > 1) {
            if (count >= 2) {
                # Sort the paths and print
                n = asorti(files, sorted)
                for (i = 1; i <= n; i++) {
                    printf "%s%s", sorted[i], (i < n ? " " : "\n")
                }
            }
            delete files
            count = 0
        }
        prev_hash = $1
        files[$2] = 1
        count++
    }
    END {
        if (count >= 2) {
            n = asorti(files, sorted)
            for (i = 1; i <= n; i++) {
                printf "%s%s", sorted[i], (i < n ? " " : "\n")
            }
        }
    }
    ' "$tmpfile"
    
    rm -f "$tmpfile"
}