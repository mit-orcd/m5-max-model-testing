find_dupes() {
    local search_dir="$1"
    find "$search_dir" -type f -exec md5 -r {} + | \
    sort | \
    awk '
        {
            hash = $1
            $1 = ""
            sub(/^ /, "", $0)
            path = $0
            
            # Handle cases where paths might contain spaces if md5 -r output format differs
            # but standard md5 -r is "hash  path"
            # We split by the first occurrence of '  '
            split($0, parts, "  ")
            
            # Re-calculating based on the fact that $1 is hash and the rest is path
            # In md5 -r, the output is: hash  path
            # We need to extract the path correctly.
        }
    '
    # Since awk parsing of 'hash  path' with spaces is tricky, 
    # let's use a more robust approach for the logic.
}