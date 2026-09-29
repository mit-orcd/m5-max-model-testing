find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    trap "rm -f '$tmpfile'" RETURN

    # Find all regular files, compute MD5, group by hash
    while IFS= read -r -d '' file; do
        # Use md5 -r (BSD format: "hash  filename") and extract hash
        md5 -r "$file" | while read -r hash _; do
            printf '%s\t%s\n' "$hash" "$file"
        done
    done < <(find "$dir" -type f -print0) > "$tmpfile"

    # Process the temp file: group by hash, filter groups with 2+ files, sort
    awk -F'\t' '
        {
            hash = $1
            file = $2
            files[hash] = files[hash] (files[hash] ? " " : "") file
            count[hash]++
        }
        END {
            for (h in count) {
                if (count[h] >= 2) {
                    # Sort files in the group
                    n = split(files[h], arr, " ")
                    # Manual bubble sort for portability
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
                    sorted = ""
                    for (i = 1; i <= n; i++) {
                        sorted = sorted (sorted ? " " : "") arr[i]
                    }
                    print sorted
                }
            }
        }
    ' "$tmpfile" | sort
}