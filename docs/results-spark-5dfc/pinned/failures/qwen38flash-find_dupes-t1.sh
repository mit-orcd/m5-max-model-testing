find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, store "md5 path"
    # Using md5 -r on macOS which outputs "md5 filename"
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
        # md5 -r output format: "hash  filename" (two spaces before filename)
        # Extract hash and path
        local hash=$(echo "$line" | cut -d' ' -f1)
        local path=$(echo "$line" | cut -d' ' -f2-)
        echo "$hash $path"
    done > "$tmpfile"
    
    # Group by hash, sort paths within each group, filter groups with 2+ files
    # Then sort groups by first path
    sort "$tmpfile" | awk '
        {
            hash = $1
            path = substr($0, length(hash) + 2)
            if (hash != prev_hash) {
                if (count > 1) {
                    # Print the group
                    printf "%s", paths[1]
                    for (i = 2; i <= count; i++) {
                        printf " %s", paths[i]
                    }
                    printf "\n"
                }
                count = 0
                delete paths
                prev_hash = hash
            }
            count++
            paths[count] = path
        }
        END {
            if (count > 1) {
                printf "%s", paths[1]
                for (i = 2; i <= count; i++) {
                    printf " %s", paths[i]
                }
                printf "\n"
            }
        }
    ' | sort
    
    rm -f "$tmpfile"
}