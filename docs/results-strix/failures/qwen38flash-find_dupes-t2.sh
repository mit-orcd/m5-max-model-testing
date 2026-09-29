find_dupes() {
    local dir="$1"
    local tmpfile=$(mktemp)
    
    # Find all regular files, compute MD5 for each, store as "md5 path"
    find "$dir" -type f -exec md5 -r {} \; 2>/dev/null | sort > "$tmpfile"
    
    # Read the sorted md5/path pairs and group by md5
    local prev_md5=""
    local group=""
    local found_first=0
    
    while IFS=' ' read -r md5 path; do
        # Handle paths with spaces: md5 is first token, rest is path
        # Actually md5 -r outputs "hash filename", so we need to be careful
        # The format is: <hash> <filename>
        # Since filenames can have spaces, we need to extract hash and path properly
        
        # Extract the hash (first field) and the path (rest of line)
        # But we already split on space above, which breaks paths with spaces.
        # Let's re-read properly.
        :
    done < "$tmpfile"
    
    rm -f "$tmpfile"
}