find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    
    # Find all regular files and compute their MD5 hashes
    while IFS= read -r -d '' file; do
        # Get MD5 hash using macOS's md5 command
        hash=$(md5 -r "$file" | awk '{print $1}')
        echo "$hash"$'\t'"$file"
    done < <(find "$dir" -type f -print0) > "$tmpfile"
    
    # Group files by hash, sort each group, and output groups with 2+ files
    awk -F'\t' '
    {
        hash = $1
        file = $2
        files[hash] = files[hash] (files[hash] ? " " : "") file
    }
    END {
        # Sort files within each group
        for (hash in files) {
            n = split(files[hash], arr, " ")
            # Sort the array
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (arr[i] > arr[j]) {
                        tmp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = tmp
                    }
                }
            }
            # Rebuild sorted string
            sorted = arr[1]
            for (i = 2; i <= n; i++) {
                sorted = sorted " " arr[i]
            }
            files[hash] = sorted
        }
        
        # Sort groups by first path and print those with 2+ files
        n = 0
        for (hash in files) {
            n++
            groups[n] = files[hash]
            first_paths[n] = (split(files[hash], tmp, " ") ? tmp[1] : "")
        }
        
        # Sort by first path
        for (i = 1; i <= n; i++) {
            for (j = i + 1; j <= n; j++) {
                if (first_paths[i] > first_paths[j]) {
                    tmp = groups[i]
                    groups[i] = groups[j]
                    groups[j] = tmp
                    tmp = first_paths[i]
                    first_paths[i] = first_paths[j]
                    first_paths[j] = tmp
                }
            }
        }
        
        # Print groups with 2+ files
        for (i = 1; i <= n; i++) {
            if (split(groups[i], tmp, " ") >= 2) {
                print groups[i]
            }
        }
    }' "$tmpfile"
    
    rm -f "$tmpfile"
}