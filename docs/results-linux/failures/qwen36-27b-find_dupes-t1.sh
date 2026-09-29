find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        return 1
    fi

    # Create a temporary file to store hash and path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find all regular files, compute MD5 hash, and store with path
    # On macOS/BSD: md5 -r outputs "hash *filename"
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file")
        # Extract just the hash part (before the space and asterisk)
        hash_val="${hash%% *}"
        printf '%s\t%s\n' "$hash_val" "$file"
    done > "$tmpfile"

    # Sort by hash, then by path to group duplicates and sort within groups
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            group = group " " path
            count++
        } else {
            if (count >= 2) {
                # Sort the paths in the group
                n = split(group, paths, " ")
                # Simple bubble sort for paths
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (paths[i] > paths[j]) {
                            tmp = paths[i]
                            paths[i] = paths[j]
                            paths[j] = tmp
                        }
                    }
                }
                # Reconstruct sorted group
                sorted_group = paths[1]
                for (i = 2; i <= n; i++) {
                    sorted_group = sorted_group " " paths[i]
                }
                print sorted_group
            }
            group = path
            count = 1
            prev_hash = hash
        }
    }
    END {
        if (count >= 2) {
            n = split(group, paths, " ")
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (paths[i] > paths[j]) {
                        tmp = paths[i]
                        paths[i] = paths[j]
                        paths[j] = tmp
                    }
                }
            }
            sorted_group = paths[1]
            for (i = 2; i <= n; i++) {
                sorted_group = sorted_group " " paths[i]
            }
            print sorted_group
        }
    }
    ' | sort
}