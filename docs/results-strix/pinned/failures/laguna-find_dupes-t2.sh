find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    
    # Find all regular files and compute their MD5 hashes
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            md5 -r "$file" | awk '{print $1 "\t" $2}'
        fi
    done < <(find "$dir" -type f -print0) | sort > "$tmpfile"
    
    # Group files by hash and print groups with 2+ files
    awk -F'\t' '
    {
        hash = $1
        file = $2
        files[hash] = files[hash] (files[hash] ? " " : "") file
        count[hash]++
    }
    END {
        # Sort hashes by their first file path
        n = asorti(count, sorted_hashes, "@ind_str_asc")
        
        # Re-sort to get groups ordered by first path
        for (i = 1; i <= n; i++) {
            h = sorted_hashes[i]
            if (count[h] >= 2) {
                # Split files and sort them
                nfiles = split(files[h], arr, " ")
                # Bubble sort the file paths
                for (j = 1; j <= nfiles; j++) {
                    for (k = j + 1; k <= nfiles; k++) {
                        if (arr[j] > arr[k]) {
                            tmp = arr[j]
                            arr[j] = arr[k]
                            arr[k] = tmp
                        }
                    }
                }
                # Print sorted file paths
                printf "%s", arr[1]
                for (j = 2; j <= nfiles; j++) {
                    printf " %s", arr[j]
                }
                print ""
            }
        }
    }' "$tmpfile"
    
    rm -f "$tmpfile"
}