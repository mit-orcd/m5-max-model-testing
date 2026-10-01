find_dupes() {
    local dir="${1:-.}"
    if [[ ! -d "$dir" ]]; then
        return 1
    fi

    # Use find to get regular files, compute md5, sort by hash then path
    # macOS md5 -r outputs: "MD5 (file) = hash" or "MD5(file) = hash" depending on version,
    # but typically "MD5 (file) = hash" for stdin or "MD5(file) = hash" for file arg.
    # We use md5 -r <file> which outputs: "MD5 (file) = hash"
    # We need to parse the hash.
    
    # Alternative: use md5 -q <file> for just the hash on some systems, but -r is more standard for BSD.
    # Actually, on macOS, `md5 -r file` outputs: "MD5 (file) = hash"
    # We can use sed to extract the hash.
    
    find "$dir" -type f -print0 | while IFS= read -r -d '' file; do
        hash=$(md5 -r "$file" | awk '{print $NF}')
        printf '%s\t%s\n' "$hash" "$file"
    done | sort -t$'\t' -k1,1 -k2,2 | \
    awk -F'\t' '
    {
        if ($1 == prev_hash) {
            if (count == 1) {
                # Start of a new group, print previous single file? No, we need 2+
                # We need to buffer.
            }
            paths[count] = $2
            count++
        } else {
            if (count >= 2) {
                # Print previous group
                for (i = 1; i < count; i++) {
                    printf "%s%s", paths[i], (i < count-1 ? " " : "\n")
                }
            }
            # Reset for new hash
            delete paths
            paths[1] = $2
            count = 1
            prev_hash = $1
        }
    }
    END {
        if (count >= 2) {
            for (i = 1; i < count; i++) {
                printf "%s%s", paths[i], (i < count-1 ? " " : "\n")
            }
        }
    }
    ' | sort
}