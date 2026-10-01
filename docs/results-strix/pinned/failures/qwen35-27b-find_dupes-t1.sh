find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" EXIT

    # Find regular files, compute MD5, output "hash path"
    # macOS 'find' does not support -printf, so we use -exec
    # macOS 'md5' uses -r for recursive/batch mode but here we pipe filenames
    # We use a loop to handle filenames with spaces/newlines safely
    while IFS= read -r -d '' file; do
        hash=$(md5 -q "$file" | awk '{print $1}')
        printf '%s\t%s\n' "$hash" "$file"
    done < <(find "$dir" -type f -print0) > "$tmpfile"

    # Sort by hash then path
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            paths[hash] = paths[hash] " " path
            count[hash]++
        } else {
            if (count[prev_hash] >= 2) {
                # Sort paths within the group
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
    }' | sort
}