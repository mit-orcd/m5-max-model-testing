find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    # Create temp file for hash-path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find all regular files and compute MD5 hashes
    # Use md5 -r for BSD/macOS compatibility
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done > "$tmpfile"

    # Sort by hash, then by path, and group duplicates
    sort -t $'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        if ($1 == prev_hash) {
            paths[NR] = $2
            count++
        } else {
            if (count > 1) {
                # Sort the collected paths for this group
                n = asorti(paths, sorted_paths)
                for (i = 1; i <= n; i++) {
                    printf "%s%s", sorted_paths[i], (i < n ? " " : "\n")
                }
            }
            delete paths
            count = 1
            paths[NR] = $2
            prev_hash = $1
        }
    }
    END {
        if (count > 1) {
            n = asorti(paths, sorted_paths)
            for (i = 1; i <= n; i++) {
                printf "%s%s", sorted_paths[i], (i < n ? " " : "\n")
            }
        }
    }' | sort
}