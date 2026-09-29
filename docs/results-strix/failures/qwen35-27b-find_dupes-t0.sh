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
                # Remove empty first element if any
                if (arr[1] == "") {
                    for (i = 1; i < n; i++) arr[i] = arr[i+1]
                    n--
                }
                # Simple bubble sort for paths (small groups usually)
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (arr[i] > arr[j]) {
                            tmp = arr[i]
                            arr[i] = arr[j]
                            arr[j] = tmp
                        }
                    }
                }
                # Build sorted line
                line = arr[1]
                for (i = 2; i <= n; i++) line = line " " arr[i]
                # Store for final sorting by first path
                results[++total] = line
                first_paths[total] = arr[1]
            }
            prev_hash = hash
            paths[hash] = path
            count[hash] = 1
        }
    }
    END {
        if (count[prev_hash] >= 2) {
            n = split(paths[prev_hash], arr, " ")
            if (arr[1] == "") {
                for (i = 1; i < n; i++) arr[i] = arr[i+1]
                n--
            }
            for (i = 1; i <= n; i++) {
                for (j = i + 1; j <= n; j++) {
                    if (arr[i] > arr[j]) {
                        tmp = arr[i]
                        arr[i] = arr[j]
                        arr[j] = tmp
                    }
                }
            }
            line = arr[1]
            for (i = 2; i <= n; i++) line = line " " arr[i]
            results[++total] = line
            first_paths[total] = arr[1]
        }
        # Sort results by first path
        for (i = 1; i <= total; i++) {
            for (j = i + 1; j <= total; j++) {
                if (first_paths[i] > first_paths[j]) {
                    tmp = results[i]; results[i] = results[j]; results[j] = tmp
                    tmp = first_paths[i]; first_paths[i] = first_paths[j]; first_paths[j] = tmp
                }
            }
        }
        for (i = 1; i <= total; i++) print results[i]
    }'
}