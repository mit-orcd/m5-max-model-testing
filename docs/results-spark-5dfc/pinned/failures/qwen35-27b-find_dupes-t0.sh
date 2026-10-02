find_dupes() {
    local dir="$1"
    [[ -z "$dir" ]] && return 1

    # Create a temporary file to store hash:path pairs
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find all regular files and compute their MD5 hashes
    # macOS 'find' does not support -printf, so we use -exec
    # macOS 'md5' uses -r for recursive/batch mode but here we pipe filenames
    # We use -exec md5 -q {} \; to get "MD5 (filename) = hash" format, then parse
    # Actually, 'md5 -r' reads from stdin if filename is -, but we need per file.
    # Better approach: find files, then loop or xargs.
    # To handle filenames with spaces/newlines safely, we use -print0 and read -d ''
    
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        # md5 -q outputs: MD5 (filename) = hash
        # We want: hash filename
        hash=$(md5 -q "$file" | awk '{print $NF}')
        printf '%s\t%s\n' "$hash" "$file"
    done > "$tmpfile"

    # Sort by hash, then by path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            paths[hash] = paths[hash] " " path
            count[hash]++
        } else {
            if (count[prev_hash] >= 2) {
                # Sort the paths for this group
                n = split(paths[prev_hash], arr, " ")
                # Simple bubble sort for paths (awk limitation)
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                # Reconstruct sorted string
                sorted_str = arr[1]
                for (i = 2; i <= n; i++) {
                    sorted_str = sorted_str " " arr[i]
                }
                print sorted_str
            }
            prev_hash = hash
            paths[hash] = path
            count[hash] = 1
        }
    }
    END {
        if (count[prev_hash] >= 2) {
            n = split(paths[prev_hash], arr, " ")
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (arr[i] > arr[j]) {
                        tmp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = tmp
                    }
                }
            }
            sorted_str = arr[1]
            for (i = 2; i <= n; i++) {
                sorted_str = sorted_str " " arr[i]
            }
            print sorted_str
        }
    }
    ' | sort
}