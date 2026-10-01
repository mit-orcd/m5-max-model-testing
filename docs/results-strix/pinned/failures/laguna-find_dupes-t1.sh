find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    
    # Find all regular files and compute their MD5 hashes
    while IFS= read -r -d '' file; do
        if [[ -f "$file" ]]; then
            hash=$(md5 -r "$file" | awk '{print $1}')
            printf '%s\t%s\n' "$hash" "$file"
        fi
    done < <(find "$dir" -type f -print0) > "$tmpfile"
    
    # Group files by hash, filter groups with 2+ files, sort and output
    awk -F'\t' '
        {
            hash = $1
            file = $2
            files[hash] = files[hash] (files[hash] ? " " : "") file
        }
        END {
            for (hash in files) {
                if (split(files[hash], arr, " ") >= 2) {
                    n = asort(arr)
                    printf "%s\n", arr[1]
                    for (i = 2; i <= n; i++) {
                        printf " %s", arr[i]
                    }
                    printf "\n"
                }
            }
        }
    ' "$tmpfile" | sort
    
    rm -f "$tmpfile"
}