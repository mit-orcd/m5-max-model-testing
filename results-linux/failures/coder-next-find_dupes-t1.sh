find_dupes() {
    local dir="$1"
    [ -z "$dir" ] && { echo "Usage: find_dupes <dir>" >&2; return 1; }
    [ ! -d "$dir" ] && { echo "Error: '$dir' is not a directory" >&2; return 1; }
    
    # Create temp file for hash-path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT
    
    # Find all regular files, compute MD5, store "hash path" pairs
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | cut -d' ' -f1)
        printf '%s\t%s\n' "$hash" "$file"
    done | sort -t$'\t' -k1,1 -k2,2 > "$tmpfile"
    
    # Group by hash and output groups with 2+ files
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            files[++n] = $2
        } else {
            if (n >= 2) {
                # Sort files and print as space-separated
                for (i = 1; i <= n; i++) sorted[i] = files[i]
                asort(sorted)
                for (i = 1; i <= n; i++) printf "%s%s", sorted[i], (i < n ? " " : "\n")
            }
            prev_hash = $1
            n = 1
            files[1] = $2
        }
    }
    END {
        if (n >= 2) {
            for (i = 1; i <= n; i++) sorted[i] = files[i]
            asort(sorted)
            for (i = 1; i <= n; i++) printf "%s%s", sorted[i], (i < n ? " " : "\n")
        }
    }
    ' "$tmpfile" | sort
}