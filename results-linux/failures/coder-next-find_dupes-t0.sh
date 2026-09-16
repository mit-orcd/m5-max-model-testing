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
        hash = $1
        path = $2
        if (hash in files) {
            files[hash] = files[hash] "\t" path
            count[hash]++
        } else {
            files[hash] = path
            count[hash] = 1
        }
    }
    END {
        for (hash in files) {
            if (count[hash] >= 2) {
                n = split(files[hash], arr, "\t")
                # Sort the paths
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                # Print sorted paths space-separated
                line = arr[1]
                for (i = 2; i <= n; i++) {
                    line = line " " arr[i]
                }
                print line
            }
        }
    }
    ' "$tmp_file" | sort
    
    rm -f "$tmp_file"
}