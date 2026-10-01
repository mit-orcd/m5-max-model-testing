find_dupes() {
    local dir="${1:-.}"
    
    # Use find to get all regular files, then compute md5 for each.
    # macOS/BSD md5 outputs: "MD5 (filename) = hash" or "hash  filename" depending on version.
    # md5 -r outputs: "hash  filename" (two spaces between hash and filename).
    
    # We will collect "hash  filepath" lines.
    # Note: find -print0 and xargs -0 are safer for filenames with spaces/newlines.
    
    local tmpfile
    tmpfile=$(mktemp)
    
    # Find all regular files, compute md5 -r, store hash and path.
    # md5 -r format: <hash>  <filename>
    find "$dir" -type f -print0 | xargs -0 -n 1 md5 -r 2>/dev/null | while IFS= read -r line; do
        # Extract hash (first field) and filename (rest after two spaces)
        # md5 -r output: "d41d8cd98f00b204e9800998ecf8427e  /path/to/file"
        hash="${line%%  *}"
        filepath="${line#*  }"
        printf '%s\t%s\n' "$hash" "$filepath"
    done > "$tmpfile"
    
    # Sort by hash, then by filepath
    sort -t$'\t' -k1,1 -k2,2 "$tmpfile" | \
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            group = group " " path
            count++
        } else {
            if (count >= 2) {
                # Sort the group paths
                n = split(group, paths, " ")
                # Simple insertion sort for paths
                for (i = 2; i <= n; i++) {
                    key = paths[i]
                    j = i - 1
                    while (j > 0 && paths[j] > key) {
                        paths[j+1] = paths[j]
                        j--
                    }
                    paths[j+1] = key
                }
                # Print sorted group
                printf "%s", paths[1]
                for (i = 2; i <= n; i++) {
                    printf " %s", paths[i]
                }
                printf "\n"
            }
            prev_hash = hash
            group = path
            count = 1
        }
    }
    END {
        if (count >= 2) {
            n = split(group, paths, " ")
            for (i = 2; i <= n; i++) {
                key = paths[i]
                j = i - 1
                while (j > 0 && paths[j] > key) {
                    paths[j+1] = paths[j]
                    j--
                }
                paths[j+1] = key
            }
            printf "%s", paths[1]
            for (i = 2; i <= n; i++) {
                printf " %s", paths[i]
            }
            printf "\n"
        }
    }
    ' | sort
    
    rm -f "$tmpfile"
}