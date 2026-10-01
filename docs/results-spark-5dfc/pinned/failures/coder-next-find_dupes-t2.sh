find_dupes() {
    local dir="$1"
    local tmp_file
    tmp_file=$(mktemp)
    
    # Find all regular files, compute MD5, and store hash + path
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | cut -d' ' -f1)
        printf '%s\t%s\n' "$hash" "$file"
    done | sort -t$'\t' -k1,1 -k2,2 > "$tmp_file"
    
    # Group files by hash and output groups with 2+ files
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            files = files " " $2
            count++
        } else {
            if (count >= 2) {
                # Sort the files in the group
                n = split(files, arr, " ")
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                sorted_files = arr[1]
                for (i = 2; i <= n; i++) {
                    sorted_files = sorted_files " " arr[i]
                }
                print sorted_files
            }
            prev_hash = $1
            files = $2
            count = 1
        }
    }
    END {
        if (count >= 2) {
            n = split(files, arr, " ")
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (arr[i] > arr[j]) {
                        tmp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = tmp
                    }
                }
            }
            sorted_files = arr[1]
            for (i = 2; i <= n; i++) {
                sorted_files = sorted_files " " arr[i]
            }
            print sorted_files
        }
    }
    ' "$tmp_file" | sort
    
    rm -f "$tmp_file"
}