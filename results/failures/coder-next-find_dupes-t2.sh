find_dupes() {
    local dir="$1"
    if [[ -z "$dir" || ! -d "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi
    
    # Create temp file for hash-file mapping
    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT
    
    # Compute MD5 for all regular files and store hash and path
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file")
        printf '%s\t%s\n' "$hash" "$file"
    done > "$tmpfile"
    
    # Group by hash, filter groups with duplicates, sort paths within groups, sort groups by first path
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (!(hash in first)) {
            first[hash] = path
        }
        paths[hash] = paths[hash] (paths[hash] ? " " : "") path
        count[hash]++
    }
    END {
        n = 0
        for (hash in count) {
            if (count[hash] > 1) {
                # Sort paths within group
                split(paths[hash], arr, " ")
                len = asort(arr)
                sorted_paths = ""
                for (i = 1; i <= len; i++) {
                    sorted_paths = sorted_paths (sorted_paths ? " " : "") arr[i]
                }
                groups[++n] = sorted_paths
                first_path[n] = arr[1]
            }
        }
        # Sort groups by first path
        for (i = 1; i <= n; i++) {
            for (j = i + 1; j <= n; j++) {
                if (first_path[i] > first_path[j]) {
                    tmp = first_path[i]; first_path[i] = first_path[j]; first_path[j] = tmp
                    tmp = groups[i]; groups[i] = groups[j]; groups[j] = tmp
                }
            }
        }
        for (i = 1; i <= n; i++) {
            print groups[i]
        }
    }
    ' "$tmpfile"
}