find_dupes() {
    local dir="$1"
    local tmp_file=$(mktemp)
    
    # Find all regular files and compute MD5 hashes
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file")
        printf '%s\t%s\n' "$hash" "$file"
    done | sort > "$tmp_file"
    
    # Group files by hash and output groups with 2+ files
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            files[++n] = $2
        } else {
            if (n >= 2) {
                # Sort the files array
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (files[i] > files[j]) {
                            tmp = files[i]
                            files[i] = files[j]
                            files[j] = tmp
                        }
                    }
                }
                # Print sorted files
                printf "%s", files[1]
                for (i = 2; i <= n; i++) {
                    printf " %s", files[i]
                }
                printf "\n"
            }
            prev_hash = $1
            n = 1
            files[1] = $2
        }
    }
    END {
        if (n >= 2) {
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (files[i] > files[j]) {
                        tmp = files[i]
                        files[i] = files[j]
                        files[j] = tmp
                    }
                }
            }
            printf "%s", files[1]
            for (i = 2; i <= n; i++) {
                printf " %s", files[i]
            }
            printf "\n"
        }
    }
    ' "$tmp_file" | sort
    
    rm -f "$tmp_file"
}