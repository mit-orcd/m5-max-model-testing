find_dupes() {
    local dir="${1:-.}"
    
    # Find all regular files, compute md5, sort by md5 then path
    # Use find to get files, then md5 -r to hash them
    # md5 -r output format: "hash  filename"
    
    find "$dir" -type f -exec md5 -r {} + 2>/dev/null | \
    awk '{
        # md5 -r outputs: hash  filename (two spaces)
        # We need to extract hash and filename
        hash = $1
        # The filename starts after the first two spaces
        # In awk, $2 is the filename if there are no spaces in names, but find paths might have spaces
        # Actually, md5 -r output is: "hash  filename"
        # So we can reconstruct filename from $2 onwards
        fname = ""
        for (i=2; i<=NF; i++) {
            if (i==2) fname = $i
            else fname = fname " " $i
        }
        print hash "\t" fname
    }' | \
    sort -t$'\t' -k1,1 -k2,2 | \
    awk -F'\t' '
    {
        hash = $1
        path = $2
        if (hash == prev_hash) {
            group = group " " path
            count++
        } else {
            if (count >= 2) {
                print group
            }
            prev_hash = hash
            group = path
            count = 1
        }
    }
    END {
        if (count >= 2) {
            print group
        }
    }'
}