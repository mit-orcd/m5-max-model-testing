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
        # In awk, $0 is the whole line. $1 is hash.
        # The rest is filename. But awk splits by whitespace.
        # md5 -r format: "d41d8cd98f00b204e9800998ecf8427e  /path/to/file"
        # So we can reconstruct the filename by removing the hash and the two spaces.
        # Actually, simpler: use substr to get filename after "hash  "
        # But let's use a more robust approach:
        # Find the position of the first space, then skip two spaces.
        idx = index($0, "  ")
        if (idx > 0) {
            filename = substr($0, idx + 2)
        } else {
            next
        }
        print hash "\t" filename
    }' | \
    sort -t$'\t' -k1,1 -k2,2 | \
    awk -F'\t' '
    {
        hash = $1
        file = $2
        if (hash == prev_hash) {
            if (group == "") {
                # First duplicate found, start group with previous file
                group = prev_file
            }
            group = group " " file
            count++
        } else {
            if (count >= 2) {
                # Print previous group
                print group
            }
            group = ""
            count = 1
        }
        prev_hash = hash
        prev_file = file
    }
    END {
        if (count >= 2) {
            print group
        }
    }' | \
    sort
}