find_dupes() {
    local dir="$1"
    local tmp_file
    tmp_file=$(mktemp)
    
    # Find all regular files, compute MD5, store hash and path
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done | sort > "$tmp_file"
    
    # Group by hash and output groups with 2+ files
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            paths[NR] = $2
            count++
        } else {
            if (count >= 2) {
                # Sort paths and print
                n = asorti(paths, sorted_idx)
                for (i = 1; i <= n; i++) {
                    printf "%s%s", paths[sorted_idx[i]], (i < n ? " " : "\n")
                }
            }
            delete paths
            paths[NR] = $2
            prev_hash = $1
            count = 1
        }
    }
    END {
        if (count >= 2) {
            n = asorti(paths, sorted_idx)
            for (i = 1; i <= n; i++) {
                printf "%s%s", paths[sorted_idx[i]], (i < n ? " " : "\n")
            }
        }
    }
    ' "$tmp_file" | sort
    
    rm -f "$tmp_file"
}