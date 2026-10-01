find_dupes() {
    local dir="$1"
    local tmp_file=$(mktemp)
    
    # Find all regular files and compute MD5 hashes
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | cut -d' ' -f1)
        printf '%s\t%s\n' "$hash" "$file"
    done | sort > "$tmp_file"
    
    # Group files by hash and output groups with 2+ files
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            files[NR] = $2
            count++
        } else {
            if (count >= 2) {
                # Sort the collected files
                n = asorti(files, sorted_idx)
                for (i = 1; i <= n; i++) {
                    printf "%s%s", files[sorted_idx[i]], (i < n ? " " : "\n")
                }
            }
            delete files
            files[NR] = $2
            count = 1
            prev_hash = $1
        }
    }
    END {
        if (count >= 2) {
            n = asorti(files, sorted_idx)
            for (i = 1; i <= n; i++) {
                printf "%s%s", files[sorted_idx[i]], (i < n ? " " : "\n")
            }
        }
    }
    ' "$tmp_file" | sort
    
    rm -f "$tmp_file"
}