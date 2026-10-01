find_dupes() {
    local dir="$1"
    if [[ -z "$dir" ]]; then
        echo "Usage: find_dupes <dir>" >&2
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp)
    trap 'rm -f "$tmpfile"' EXIT

    # Find regular files, compute MD5, format as "hash  path"
    # Using 'md5 -r' for BSD/macOS compatibility
    find "$dir" -type f -exec sh -c '
        for f do
            hash=$(md5 -r "$f" 2>/dev/null | cut -d" " -f1)
            if [ -n "$hash" ]; then
                printf "%s  %s\n" "$hash" "$f"
            fi
        done
    ' sh {} + > "$tmpfile"

    # Sort by hash, then by path to group duplicates and ensure consistent ordering within groups
    sort -k1,1 -k2 "$tmpfile" | awk '
    {
        hash = $1
        # Reconstruct path (in case path contains spaces, though find_dupes usually implies simple paths or we handle them)
        # The format from md5 -r is "hash  filename", so $2 is the start of the path.
        # We need to capture the rest of the line as the path.
        path = ""
        for (i = 2; i <= NF; i++) {
            if (i == 2) path = $i
            else path = path " " $i
        }

        if (hash == prev_hash) {
            group = group " " path
            count++
        } else {
            if (count >= 2) {
                # Sort the paths within the group
                n = split(group, arr, " ")
                # Simple bubble sort for paths within the group
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                # Rebuild sorted group string
                sorted_group = arr[1]
                for (i = 2; i <= n; i++) {
                    sorted_group = sorted_group " " arr[i]
                }
                print sorted_group
            }
            prev_hash = hash
            group = path
            count = 1
        }
    }
    END {
        if (count >= 2) {
            n = split(group, arr, " ")
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (arr[i] > arr[j]) {
                        tmp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = tmp
                    }
                }
            }
            sorted_group = arr[1]
            for (i = 2; i <= n; i++) {
                sorted_group = sorted_group " " arr[i]
            }
            print sorted_group
        }
    }
    ' | sort -k1,1
}