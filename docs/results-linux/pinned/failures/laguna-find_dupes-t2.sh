find_dupes() {
    local dir="$1"
    local tmpfile
    tmpfile=$(mktemp)
    local tmpfile2
    tmpfile2=$(mktemp)
    
    # Find all regular files and compute their MD5 hashes
    find "$dir" -type f | while IFS= read -r file; do
        md5 -r "$file" 2>/dev/null | awk -v file="$file" '{print $1 "\t" file}'
    done > "$tmpfile"
    
    # Group files by hash and output groups with 2+ files
    awk -F'\t' '
    {
        hash = $1
        file = $2
        files[hash] = files[hash] (files[hash] ? " " : "") file
        count[hash]++
    }
    END {
        for (hash in count) {
            if (count[hash] >= 2) {
                n = split(files[hash], arr, " ")
                # Sort the file paths
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
                for (i = 2; i <= n; i++) {
                    line = line " " arr[i]
                }
                print line
            }
        }
    }' "$tmpfile" | sort > "$tmpfile2"
    
    cat "$tmpfile2"
    
    rm -f "$tmpfile" "$tmpfile2"
}