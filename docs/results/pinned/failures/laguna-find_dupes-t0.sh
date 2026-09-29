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
            count[hash]++
        }
        END {
            for (h in count) {
                if (count[h] >= 2) {
                    n = split(files[h], arr, " ")
                    for (i = 0; i < n; i++) {
                        print arr[i]
                    }
                    print ""
                }
            }
        }
    ' "$tmpfile" | awk '
        BEGIN { RS = "" }
        {
            n = split($0, lines, "\n")
            for (i = 1; i <= n; i++) {
                if (lines[i] != "") files[i] = lines[i]
            }
            n = asort(files)
            for (i = 1; i <= n; i++) {
                printf "%s", files[i] (i < n ? " " : "")
            }
            print ""
        }
    '
    
    rm -f "$tmpfile"
}