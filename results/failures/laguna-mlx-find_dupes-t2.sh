find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files and compute their MD5 hashes
    find "$dir" -type f -exec md5 -r {} + | sort -k2 | \
    awk '{printf "%s %s\n", $2, $1}' > "$tmpfile"
    
    # Group files by MD5 hash
    awk '{groups[$1] = groups[$1] " " $2} 
    END {
        for (hash in groups) {
            split(groups[hash], files, " ")
            # Skip if less than 2 files
            if (length(files) < 3) continue
            # Sort the file paths
            n = asorti(files, sorted)
            for (i = 1; i <= n; i++) {
                if (i > 1) printf " "
                printf "%s", sorted[i]
            }
            print ""
        }
    }' "$tmpfile" | sort | \
    awk 'NF >= 2 {print}'
    
    rm -f "$tmpfile"
}