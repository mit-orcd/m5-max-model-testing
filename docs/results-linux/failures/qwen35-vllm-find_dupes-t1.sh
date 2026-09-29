find_dupes() {
    local dir="$1"
    [ -z "$dir" ] && return 1
    
    # Use find with -exec to run md5 for each file, then sort and group
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | \
    awk '{
        # md5 -r output format: "hash  filename" (two spaces) or "hash  -filename"
        hash = $1
        # Remove the leading "hash  " (hash + 2 spaces)
        idx = index($0, "  ")
        if (idx > 0) {
            filename = substr($0, idx + 2)
        } else {
            # Fallback if format varies slightly
            filename = $2
        }
        files[hash] = files[hash] " " filename
        count[hash]++
    }
    END {
        for (h in count) {
            if (count[h] >= 2) {
                n = split(files[h], arr, " ")
                # Remove first empty element caused by leading space
                delete arr[1]
                # Re-index
                j = 1
                for (i = 2; i <= n; i++) {
                    if (arr[i] != "") {
                        sorted[j++] = arr[i]
                    }
                }
                # Sort the paths for this hash
                m = asort(sorted)
                line = ""
                for (k = 1; k <= m; k++) {
                    if (k > 1) line = line " "
                    line = line sorted[k]
                }
                # Store for final sorting
                result[line] = 1
                delete sorted
            }
        }
        # Sort groups by first path
        n = asorti(result, sorted_groups)
        for (i = 1; i <= n; i++) {
            print sorted_groups[i]
        }
    }'
}