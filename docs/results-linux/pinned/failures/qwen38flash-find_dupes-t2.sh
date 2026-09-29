find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5, store hash and path
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | while IFS= read -r line; do
        # md5 -r outputs: <hash> <path>
        local hash="${line%% *}"
        local path="${line#* }"
        echo "$hash $path"
    done > "$tmpfile"
    
    # Group by hash, filter groups with 2+ files, sort paths within group, sort groups by first path
    awk '
    {
        hash = $1
        path = substr($0, length(hash) + 2)
        if (hash in groups) {
            groups[hash] = groups[hash] "\n" path
        } else {
            groups[hash] = path
        }
        count[hash]++
    }
    END {
        for (hash in groups) {
            if (count[hash] >= 2) {
                n = split(groups[hash], paths, "\n")
                # Sort paths
                for (i = 1; i <= n; i++) {
                    for (j = i + 1; j <= n; j++) {
                        if (paths[i] > paths[j]) {
                            tmp = paths[i]
                            paths[i] = paths[j]
                            paths[j] = tmp
                        }
                    }
                }
                # Build output line
                out = ""
                for (i = 1; i <= n; i++) {
                    if (i > 1) out = out " "
                    out = out paths[i]
                }
                print out
            }
        }
    }
    ' "$tmpfile" | sort
    
    rm -f "$tmpfile"
}